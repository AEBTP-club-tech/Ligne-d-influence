import sys
import json
import copy
import os
import importlib
from pathlib import Path
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QLabel, QPushButton, QComboBox, 
                             QSpinBox, QCheckBox, QSlider,
                             QGroupBox, QGridLayout, QMessageBox, QFileDialog,
                             QMenuBar, QMenu, QToolBar, QDialog, QDialogButtonBox,
                             QSplitter, QTextEdit)
from PyQt6.QtCore import Qt, QSize, QThread, pyqtSignal, QMetaObject, QTimer, QObject
from PyQt6.QtGui import QFont, QShortcut, QKeySequence, QAction, QIcon
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure
import numpy as np
import utils

class CalculationWorker(QObject):
    """Worker pour exécuter le calcul dans un thread séparé"""
    finished = pyqtSignal(str, str)  # exe_name, working_dir
    error = pyqtSignal(str)  # error message
    
    def run_calculation(self, exe_path, working_dir):
        """Exécute le calcul dans le thread worker"""
        try:
            original_cwd = os.getcwd()
            os.chdir(str(working_dir))
            
            try:
                return_code = os.system(f'"{exe_path}"')
                exe_name = str(exe_path.name)
                wd = str(working_dir)
                # Émettre le signal de fin (sera reçu dans le thread principal)
                self.finished.emit(exe_name, wd)
            finally:
                os.chdir(original_cwd)
        except Exception as e:
            self.error.emit(str(e))


class StructuralAnalysisGUI(QMainWindow):
    PLACEHOLDER_MESSAGE = "Selectionnez une option et cliquez sur Tracer"
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Analyse Structurelle - Interface de Visualisation")
        self.setGeometry(100, 100, 1400, 900)
        
        self.config_path = Path(__file__).resolve().with_name("Configuration.json")
        self.config = self.load_configuration()
        
        self.is_playing = False
        self.animation = None
        self.animation_line = None
        self.data_cache = {}
        self.is_fullscreen = False
        
        self.setup_ui()
        self.apply_dark_theme()
        self.setup_shortcuts()
        self.update_spinbox_ranges()

    def load_configuration(self):
        try:
            if self.config_path.exists():
                with open(self.config_path, "r", encoding="utf-8") as file:
                    loaded = json.load(file)
                    config = copy.deepcopy(utils.DEFAULT_CONFIG)
                    config.update({k: v for k, v in loaded.items() if k != "style"})
                    if "style" in loaded and isinstance(loaded["style"], dict):
                        config["style"].update(loaded["style"])
                    self.sync_utils_config(config)
                    return config
        except Exception as exc:
            print(f"Impossible de charger Configuration.json: {exc}")
        fallback = copy.deepcopy(utils.DEFAULT_CONFIG)
        self.sync_utils_config(fallback)
        return fallback

    def sync_utils_config(self, config=None):
        try:
            utils.current_config = copy.deepcopy(config or self.config)
        except Exception:
            pass

    def persist_configuration(self, silent=True):
        try:
            with open(self.config_path, "w", encoding="utf-8") as file:
                json.dump(self.config, file, ensure_ascii=False, indent=4)
            self.sync_utils_config()
            if not silent:
                QMessageBox.information(self, "Configuration", "Configuration sauvegardée.")
        except Exception as exc:
            message = f"Erreur lors de la sauvegarde: {exc}"
            if silent:
                print(message)
            else:
                QMessageBox.critical(self, "Erreur", message)
        
    def setup_ui(self):
        # Créer la barre de menu
        self.create_menu_bar()
        
        # Créer la barre d'outils avec icônes
        self.create_toolbar()
        
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout(central_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        
        # Créer un splitter pour permettre le redimensionnement
        self.splitter = QSplitter(Qt.Orientation.Horizontal)
        
        self.left_panel = self.create_left_panel()
        self.left_panel_visible = True
        self.splitter.addWidget(self.left_panel)
        
        right_panel = self.create_right_panel()
        self.splitter.addWidget(right_panel)
        
        # Définir les proportions initiales (1:2 = panneau gauche plus petit)
        self.splitter.setStretchFactor(0, 1)
        self.splitter.setStretchFactor(1, 2)
        self.splitter.setSizes([300, 600])  # Largeurs initiales en pixels
        
        main_layout.addWidget(self.splitter)
    
    def create_menu_bar(self):
        """Crée la barre de menu avec toutes les options"""
        menubar = self.menuBar()
        
        # Menu Fichier
        file_menu = menubar.addMenu("Fichier")
        edit_input_action = file_menu.addAction("Éditer input.txt")
        edit_input_action.triggered.connect(self.edit_input_file)
        run_calc_action = file_menu.addAction("Lancer le Calcul C++")
        run_calc_action.triggered.connect(self.run_calculation)
        file_menu.addSeparator()
        save_config_action = file_menu.addAction("Sauvegarder Configuration")
        save_config_action.triggered.connect(self.save_configuration)
        file_menu.addSeparator()
        export_action = file_menu.addAction("Exporter Animation (GIF)")
        export_action.triggered.connect(self.export_animation)
        file_menu.addSeparator()
        quit_action = file_menu.addAction("Quitter")
        quit_action.setShortcut("Ctrl+Q")
        quit_action.triggered.connect(self.close)
        
        # Menu Visualisation
        view_menu = menubar.addMenu("Visualisation")
        plot_action = view_menu.addAction("Tracer")
        plot_action.setShortcut("Ctrl+P")
        plot_action.triggered.connect(self.plot_current_selection)
        view_menu.addSeparator()
        animate_action = view_menu.addAction("Animer (Courbe)")
        animate_action.setShortcut("Ctrl+A")
        animate_action.triggered.connect(self.animate_current_selection)
        animate_full_action = view_menu.addAction("Animer (Complet)")
        animate_full_action.setShortcut("Ctrl+Shift+A")
        animate_full_action.triggered.connect(self.animate_full_curve)
        view_menu.addSeparator()
        max_action = view_menu.addAction("Afficher Maximum")
        max_action.setShortcut("Ctrl+M")
        max_action.triggered.connect(self.show_maximum)
        
        # Menu Configuration
        config_menu = menubar.addMenu("Configuration")
        
        # Options de visibilité
        self.grid_action = config_menu.addAction("Afficher la Grille")
        self.grid_action.setCheckable(True)
        self.grid_action.setChecked(self.config["grid"])
        self.grid_action.triggered.connect(lambda checked: self.toggle_option("grid", checked))
        
        self.travee_action = config_menu.addAction("Afficher les Travées")
        self.travee_action.setCheckable(True)
        self.travee_action.setChecked(self.config["travee"])
        self.travee_action.triggered.connect(lambda checked: self.toggle_option("travee", checked))
        
        self.noeud_action = config_menu.addAction("Afficher les Noeuds")
        self.noeud_action.setCheckable(True)
        self.noeud_action.setChecked(self.config["noeud"])
        self.noeud_action.triggered.connect(lambda checked: self.toggle_option("noeud", checked))
        
        self.legend_action = config_menu.addAction("Afficher la Légende")
        self.legend_action.setCheckable(True)
        self.legend_action.setChecked(self.config["legend"])
        self.legend_action.triggered.connect(lambda checked: self.toggle_option("legend", checked))
        
        config_menu.addSeparator()
        
        self.invert_action = config_menu.addAction("Inverser l'Axe Y")
        self.invert_action.setCheckable(True)
        self.invert_action.setChecked(self.config["axe_y_inverser"])
        self.invert_action.triggered.connect(lambda checked: self.toggle_option("axe_y_inverser", checked))
        
        self.style_action = config_menu.addAction("Style Matplotlib par Défaut")
        self.style_action.setCheckable(True)
        self.style_action.setChecked(self.config["default_matplotlib_style"])
        self.style_action.triggered.connect(lambda checked: self.toggle_option("default_matplotlib_style", checked))
        
        # Menu Vue
        window_menu = menubar.addMenu("Vue")
        
        # Action pour afficher/masquer la barre d'outils
        self.toolbar_visible_action = window_menu.addAction("Afficher la Barre d'Outils")
        self.toolbar_visible_action.setCheckable(True)
        self.toolbar_visible_action.setChecked(True)
        self.toolbar_visible_action.triggered.connect(self.toggle_toolbar)
        
        window_menu.addSeparator()
        
        fullscreen_action = window_menu.addAction("Mode Plein Écran")
        # Le raccourci F11 est défini dans la barre d'outils pour éviter les conflits
        fullscreen_action.triggered.connect(self.toggle_fullscreen)
        
        # Menu Aide
        help_menu = menubar.addMenu("Aide")
        help_action = help_menu.addAction("Guide d'Utilisation")
        help_action.setShortcut(QKeySequence("F1"))
        help_action.triggered.connect(self.show_help)
        help_menu.addSeparator()
        shortcuts_action = help_menu.addAction("Raccourcis Clavier")
        shortcuts_action.triggered.connect(self.show_shortcuts)
        help_menu.addSeparator()
        about_action = help_menu.addAction("À propos")
        about_action.triggered.connect(self.show_about)
    
    def toggle_option(self, option_key, checked):
        """Bascule une option d'affichage"""
        self.config[option_key] = checked
        self.update_config(option_key, checked)
        # Mettre à jour les actions du menu
        if option_key == "grid":
            self.grid_action.setChecked(checked)
        elif option_key == "travee":
            self.travee_action.setChecked(checked)
        elif option_key == "noeud":
            self.noeud_action.setChecked(checked)
        elif option_key == "legend":
            self.legend_action.setChecked(checked)
        elif option_key == "axe_y_inverser":
            self.invert_action.setChecked(checked)
        elif option_key == "default_matplotlib_style":
            self.style_action.setChecked(checked)
    
    def show_help(self):
        """Affiche la fenêtre d'aide complète"""
        help_dialog = QDialog(self)
        help_dialog.setWindowTitle("Guide d'Utilisation - Analyse Structurelle")
        help_dialog.setModal(False)
        help_dialog.resize(900, 700)
        
        # Appliquer le thème sombre à la fenêtre
        help_dialog.setStyleSheet("""
            QDialog {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QTextEdit {
                background-color: #2b2b2b;
                color: #ffffff;
                border: 1px solid #555555;
            }
            QDialogButtonBox {
                background-color: #2b2b2b;
            }
            QPushButton {
                background-color: #404040;
                color: #ffffff;
                border: 1px solid #555555;
                padding: 5px 15px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #505050;
            }
        """)
        
        layout = QVBoxLayout(help_dialog)
        
        # Zone de texte avec contenu HTML
        help_text = QTextEdit()
        help_text.setReadOnly(True)
        help_text.setHtml(self.get_help_content())
        layout.addWidget(help_text)
        
        # Bouton de fermeture
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
        button_box.rejected.connect(help_dialog.close)
        layout.addWidget(button_box)
        
        help_dialog.exec()
    
    def get_help_content(self):
        """Retourne le contenu HTML de l'aide"""
        return """
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; line-height: 1.6; background-color: #2b2b2b; color: #ffffff; }
                h1 { color: #5dade2; border-bottom: 3px solid #3498db; padding-bottom: 10px; }
                h2 { color: #85c1e2; margin-top: 25px; border-left: 4px solid #3498db; padding-left: 10px; }
                h3 { color: #aed6f1; margin-top: 20px; }
                p { margin: 10px 0; text-align: justify; color: #e0e0e0; }
                ul, ol { margin: 10px 0; padding-left: 30px; color: #e0e0e0; }
                li { margin: 5px 0; }
                code { background-color: #404040; color: #f39c12; padding: 2px 6px; border-radius: 3px; font-family: 'Courier New', monospace; }
                .section { margin: 20px 0; padding: 15px; background-color: #353535; border-radius: 5px; border: 1px solid #555555; }
                .icon { font-size: 1.2em; }
                .shortcut { background-color: #404040; color: #f39c12; padding: 2px 8px; border-radius: 3px; font-weight: bold; font-family: 'Courier New', monospace; }
                .warning { background-color: #4a3a00; color: #ffd700; padding: 10px; border-left: 4px solid #ffc107; margin: 10px 0; border-radius: 3px; }
                .info { background-color: #003d4a; color: #5dade2; padding: 10px; border-left: 4px solid #17a2b8; margin: 10px 0; border-radius: 3px; }
                strong { color: #ffffff; }
                em { color: #aed6f1; }
            </style>
        </head>
        <body>
            <h1>📚 Guide d'Utilisation - Analyse Structurelle</h1>
            
            <div class="section">
                <h2>🎯 Vue d'Ensemble</h2>
                <p>Cette application permet de visualiser et d'analyser les lignes d'influence pour l'analyse structurelle des tabliers de ponts. 
                Elle offre des fonctionnalités de tracé, d'animation et d'exportation des résultats de calcul.</p>
            </div>
            
            <div class="section">
                <h2>🚀 Démarrage Rapide</h2>
                <ol>
                    <li><strong>Lancer le calcul C++</strong> : Cliquez sur l'icône <span class="icon">▶️</span> dans la barre d'outils ou utilisez le menu <em>Fichier → Lancer le Calcul C++</em></li>
                    <li><strong>Sélectionner le type de courbe</strong> : Utilisez l'icône <span class="icon">📊</span> ou le menu <em>Configuration</em></li>
                    <li><strong>Choisir la travée et la section</strong> : Utilisez les contrôles dans le panneau de configuration</li>
                    <li><strong>Tracer la courbe</strong> : Cliquez sur l'icône <span class="icon">📈</span> ou appuyez sur <span class="shortcut">Ctrl+P</span></li>
                    <li><strong>Animer</strong> : Utilisez l'icône <span class="icon">🎬</span> pour animer la courbe sélectionnée</li>
                </ol>
            </div>
            
            <div class="section">
                <h2>📋 Fonctionnalités Principales</h2>
                
                <h3>1. Tracé de Courbes</h3>
                <p>Visualisez les lignes d'influence pour différents types de courbes :</p>
                <ul>
                    <li><strong>Moments de Travée</strong> (<code>span_moments</code>) : Moments dans les travées</li>
                    <li><strong>Forces de Cisaillement</strong> (<code>span_shear_forces</code>) : Forces de cisaillement</li>
                    <li><strong>Rotations</strong> (<code>span_rotations</code>) : Rotations aux nœuds</li>
                    <li><strong>Moments d'Appui</strong> (<code>support_moments</code>) : Moments aux appuis</li>
                    <li><strong>Réactions d'Appui</strong> (<code>support_reactions</code>) : Réactions aux appuis</li>
                </ul>
                
                <h3>2. Animation</h3>
                <p>Deux modes d'animation sont disponibles :</p>
                <ul>
                    <li><strong>Animation Courbe</strong> : Anime uniquement la courbe sélectionnée</li>
                    <li><strong>Animation Complète</strong> : Anime toutes les courbes avec les éléments structurels</li>
                </ul>
                <div class="info">
                    <strong>💡 Astuce</strong> : Utilisez le slider dans le panneau de configuration pour ajuster la vitesse d'animation.
                </div>
                
                <h3>3. Affichage du Maximum</h3>
                <p>L'icône <span class="icon">📊</span> permet d'afficher automatiquement la courbe avec la plus grande aire de moment, 
                ce qui correspond généralement au cas le plus défavorable.</p>
                
                <h3>4. Exportation</h3>
                <p>Exportez vos visualisations :</p>
                <ul>
                    <li><strong>Export Animation</strong> : Exporte l'animation en format GIF</li>
                    <li><strong>Sauvegarde Configuration</strong> : Sauvegarde vos paramètres dans <code>Configuration.json</code></li>
                </ul>
            </div>
            
            <div class="section">
                <h2>⚙️ Configuration</h2>
                
                <h3>Options d'Affichage</h3>
                <ul>
                    <li><strong>Grille</strong> : Affiche/masque la grille du graphique</li>
                    <li><strong>Travées</strong> : Affiche les limites des travées sur le graphique</li>
                    <li><strong>Nœuds</strong> : Affiche les positions des nœuds</li>
                    <li><strong>Légende</strong> : Affiche/masque la légende</li>
                    <li><strong>Inverser l'Axe Y</strong> : Inverse l'orientation de l'axe vertical</li>
                </ul>
                
                <h3>Édition de input.txt</h3>
                <p>L'icône <span class="icon">📝</span> permet d'éditer directement le fichier <code>input.txt</code> depuis l'interface. 
                Les modifications seront prises en compte lors du prochain calcul.</p>
                <div class="warning">
                    <strong>⚠️ Attention</strong> : Assurez-vous de sauvegarder les modifications avant de lancer un nouveau calcul.
                </div>
            </div>
            
            <div class="section">
                <h2>🎨 Interface Utilisateur</h2>
                
                <h3>Barre d'Outils</h3>
                <p>La barre d'outils contient les actions principales :</p>
                <ul>
                    <li><span class="icon">📈</span> <strong>Tracer</strong> : Trace la courbe sélectionnée</li>
                    <li><span class="icon">🎬</span> <strong>Animer</strong> : Lance l'animation</li>
                    <li><span class="icon">📊</span> <strong>Maximum</strong> : Affiche le maximum</li>
                    <li><span class="icon">💾</span> <strong>Sauvegarder</strong> : Sauvegarde la configuration</li>
                    <li><span class="icon">📤</span> <strong>Exporter</strong> : Exporte l'animation</li>
                    <li><span class="icon">🔲</span> <strong>Plein Écran</strong> : Active le mode plein écran</li>
                    <li><span class="icon">⚙️</span> <strong>Configuration</strong> : Ouvre le dialogue de configuration</li>
                    <li><span class="icon">📝</span> <strong>Éditer input.txt</strong> : Édite le fichier d'entrée</li>
                    <li><span class="icon">▶️</span> <strong>Lancer Calcul</strong> : Lance le calcul C++</li>
                    <li><span class="icon">📊</span> <strong>Type de Courbe</strong> : Sélectionne le type de courbe</li>
                    <li><span class="icon">🎯</span> <strong>Sélection Travée/Section</strong> : Ouvre le sélecteur</li>
                    <li><span class="icon">👁️</span> <strong>Masquer/Afficher Panneau</strong> : Bascule la visibilité du panneau</li>
                </ul>
                
                <h3>Panneau de Configuration</h3>
                <p>Le panneau de gauche peut être :</p>
                <ul>
                    <li><strong>Redimensionné</strong> : Glissez la bordure pour ajuster la taille</li>
                    <li><strong>Masqué</strong> : Utilisez l'icône <span class="icon">👁️</span> ou le menu <em>Vue</em></li>
                </ul>
            </div>
            
            <div class="section">
                <h2>⌨️ Raccourcis Clavier</h2>
                <ul>
                    <li><span class="shortcut">F1</span> : Afficher l'aide</li>
                    <li><span class="shortcut">F11</span> : Basculer le mode plein écran</li>
                    <li><span class="shortcut">Ctrl+P</span> : Tracer la courbe</li>
                    <li><span class="shortcut">Ctrl+A</span> : Animer la courbe</li>
                    <li><span class="shortcut">Ctrl+M</span> : Afficher le maximum</li>
                    <li><span class="shortcut">Ctrl+S</span> : Sauvegarder la configuration</li>
                    <li><span class="shortcut">Ctrl+E</span> : Exporter l'animation</li>
                    <li><span class="shortcut">Ctrl+Q</span> : Quitter l'application</li>
                </ul>
            </div>
            
            <div class="section">
                <h2>📁 Structure des Fichiers</h2>
                <p>L'application utilise les fichiers suivants :</p>
                <ul>
                    <li><code>input.txt</code> : Fichier d'entrée pour le calcul C++</li>
                    <li><code>Configuration.json</code> : Fichier de configuration de l'interface</li>
                    <li><code>data/results/</code> : Dossier contenant les résultats du calcul</li>
                    <li><code>Ligne d'influence.exe</code> : Exécutable du calcul C++</li>
                </ul>
            </div>
            
            <div class="section">
                <h2>🔧 Dépannage</h2>
                
                <h3>Le calcul ne se lance pas</h3>
                <ul>
                    <li>Vérifiez que <code>Ligne d'influence.exe</code> existe dans le répertoire</li>
                    <li>Assurez-vous que <code>input.txt</code> est présent et valide</li>
                    <li>Vérifiez les permissions d'exécution</li>
                </ul>
                
                <h3>Les données ne se chargent pas</h3>
                <ul>
                    <li>Vérifiez que le calcul C++ s'est terminé avec succès</li>
                    <li>Vérifiez que le dossier <code>data/results/</code> contient les fichiers JSON</li>
                    <li>Relancez le calcul si nécessaire</li>
                </ul>
                
                <h3>L'animation ne fonctionne pas</h3>
                <ul>
                    <li>Assurez-vous qu'une courbe est sélectionnée</li>
                    <li>Vérifiez que les données sont chargées</li>
                    <li>Essayez de tracer d'abord la courbe avant d'animer</li>
                </ul>
            </div>
            
            <div class="section">
                <h2>📞 Support</h2>
                <p>Pour toute question ou problème, consultez :</p>
                <ul>
                    <li>Le menu <em>Aide → Raccourcis Clavier</em> pour la liste complète des raccourcis</li>
                    <li>Le menu <em>Aide → À propos</em> pour les informations sur la version</li>
                </ul>
            </div>
            
            <div class="info">
                <strong>💡 Note</strong> : Cette application nécessite que le calcul C++ soit exécuté au préalable pour générer les données de visualisation.
            </div>
        </body>
        </html>
        """
    
    def show_shortcuts(self):
        """Affiche la fenêtre des raccourcis clavier"""
        shortcuts_dialog = QDialog(self)
        shortcuts_dialog.setWindowTitle("Raccourcis Clavier")
        shortcuts_dialog.setModal(False)
        shortcuts_dialog.resize(600, 500)
        
        # Appliquer le thème sombre à la fenêtre
        shortcuts_dialog.setStyleSheet("""
            QDialog {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QTextEdit {
                background-color: #2b2b2b;
                color: #ffffff;
                border: 1px solid #555555;
            }
            QDialogButtonBox {
                background-color: #2b2b2b;
            }
            QPushButton {
                background-color: #404040;
                color: #ffffff;
                border: 1px solid #555555;
                padding: 5px 15px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #505050;
            }
        """)
        
        layout = QVBoxLayout(shortcuts_dialog)
        
        # Zone de texte avec contenu HTML
        shortcuts_text = QTextEdit()
        shortcuts_text.setReadOnly(True)
        shortcuts_text.setHtml(self.get_shortcuts_content())
        layout.addWidget(shortcuts_text)
        
        # Bouton de fermeture
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
        button_box.rejected.connect(shortcuts_dialog.close)
        layout.addWidget(button_box)
        
        shortcuts_dialog.exec()
    
    def get_shortcuts_content(self):
        """Retourne le contenu HTML des raccourcis"""
        return """
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; line-height: 1.8; background-color: #2b2b2b; color: #ffffff; }
                h1 { color: #5dade2; border-bottom: 3px solid #3498db; padding-bottom: 10px; }
                h2 { color: #85c1e2; margin-top: 25px; border-left: 4px solid #3498db; padding-left: 10px; }
                table { width: 100%; border-collapse: collapse; margin: 15px 0; }
                th { background-color: #404040; color: #ffffff; padding: 12px; text-align: left; border: 1px solid #555555; }
                td { padding: 10px; border-bottom: 1px solid #555555; color: #e0e0e0; }
                tr:hover { background-color: #353535; }
                .shortcut { background-color: #404040; color: #f39c12; padding: 4px 10px; border-radius: 3px; font-weight: bold; font-family: 'Courier New', monospace; }
                .category { font-weight: bold; color: #ffffff; }
                ul { color: #e0e0e0; }
                li { margin: 5px 0; }
                strong { color: #ffffff; }
            </style>
        </head>
        <body>
            <h1>⌨️ Raccourcis Clavier</h1>
            
            <h2>📊 Visualisation</h2>
            <table>
                <tr>
                    <th>Raccourci</th>
                    <th>Action</th>
                </tr>
                <tr>
                    <td><span class="shortcut">Ctrl+P</span></td>
                    <td>Tracer la courbe sélectionnée</td>
                </tr>
                <tr>
                    <td><span class="shortcut">Ctrl+A</span></td>
                    <td>Animer la courbe sélectionnée</td>
                </tr>
                <tr>
                    <td><span class="shortcut">Ctrl+M</span></td>
                    <td>Afficher le maximum (plus grande aire)</td>
                </tr>
            </table>
            
            <h2>💾 Fichier</h2>
            <table>
                <tr>
                    <th>Raccourci</th>
                    <th>Action</th>
                </tr>
                <tr>
                    <td><span class="shortcut">Ctrl+S</span></td>
                    <td>Sauvegarder la configuration</td>
                </tr>
                <tr>
                    <td><span class="shortcut">Ctrl+E</span></td>
                    <td>Exporter l'animation (GIF)</td>
                </tr>
                <tr>
                    <td><span class="shortcut">Ctrl+Q</span></td>
                    <td>Quitter l'application</td>
                </tr>
            </table>
            
            <h2>👁️ Vue</h2>
            <table>
                <tr>
                    <th>Raccourci</th>
                    <th>Action</th>
                </tr>
                <tr>
                    <td><span class="shortcut">F11</span></td>
                    <td>Basculer le mode plein écran</td>
                </tr>
            </table>
            
            <h2>❓ Aide</h2>
            <table>
                <tr>
                    <th>Raccourci</th>
                    <th>Action</th>
                </tr>
                <tr>
                    <td><span class="shortcut">F1</span></td>
                    <td>Afficher le guide d'utilisation</td>
                </tr>
            </table>
            
            <h2>🎯 Navigation dans les Graphiques</h2>
            <p>La barre d'outils Matplotlib intégrée permet également d'utiliser :</p>
            <ul>
                <li><strong>Zoom</strong> : Utilisez la molette de la souris ou les boutons de zoom</li>
                <li><strong>Pan</strong> : Cliquez et glissez pour déplacer la vue</li>
                <li><strong>Réinitialiser</strong> : Utilisez le bouton "Home" pour revenir à la vue initiale</li>
                <li><strong>Configuration</strong> : Utilisez le bouton "Configure" pour ajuster les paramètres du graphique</li>
            </ul>
        </body>
        </html>
        """
    
    def show_about(self):
        """Affiche la boîte de dialogue À propos améliorée"""
        about_dialog = QDialog(self)
        about_dialog.setWindowTitle("À propos")
        about_dialog.setModal(True)
        about_dialog.resize(500, 400)
        
        # Appliquer le thème sombre à la fenêtre
        about_dialog.setStyleSheet("""
            QDialog {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QTextEdit {
                background-color: #2b2b2b;
                color: #ffffff;
                border: 1px solid #555555;
            }
            QDialogButtonBox {
                background-color: #2b2b2b;
            }
            QPushButton {
                background-color: #404040;
                color: #ffffff;
                border: 1px solid #555555;
                padding: 5px 15px;
                border-radius: 3px;
            }
            QPushButton:hover {
                background-color: #505050;
            }
        """)
        
        layout = QVBoxLayout(about_dialog)
        
        # Zone de texte avec contenu HTML
        about_text = QTextEdit()
        about_text.setReadOnly(True)
        about_text.setHtml(self.get_about_content())
        layout.addWidget(about_text)
        
        # Bouton de fermeture
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
        button_box.rejected.connect(about_dialog.close)
        layout.addWidget(button_box)
        
        about_dialog.exec()
    
    def get_about_content(self):
        """Retourne le contenu HTML de la boîte À propos"""
        return """
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; line-height: 1.8; text-align: center; background-color: #2b2b2b; color: #ffffff; }
                h1 { color: #5dade2; margin-bottom: 20px; }
                h2 { color: #85c1e2; margin-top: 25px; border-bottom: 2px solid #3498db; padding-bottom: 5px; }
                p { margin: 10px 0; text-align: justify; color: #e0e0e0; }
                .version { font-size: 1.2em; color: #3498db; font-weight: bold; margin: 20px 0; }
                .description { background-color: #353535; padding: 15px; border-radius: 5px; margin: 15px 0; border: 1px solid #555555; }
                .tech { background-color: #353535; padding: 10px; border-radius: 5px; margin: 10px 0; border: 1px solid #555555; }
                ul { text-align: left; display: inline-block; color: #e0e0e0; }
                li { margin: 5px 0; }
                strong { color: #ffffff; }
            </style>
        </head>
        <body>
            <h1>🏗️ Analyse Structurelle</h1>
            <h2>Interface de Visualisation</h2>
            
            <div class="version">Version 1.0</div>
            
            <div class="description">
                <p><strong>Logiciel de visualisation des lignes d'influence</strong> pour l'analyse structurelle des tabliers de ponts.</p>
                <p>Cette application permet de visualiser, analyser et exporter les résultats de calculs structurels, 
                notamment les moments, forces de cisaillement, rotations et réactions d'appui.</p>
            </div>
            
            <h2>🔧 Technologies Utilisées</h2>
            <div class="tech">
                <ul>
                    <li><strong>PyQt6</strong> : Interface graphique</li>
                    <li><strong>Matplotlib</strong> : Visualisation et animation des graphiques</li>
                    <li><strong>NumPy</strong> : Calculs numériques</li>
                    <li><strong>Python</strong> : Langage de programmation</li>
                </ul>
            </div>
            
            <h2>📋 Fonctionnalités</h2>
            <ul>
                <li>Visualisation des lignes d'influence</li>
                <li>Animation des courbes</li>
                <li>Exportation en format GIF</li>
                <li>Configuration personnalisable</li>
                <li>Intégration avec calculs C++</li>
            </ul>
            
            <p style="margin-top: 30px; color: #7f8c8d; font-size: 0.9em;">
                Développé pour l'analyse structurelle des ponts
            </p>
        </body>
        </html>
        """
    
    def show_config_dialog(self):
        """Affiche une boîte de dialogue avec les options de configuration"""
        dialog = QDialog(self)
        dialog.setWindowTitle("Options de Configuration")
        dialog.setModal(True)
        dialog.resize(450, 650)
        
        layout = QVBoxLayout(dialog)
        
        # Groupe Sélection de Courbe
        curve_group = QGroupBox("Type de Courbe")
        curve_layout = QVBoxLayout()
        
        curve_combo = QComboBox()
        curve_combo.addItems([
            "span_moments - Moments de Travee",
            "span_shear_forces - Forces de Cisaillement",
            "span_rotations - Rotations",
            "span_deflections - Deflexions",
            "support_moments - Moments d'Appui",
            "support_reactions - Reactions d'Appui"
        ])
        # Trouver l'index de la courbe actuelle
        current_curve = self.get_current_curve_type()
        for i in range(curve_combo.count()):
            if curve_combo.itemText(i).startswith(current_curve):
                curve_combo.setCurrentIndex(i)
                break
        curve_layout.addWidget(curve_combo)
        curve_group.setLayout(curve_layout)
        layout.addWidget(curve_group)
        
        # Groupe Sélection Travée & Section
        span_section_group = QGroupBox("Sélection Travée & Section")
        span_section_layout = QGridLayout()
        
        span_section_layout.addWidget(QLabel("Travée (Span):"), 0, 0)
        span_spin = QSpinBox()
        span_spin.setMinimum(0)
        span_spin.setMaximum(100)
        span_spin.setValue(self.span_spin.value())
        span_section_layout.addWidget(span_spin, 0, 1)
        
        span_section_layout.addWidget(QLabel("Section:"), 1, 0)
        section_spin = QSpinBox()
        section_spin.setMinimum(0)
        section_spin.setMaximum(100)
        section_spin.setValue(self.section_spin.value())
        span_section_layout.addWidget(section_spin, 1, 1)
        
        span_section_group.setLayout(span_section_layout)
        layout.addWidget(span_section_group)
        
        # Groupe Animation
        animation_group = QGroupBox("Animation")
        animation_layout = QVBoxLayout()
        
        speed_label = QLabel(f"Vitesse: {self.config['vitesse_bridge']:.3f}")
        animation_layout.addWidget(speed_label)
        
        speed_slider = QSlider(Qt.Orientation.Horizontal)
        speed_slider.setMinimum(1)
        speed_slider.setMaximum(50)
        speed_slider.setValue(int(self.config['vitesse_bridge'] * 1000))
        
        def update_speed_label(value):
            speed_label.setText(f"Vitesse: {value / 1000.0:.3f}")
        speed_slider.valueChanged.connect(update_speed_label)
        animation_layout.addWidget(speed_slider)
        
        animation_group.setLayout(animation_layout)
        layout.addWidget(animation_group)
        
        # Groupe Affichage
        display_group = QGroupBox("Affichage")
        display_layout = QVBoxLayout()
        
        grid_check = QCheckBox("Afficher la Grille")
        grid_check.setChecked(self.config["grid"])
        display_layout.addWidget(grid_check)
        
        travee_check = QCheckBox("Afficher les Travées")
        travee_check.setChecked(self.config["travee"])
        display_layout.addWidget(travee_check)
        
        noeud_check = QCheckBox("Afficher les Noeuds")
        noeud_check.setChecked(self.config["noeud"])
        display_layout.addWidget(noeud_check)
        
        legend_check = QCheckBox("Afficher la Légende")
        legend_check.setChecked(self.config["legend"])
        display_layout.addWidget(legend_check)
        
        display_group.setLayout(display_layout)
        layout.addWidget(display_group)
        
        # Groupe Options
        options_group = QGroupBox("Options")
        options_layout = QVBoxLayout()
        
        invert_check = QCheckBox("Inverser l'Axe Y")
        invert_check.setChecked(self.config["axe_y_inverser"])
        options_layout.addWidget(invert_check)
        
        style_check = QCheckBox("Style Matplotlib par Défaut")
        style_check.setChecked(self.config["default_matplotlib_style"])
        options_layout.addWidget(style_check)
        
        options_group.setLayout(options_layout)
        layout.addWidget(options_group)
        
        # Groupe Maximum
        max_group = QGroupBox("Options Maximum")
        max_layout = QVBoxLayout()
        
        max_area_check = QCheckBox("Utiliser Aire Maximum (au lieu de Point)")
        max_area_check.setChecked(self.config.get("max_area", False))
        max_area_check.setToolTip(
            "Coché: utilise largest_moment_areas.json (aire maximum)\n"
            "Décoché: utilise max_span_moments.json (point maximum)"
        )
        max_layout.addWidget(max_area_check)
        
        max_group.setLayout(max_layout)
        layout.addWidget(max_group)
        
        # Boutons
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        button_box.accepted.connect(lambda: self.apply_config_dialog(
            dialog, curve_combo, span_spin.value(), section_spin.value(),
            speed_slider.value() / 1000.0,
            grid_check.isChecked(), travee_check.isChecked(), 
            noeud_check.isChecked(), legend_check.isChecked(),
            invert_check.isChecked(), style_check.isChecked(),
            max_area_check.isChecked()))
        button_box.rejected.connect(dialog.reject)
        layout.addWidget(button_box)
        
        dialog.exec()
    
    def apply_config_dialog(self, dialog, curve_combo, span, section, vitesse, grid, travee, noeud, legend, invert, style, max_area):
        """Applique les changements de configuration depuis la boîte de dialogue"""
        # Mettre à jour le type de courbe
        curve_text = curve_combo.currentText()
        curve_type = curve_text.split(" - ")[0]
        # Trouver l'index dans le ComboBox principal
        for i in range(self.curve_combo.count()):
            if self.curve_combo.itemText(i).startswith(curve_type):
                self.curve_combo.setCurrentIndex(i)
                break
        
        # Mettre à jour span et section
        self.span_spin.setValue(span)
        self.section_spin.setValue(section)
        
        # Mettre à jour la vitesse
        self.config["vitesse_bridge"] = vitesse
        self.speed_slider.setValue(int(vitesse * 1000))
        self.speed_label.setText(f"Vitesse: {vitesse:.3f}")
        
        # Mettre à jour les options d'affichage
        self.config["grid"] = grid
        self.config["travee"] = travee
        self.config["noeud"] = noeud
        self.config["legend"] = legend
        self.config["axe_y_inverser"] = invert
        self.config["default_matplotlib_style"] = style
        self.config["max_area"] = max_area
        
        # Mettre à jour les actions du menu
        self.grid_action.setChecked(grid)
        self.travee_action.setChecked(travee)
        self.noeud_action.setChecked(noeud)
        self.legend_action.setChecked(legend)
        self.invert_action.setChecked(invert)
        self.style_action.setChecked(style)
        
        # Mettre à jour le checkbox du panneau de gauche
        if hasattr(self, 'max_area_check'):
            self.max_area_check.setChecked(max_area)
        
        # Synchroniser avec utils
        self.sync_utils_config()
        
        # Mettre à jour les plages des spinboxes
        self.update_spinbox_ranges()
        
        dialog.accept()
        
    def create_left_panel(self):
        panel = QWidget()
        layout = QVBoxLayout(panel)
        
        title = QLabel("Configuration")
        title.setFont(QFont("Arial", 16, QFont.Weight.Bold))
        layout.addWidget(title)
        
        curve_group = self.create_curve_selection()
        layout.addWidget(curve_group)
        
        span_section_group = self.create_span_section_selection()
        layout.addWidget(span_section_group)
        
        animation_group = self.create_animation_controls()
        layout.addWidget(animation_group)
        
        # Groupe Maximum
        max_group = self.create_maximum_options()
        layout.addWidget(max_group)
        
        layout.addStretch()
        
        return panel
    
    def create_curve_selection(self):
        group = QGroupBox("Type de Courbe")
        layout = QVBoxLayout()
        
        self.curve_combo = QComboBox()
        self.curve_combo.addItems([
            "span_moments - Moments de Travee",
            "span_shear_forces - Forces de Cisaillement",
            "span_rotations - Rotations",
            "span_deflections - Deflexions",
            "support_moments - Moments d'Appui",
            "support_reactions - Reactions d'Appui"
        ])
        self.curve_combo.currentIndexChanged.connect(self.on_curve_changed)
        
        layout.addWidget(self.curve_combo)
        group.setLayout(layout)
        return group
    
    def create_span_section_selection(self):
        group = QGroupBox("Selection Travee & Section")
        layout = QGridLayout()
        
        layout.addWidget(QLabel("Travee (Span):"), 0, 0)
        self.span_spin = QSpinBox()
        self.span_spin.setMinimum(0)
        self.span_spin.setMaximum(100)
        self.span_spin.setValue(0)
        self.span_spin.valueChanged.connect(self.update_spinbox_ranges)
        layout.addWidget(self.span_spin, 0, 1)
        
        layout.addWidget(QLabel("Section:"), 1, 0)
        self.section_spin = QSpinBox()
        self.section_spin.setMinimum(0)
        self.section_spin.setMaximum(100)
        self.section_spin.setValue(0)
        self.section_spin.valueChanged.connect(lambda: None)
        layout.addWidget(self.section_spin, 1, 1)
        
        group.setLayout(layout)
        return group
    
    def create_animation_controls(self):
        group = QGroupBox("Controles d'Animation")
        layout = QVBoxLayout()
        
        button_layout = QHBoxLayout()
        self.play_button = QPushButton("Lecture")
        self.play_button.clicked.connect(self.toggle_animation)
        button_layout.addWidget(self.play_button)
        
        self.reset_button = QPushButton("Reinitialiser")
        self.reset_button.clicked.connect(self.reset_animation)
        button_layout.addWidget(self.reset_button)
        
        layout.addLayout(button_layout)
        
        speed_layout = QVBoxLayout()
        self.speed_label = QLabel(f"Vitesse: {self.config['vitesse_bridge']:.3f}")
        speed_layout.addWidget(self.speed_label)
        
        self.speed_slider = QSlider(Qt.Orientation.Horizontal)
        self.speed_slider.setMinimum(1)
        self.speed_slider.setMaximum(50)
        self.speed_slider.setValue(int(self.config['vitesse_bridge'] * 1000))
        self.speed_slider.valueChanged.connect(self.on_speed_changed)
        speed_layout.addWidget(self.speed_slider)
        
        layout.addLayout(speed_layout)
        
        group.setLayout(layout)
        return group
    
    def create_toolbar(self):
        """Crée une barre d'outils avec des icônes déplaçables"""
        self.main_toolbar = QToolBar("Actions principales")
        self.main_toolbar.setIconSize(QSize(24, 24))  # Petites icônes
        self.main_toolbar.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonIconOnly)
        self.main_toolbar.setMovable(True)  # Permet de déplacer la barre d'outils
        toolbar = self.main_toolbar
        
        # Utiliser des icônes de style standard ou créer des icônes simples
        # Pour PyQt6, on peut utiliser les icônes du thème ou créer des QIcon vides avec du texte
        # Création d'icônes avec du texte en attendant d'avoir de vraies icônes
        
        # Action Type de Courbe
        curve_action = QAction("📈", self)
        curve_action.setToolTip("Type de Courbe")
        curve_action.triggered.connect(self.show_curve_selection_dialog)
        toolbar.addAction(curve_action)
        
        # Action Sélection Travée & Section
        span_section_action = QAction("📍", self)
        span_section_action.setToolTip("Sélection Travée & Section")
        span_section_action.triggered.connect(self.show_span_section_dialog)
        toolbar.addAction(span_section_action)
        
        toolbar.addSeparator()
        
        # Action Tracer
        plot_action = QAction("📊", self)
        plot_action.setToolTip("Tracer (Ctrl+P)")
        plot_action.setShortcut("Ctrl+P")
        plot_action.triggered.connect(self.plot_current_selection)
        toolbar.addAction(plot_action)
        
        toolbar.addSeparator()
        
        # Action Animer Courbe
        animate_action = QAction("🎬", self)
        animate_action.setToolTip("Animer Courbe (Ctrl+A)")
        animate_action.setShortcut("Ctrl+A")
        animate_action.triggered.connect(self.animate_current_selection)
        toolbar.addAction(animate_action)
        
        # Action Animer Complet
        animate_full_action = QAction("🎞️", self)
        animate_full_action.setToolTip("Animer Complet (Ctrl+Shift+A)")
        animate_full_action.setShortcut("Ctrl+Shift+A")
        animate_full_action.triggered.connect(self.animate_full_curve)
        toolbar.addAction(animate_full_action)
        
        toolbar.addSeparator()
        
        # Action Maximum
        max_action = QAction("⬆", self)
        max_action.setToolTip("Afficher Maximum (Ctrl+M)")
        max_action.setShortcut("Ctrl+M")
        max_action.triggered.connect(self.show_maximum)
        toolbar.addAction(max_action)
        
        toolbar.addSeparator()
        
        # Action Sauvegarder
        save_action = QAction("💾", self)
        save_action.setToolTip("Sauvegarder Configuration")
        save_action.triggered.connect(self.save_configuration)
        toolbar.addAction(save_action)
        
        # Action Exporter
        export_action = QAction("📤", self)
        export_action.setToolTip("Exporter Animation (GIF)")
        export_action.triggered.connect(self.export_animation)
        toolbar.addAction(export_action)
        
        toolbar.addSeparator()
        
        # Action Masquer/Afficher Panneau Configuration
        self.toggle_panel_action = QAction("👁️", self)
        self.toggle_panel_action.setToolTip("Masquer/Afficher Panneau Configuration")
        self.toggle_panel_action.triggered.connect(self.toggle_config_panel)
        toolbar.addAction(self.toggle_panel_action)
        
        # Action Configuration
        config_action = QAction("⚙️", self)
        config_action.setToolTip("Options de Configuration")
        config_action.triggered.connect(self.show_config_dialog)
        toolbar.addAction(config_action)
        
        toolbar.addSeparator()
        
        # Action Éditer input.txt
        edit_input_action = QAction("📝", self)
        edit_input_action.setToolTip("Éditer input.txt")
        edit_input_action.triggered.connect(self.edit_input_file)
        toolbar.addAction(edit_input_action)
        
        # Action Lancer le calcul
        run_calc_action = QAction("▶️", self)
        run_calc_action.setToolTip("Lancer le Calcul C++")
        run_calc_action.triggered.connect(self.run_calculation)
        toolbar.addAction(run_calc_action)
        
        toolbar.addSeparator()
        
        # Action Plein écran
        self.fullscreen_action = QAction("🔲", self)
        self.fullscreen_action.setToolTip("Mode Plein Écran (F11)")
        self.fullscreen_action.setShortcut("F11")
        self.fullscreen_action.triggered.connect(self.toggle_fullscreen)
        toolbar.addAction(self.fullscreen_action)
        
        self.addToolBar(Qt.ToolBarArea.TopToolBarArea, toolbar)
    
    def create_right_panel(self):
        panel = QWidget()
        layout = QVBoxLayout(panel)
        
        self.figure = Figure(figsize=(8, 6))
        self.canvas = FigureCanvas(self.figure)
        self.ax = self.figure.add_subplot(111)
        
        self.toolbar = NavigationToolbar(self.canvas, panel)
        
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        
        self.display_placeholder()
        
        return panel
    
    def apply_dark_theme(self):
        self.setStyleSheet("""
            QMainWindow {
                background-color: #2b2b2b;
            }
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QGroupBox {
                border: 2px solid #3d3d3d;
                border-radius: 5px;
                margin-top: 10px;
                font-weight: bold;
                padding-top: 10px;
            }
            QGroupBox::title {
                color: #4a9eff;
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
            }
            QPushButton {
                background-color: #3d3d3d;
                color: #ffffff;
                border: none;
                padding: 8px;
                border-radius: 4px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #4a9eff;
            }
            QPushButton:pressed {
                background-color: #2d7acc;
            }
            QComboBox, QSpinBox {
                background-color: #3d3d3d;
                color: #ffffff;
                border: 1px solid #555555;
                padding: 5px;
                border-radius: 3px;
            }
            QCheckBox {
                spacing: 5px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
            }
            QCheckBox::indicator:unchecked {
                background-color: #3d3d3d;
                border: 2px solid #555555;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background-color: #4a9eff;
                border: 2px solid #4a9eff;
                border-radius: 3px;
            }
            QSlider::groove:horizontal {
                height: 8px;
                background: #3d3d3d;
                border-radius: 4px;
            }
            QSlider::handle:horizontal {
                background: #4a9eff;
                width: 18px;
                margin: -5px 0;
                border-radius: 9px;
            }
            QLabel {
                color: #ffffff;
            }
            QMenuBar {
                background-color: #2b2b2b;
                color: #ffffff;
                border-bottom: 1px solid #3d3d3d;
            }
            QMenuBar::item {
                background-color: transparent;
                padding: 8px 12px;
            }
            QMenuBar::item:selected {
                background-color: #4a9eff;
            }
            QMenu {
                background-color: #2b2b2b;
                color: #ffffff;
                border: 1px solid #3d3d3d;
            }
            QMenu::item {
                padding: 6px 30px 6px 20px;
            }
            QMenu::item:selected {
                background-color: #4a9eff;
            }
            QMenu::separator {
                height: 1px;
                background: #3d3d3d;
                margin: 4px 0;
            }
            QToolBar {
                background-color: #2b2b2b;
                border: 1px solid #3d3d3d;
                spacing: 3px;
            }
            QToolButton {
                background-color: #3d3d3d;
                border: 1px solid #555555;
                border-radius: 3px;
                padding: 4px;
                margin: 2px;
                min-width: 24px;
                min-height: 24px;
                font-size: 16px;
            }
            QToolButton:hover {
                background-color: #4a9eff;
                border-color: #4a9eff;
            }
            QToolButton:pressed {
                background-color: #2d7acc;
            }
            QToolBar::separator {
                background: #3d3d3d;
                width: 1px;
                margin: 4px 2px;
            }
        """)
    
    def setup_shortcuts(self):
        """Configure les raccourcis clavier"""
        # Le raccourci F11 est défini dans la barre d'outils pour éviter les conflits
        pass
    
    def toggle_fullscreen(self):
        """Bascule entre le mode plein écran et le mode fenêtre"""
        if self.is_fullscreen:
            self.showNormal()
            self.is_fullscreen = False
            if hasattr(self, 'fullscreen_action'):
                self.fullscreen_action.setToolTip("Mode Plein Écran (F11)")
        else:
            self.showFullScreen()
            self.is_fullscreen = True
            if hasattr(self, 'fullscreen_action'):
                self.fullscreen_action.setToolTip("Quitter Plein Écran (F11)")

    def load_curve_data(self, curve):
        if curve not in self.data_cache:
            try:
                self.data_cache[curve] = utils.open_json(curve)
            except Exception as exc:
                print(f"Impossible de charger les données pour {curve}: {exc}")
                self.data_cache[curve] = []
        return self.data_cache[curve]
    
    def reload_all_data(self):
        """Recharge toutes les données depuis les fichiers JSON après un nouveau calcul"""
        try:
            # Vider le cache pour forcer le rechargement
            self.data_cache.clear()
            
            # Recharger le module utils pour mettre à jour les variables globales
            importlib.reload(utils)
            
            # Mettre à jour les ranges des spinboxes
            self.update_spinbox_ranges()
            
            # Si un graphique est affiché, le mettre à jour
            if hasattr(self, 'current_curve') and self.current_curve:
                try:
                    self.plot_current_selection()
                except Exception as e:
                    print(f"Erreur lors de la mise à jour du graphique: {e}")
            
            return True
        except Exception as e:
            print(f"Erreur lors du rechargement des données: {e}")
            return False

    def update_spinbox_ranges(self):
        curve = self.get_current_curve_type()
        data = self.load_curve_data(curve)
        span_max = max(0, len(data) - 1) if isinstance(data, list) and len(data) else 0
        self.span_spin.setMaximum(span_max)
        if self.span_spin.value() > span_max:
            self.span_spin.setValue(span_max)
        section_max = self.get_section_limit(data, self.span_spin.value())
        self.section_spin.setMaximum(section_max)
        if self.section_spin.value() > section_max:
            self.section_spin.setValue(section_max)

    def get_section_limit(self, data, span_index):
        try:
            span_data = data[span_index]
            if isinstance(span_data, list) and len(span_data):
                return len(span_data) - 1
        except Exception:
            pass
        return 0

    def get_valid_indices(self, curve):
        data = self.load_curve_data(curve)
        if not data:
            return 0, 0
        span = min(self.span_spin.value(), len(data) - 1)
        section_limit = self.get_section_limit(data, span)
        section = min(self.section_spin.value(), section_limit)
        return span, section

    def get_curve_xy(self, curve, span, section):
        data = self.load_curve_data(curve)
        if not data:
            raise ValueError(f"Aucune donnée disponible pour {curve}")
        if curve == "span_shear_forces":
            x_source = utils.x_forces
            x_values = x_source[span][section]
            y_values = data[span][section]
        elif curve in ["support_reactions"]:
            x_values = utils.x_forces[span][0]
            y_values = data[span]
        elif curve in ["support_moments"]:
            x_values = utils.x_normal
            y_values = data[span]
        elif curve.startswith("span_"):
            x_values = utils.x_normal
            y_values = data[span][section]
        else:
            x_values = utils.x_normal
            y_values = data[span]
        if len(x_values) != len(y_values):
            raise ValueError("Les longueurs des abscisses et ordonnées ne correspondent pas.")
        return x_values, y_values

    def draw_structural_overlay(self, max_x):
        if self.config.get("travee"):
            visible_x = [x for x in utils.x_normal if x <= max_x]
            if visible_x:
                zeros = [0] * len(visible_x)
                self.ax.plot(visible_x, zeros, linestyle='--', alpha=0.4, label='Travées')
        if self.config.get("noeud"):
            visible_nodes = [n for n in utils.neouds if n <= max_x]
            if visible_nodes:
                self.ax.scatter(visible_nodes, [0]*len(visible_nodes), color=self.config["style"].get("noeud_color", "#FF4444"),
                                s=self.config["style"].get("noeud_size", 60), zorder=5, label='Noeuds')

    def stop_animation(self):
        if self.animation:
            try:
                self.animation.event_source.stop()
            except Exception:
                pass
            self.animation = None
        self.animation_line = None
        self.is_playing = False
        self.play_button.setText("Lecture")

    def restart_animation_if_running(self):
        if self.animation:
            self.animate_current_selection()

    def display_placeholder(self):
        self.ax.clear()
        self.ax.text(0.5, 0.5, self.PLACEHOLDER_MESSAGE,
                     ha='center', va='center', transform=self.ax.transAxes, fontsize=14)
        self.canvas.draw()
    
    def get_current_curve_type(self):
        curve_text = self.curve_combo.currentText()
        return curve_text.split(" - ")[0]
    
    def on_curve_changed(self):
        self.update_spinbox_ranges()
    
    def on_speed_changed(self, value):
        self.config['vitesse_bridge'] = value / 1000.0
        self.speed_label.setText(f"Vitesse: {self.config['vitesse_bridge']:.3f}")
        self.sync_utils_config()
        self.restart_animation_if_running()
    
    def update_config(self, key, value):
        self.config[key] = value
        self.sync_utils_config()
    
    def toggle_animation(self):
        if self.animation is None:
            self.animate_current_selection()
            return
        if self.is_playing:
            try:
                self.animation.event_source.stop()
            except Exception:
                pass
            self.play_button.setText("Lecture")
        else:
            try:
                self.animation.event_source.start()
            except Exception:
                pass
            self.play_button.setText("Pause")
        self.is_playing = not self.is_playing
    
    def reset_animation(self):
        self.stop_animation()
        self.display_placeholder()
    
    def plot_directly(self, curve, span, section):
        """Trace directement avec self.ax sans passer par func_plot.py"""
        self.ax.clear()
        
        # Configurer les labels de l'axe X
        self.ax.set_xticks(utils.neouds)
        self.ax.set_xticklabels(utils.distances)
        plt.setp(self.ax.get_xticklabels(), rotation=45)
        
        # Dessiner les éléments structurels
        if self.config.get("travee"):
            self.ax.plot(utils.x_normal, [0]*len(utils.x_normal), 'k--', alpha=0.5, label='Travées')
        if self.config.get("noeud"):
            self.ax.scatter(utils.neouds, [0]*len(utils.neouds), color=self.config["style"].get("noeud_color", "#FF4444"),
                          s=self.config["style"].get("noeud_size", 60), zorder=5, label='Noeuds')
        
        # Tracer la courbe
        x_values, y_values = self.get_curve_xy(curve, span, section)
        
        # Générer le label approprié selon le type de courbe
        if curve == "support_moments":
            label = f"M_{span}"
        elif curve == "support_reactions":
            label = f"R_{span}"
        else:
            label = f"Travee : {span+1}\nSection : {section}"
        self.ax.plot(x_values, y_values, label=label)
        
        # Configurer les axes et légende
        if self.config["legend"]:
            self.ax.set_title(curve.upper(), fontdict=utils.text_format)
            self.ax.set_xlabel("Distance entre les appuis".upper(), fontdict=utils.text_format)
            curve_type = curve.split("_")[1] if "_" in curve else "values"
            self.ax.set_ylabel(f"values of {curve_type.upper()}", fontdict=utils.text_format)
            self.ax.legend()
        
        if self.config["grid"]:
            self.ax.grid(True)
        if self.config["axe_y_inverser"]:
            self.ax.invert_yaxis()
                
    def plot_current_selection(self):
        try:
            self.stop_animation()
            curve = self.get_current_curve_type()
            span, section = self.get_valid_indices(curve)
            self.persist_configuration(silent=True)
            
            # Utiliser notre méthode directe au lieu de func_plot.py
            self.plot_directly(curve, span, section)
            self.canvas.draw()
            
        except Exception as e:
            self.ax.clear()
            self.ax.text(0.5, 0.5, f"Erreur: {str(e)}", 
                        ha='center', va='center', transform=self.ax.transAxes, color='red')
            print(f"Erreur: {str(e)}")
            import traceback
            traceback.print_exc()
            self.canvas.draw()
    
    def animate_current_selection(self):
        try:
            curve = self.get_current_curve_type()
            span, section = self.get_valid_indices(curve)
            x_values, y_values = self.get_curve_xy(curve, span, section)
            if len(x_values) < 2:
                raise ValueError("Nombre de points insuffisant pour animer cette courbe.")
            
            self.persist_configuration(silent=True)
            self.stop_animation()
            self.ax.clear()
            
            max_x = max(x_values)
            self.draw_structural_overlay(max_x)
            
            if self.config.get("grid"):
                self.ax.grid(True)
            
            self.ax.set_title(f"Animation - {curve}")
            self.ax.set_xlabel("Position")
            self.ax.set_ylabel("Valeur")
            self.ax.set_xlim(min(x_values), max_x)
            y_min = min(y_values)
            y_max = max(y_values)
            if y_min == y_max:
                delta = abs(y_min) if y_min != 0 else 1
                y_min -= delta
                y_max += delta
            padding = (y_max - y_min) * 0.1
            self.ax.set_ylim(y_min - padding, y_max + padding)
            if self.config.get("axe_y_inverser"):
                self.ax.invert_yaxis()
            
            (self.animation_line,) = self.ax.plot([], [], color=self.config["style"].get("line_color", "royalblue"),
                                                  linewidth=self.config["style"].get("line_width", 2.0))
            
            def init_line():
                self.animation_line.set_data([], [])
                return self.animation_line,
            
            def update_line(frame):
                self.animation_line.set_data(x_values[:frame], y_values[:frame])
                return self.animation_line,
            
            frames = range(1, len(x_values) + 1)
            interval = max(1, int(self.config["vitesse_bridge"] * 1000))
            self.animation = FuncAnimation(
                self.figure,
                update_line,
                frames=frames,
                init_func=init_line,
                interval=interval,
                blit=False,
                repeat=True
            )
            self.is_playing = True
            self.play_button.setText("Pause")
            self.canvas.draw()
        except Exception as e:
            QMessageBox.critical(self, "Erreur", f"Erreur lors de l'animation: {e}")
    
    def prepare_animation_frames(self, curve):
        """Prépare toutes les frames pour l'animation complète (comme animate.py)"""
        # Charger les données selon le type de courbe
        if curve == "span_shear_forces":
            shear_abscissas = np.array(utils.open_json("shear_abscissas"))
            curvature = utils.open_json(curve)
            frames = []
            x_coords = []
            # Aplatir la structure imbriquée en gardant les abscisses correspondantes
            for span_idx, span in enumerate(curvature):
                for section_idx, section in enumerate(span):
                    frames.append(section)
                    x_coords.append(shear_abscissas[span_idx][section_idx])
            x_normal = x_coords  # Stocker toutes les coordonnées x
        else:
            x_normal = np.array(utils.open_json())  # Par défaut total_abscissas
            curvature = utils.open_json(curve)
            frames = []
            for span in curvature:
                frames.extend(span)
        
        # Ajuster x_normal pour commencer à 0
        if curve == "span_shear_forces":
            # Pour les forces de cisaillement, chaque frame a ses propres valeurs x_normal
            x_normal = [x - min(x) for x in x_normal]
        else:
            x_normal = x_normal - min(x_normal)  # Décaler pour que le minimum soit 0
        
        return frames, x_normal, curve
    
    def animate_full_curve(self):
        """Animation complète de toutes les frames d'une courbe (comme animate.py)"""
        try:
            curve = self.get_current_curve_type()
            self.persist_configuration(silent=True)
            self.stop_animation()
            
            # Préparer les frames
            frames, x_normal, curve_type = self.prepare_animation_frames(curve)
            if not frames:
                raise ValueError("Aucune frame disponible pour cette courbe.")
            
            # Charger les données supplémentaires
            try:
                max_data = utils.open_json("max_" + curve_type, "analysis")
                absolute_max = max_data["valeur"]
            except:
                absolute_max = max(max(frame) for frame in frames) if frames else 1
            
            neouds = np.array(utils.neouds)
            distances = [f"{round(n, 5)}" for n in neouds]
            
            # Calculer les limites y globales
            y_min = min(min(frame) for frame in frames) if frames else -1
            y_max = max(max(frame) for frame in frames) if frames else 1
            y_max = absolute_max * 1.1
            y_min = -absolute_max * 1.1
            
            self.ax.clear()
            
            # Fonction d'initialisation
            def init_animation():
                self.ax.clear()
                if self.config.get("grid"):
                    self.ax.grid(True)
                
                # Configurer les labels X
                self.ax.set_xticks(neouds)
                self.ax.set_xticklabels(distances)
                plt.setp(self.ax.get_xticklabels(), rotation=45)
                
                # Définir les limites X
                if curve_type == "span_shear_forces":
                    all_x = np.concatenate(x_normal) if isinstance(x_normal, list) else x_normal
                    self.ax.set_xlim(min(all_x), max(all_x))
                else:
                    self.ax.set_xlim(0, max(x_normal))
                
                self.ax.set_ylim(y_min, y_max)
                if self.config.get("axe_y_inverser"):
                    self.ax.invert_yaxis()
                    
                return []
            
            # Fonction d'animation
            def update_animation(frame_index):
                self.ax.clear()
                if self.config.get("grid"):
                    self.ax.grid(True)
                
                # Configurer les labels X
                self.ax.set_xticks(neouds)
                self.ax.set_xticklabels(distances)
                plt.setp(self.ax.get_xticklabels(), rotation=45)
                
                # Calculer le progrès (0 à 1)
                progress = frame_index / len(frames) if len(frames) > 0 else 1.0
                
                # Dessiner les éléments structurels avec progression
                self.draw_animation_structural_elements(progress, x_normal, neouds, curve_type)
                
                # Dessiner les lignes horizontales de maximum
                self.ax.axhline(y=absolute_max, color='r', linestyle='--', alpha=0.5, 
                               label=f'Max: {absolute_max:.2f}')
                self.ax.axhline(y=-absolute_max, color='r', linestyle='--', alpha=0.5)
                
                # Tracer la courbe jusqu'à la frame actuelle
                if frame_index < len(frames):
                    if curve_type == "span_shear_forces":
                        current_x = x_normal[frame_index] if isinstance(x_normal, list) else x_normal
                        current_frame = frames[frame_index]
                    else:
                        current_x = x_normal
                        current_frame = frames[frame_index]
                    
                    line_color = self.config["style"].get("line_color", "royalblue") if not self.config.get("default_matplotlib_style") else None
                    line_width = self.config["style"].get("line_width", 2.0) if not self.config.get("default_matplotlib_style") else None
                    
                    if line_color and line_width:
                        self.ax.plot(current_x, current_frame, color=line_color, 
                                   linewidth=line_width, label=curve_type)
                    else:
                        self.ax.plot(current_x, current_frame, label=curve_type)
                
                # Configurer les labels
                if self.config.get("default_matplotlib_style"):
                    self.ax.set_xlabel("Longueur des travées")
                    self.ax.set_ylabel(curve_type.split("_")[1] if "_" in curve_type else "Valeur")
                else:
                    self.ax.set_xlabel(self.config["style"].get("xlabel", "Longueur des travées"))
                    self.ax.set_ylabel(self.config["style"].get("ylabel", "Valeur"))
                
                # Définir les limites
                if curve_type == "span_shear_forces":
                    all_x = np.concatenate(x_normal) if isinstance(x_normal, list) else x_normal
                    self.ax.set_xlim(min(all_x), max(all_x))
                else:
                    self.ax.set_xlim(0, max(x_normal))
                
                self.ax.set_ylim(y_min, y_max)
                if self.config.get("axe_y_inverser"):
                    self.ax.invert_yaxis()
                
                if self.config.get("legend"):
                    self.ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
                
                self.figure.subplots_adjust(right=0.85)
                self.canvas.draw()
                return []
            
            # Créer l'animation
            interval = max(1, int(self.config["vitesse_bridge"] * 1000))
            self.animation = FuncAnimation(
                self.figure,
                update_animation,
                frames=len(frames),
                init_func=init_animation,
                interval=interval,
                blit=False,
                repeat=True
            )
            
            self.is_playing = True
            self.play_button.setText("Pause")
            self.canvas.draw()
            
        except Exception as e:
            QMessageBox.critical(self, "Erreur", f"Erreur lors de l'animation complète: {e}")
            print(f"Erreur animate_full_curve: {e}")
            import traceback
            traceback.print_exc()
    
    def draw_animation_structural_elements(self, progress, x_normal, neouds, curve_type):
        """Dessine les éléments structurels avec progression pour l'animation complète"""
        # Dessiner les travées si activées
        if self.config.get("travee"):
            if curve_type == "span_shear_forces":
                all_x = np.concatenate(x_normal) if isinstance(x_normal, list) else x_normal
                max_x = max(all_x)
                min_x = min(all_x)
                x_range = np.linspace(min_x, max_x, 100)
                visible_x = x_range[x_range <= max_x * progress]
            else:
                max_x = max(x_normal)
                cutoff = max_x * progress
                visible_x = x_normal[x_normal <= cutoff]
            
            if len(visible_x) > 0:
                visible_zeros = np.zeros(len(visible_x))
                if self.config.get("default_matplotlib_style"):
                    self.ax.plot(visible_x, visible_zeros, linestyle='--', alpha=0.5, label='Travées')
                else:
                    edge_color = self.config["style"].get("edge_color", "#2C3E50")
                    line_width = self.config["style"].get("line_width", 2.5) / 2
                    self.ax.plot(visible_x, visible_zeros, color=edge_color, linestyle='--', 
                               linewidth=line_width, alpha=0.5, label='Travées')
        
        # Dessiner les nœuds si activés
        if self.config.get("noeud"):
            if curve_type == "span_shear_forces":
                all_x = np.concatenate(x_normal) if isinstance(x_normal, list) else x_normal
                max_x = max(all_x)
                visible_nodes = neouds[neouds <= max_x * progress]
            else:
                max_x = max(x_normal)
                visible_nodes = neouds[neouds <= max_x * progress]
            
            if len(visible_nodes) > 0:
                if self.config.get("default_matplotlib_style"):
                    self.ax.scatter(visible_nodes, [0]*len(visible_nodes), label='Noeuds')
                else:
                    noeud_color = self.config["style"].get("noeud_color", "#FF4444")
                    noeud_size = self.config["style"].get("noeud_size", 100)
                    self.ax.scatter(visible_nodes, [0]*len(visible_nodes), color=noeud_color,
                                  s=noeud_size, zorder=5, label='Noeuds')
    
    def save_configuration(self):
        self.persist_configuration(silent=False)
    
    def export_animation(self):
        if not self.animation:
            QMessageBox.information(self, "Export", "Veuillez d'abord lancer une animation.")
            return
        file_path, _ = QFileDialog.getSaveFileName(self, "Exporter l'animation", "animation.gif", "GIF (*.gif)")
        if not file_path:
            return
        try:
            self.animation.save(file_path, writer="pillow")
            QMessageBox.information(self, "Export", f"Animation exportée vers {file_path}")
        except Exception as exc:
            QMessageBox.critical(self, "Erreur", f"Echec de l'export: {exc}")
    
    def create_maximum_options(self):
        """Crée le groupe d'options pour le maximum"""
        group = QGroupBox("Options Maximum")
        layout = QVBoxLayout()
        
        # Checkbox pour choisir entre area et point
        self.max_area_check = QCheckBox("Utiliser Aire Maximum (au lieu de Point)")
        # Initialiser avec la valeur de la configuration, par défaut False (point)
        max_area_value = self.config.get("max_area", True)
        self.max_area_check.setChecked(max_area_value)
        self.max_area_check.setToolTip(
            "Coché: utilise largest_moment_areas.json (aire maximum)\n"
            "Décoché: utilise max_span_moments.json (point maximum)"
        )
        self.max_area_check.stateChanged.connect(self.on_max_area_changed)
        layout.addWidget(self.max_area_check)
        
        group.setLayout(layout)
        return group
    
    def on_max_area_changed(self, state):
        """Appelé quand l'option max_area change"""
        self.config["max_area"] = (state == Qt.CheckState.Checked.value)
        self.persist_configuration(silent=True)
    
    def show_maximum(self, area=None):
        """
        Affiche le maximum selon func_plot.py MAX()
        area=None: utilise la valeur de self.config["max_area"]
        area=False: utilise max_span_moments.json (point maximum)
        area=True: utilise largest_moment_areas.json (aire maximum)
        """
        try:
            self.stop_animation()
            curve = self.get_current_curve_type()
            self.persist_configuration(silent=True)
            
            # Utiliser la valeur de la configuration si area n'est pas spécifié
            if area is None:
                area = self.config.get("max_area", False)
            print(area)
            # Déterminer span et section pour le maximum selon func_plot.py MAX()
            if curve == "support_moments":
                if area:
                    # Utiliser l'aire maximum pour support_moments
                    span = utils.max_support_areas
                else:
                    # Utiliser le point maximum depuis max_support_moments.json
                    span = utils.max_support_point["appuis"]
                section = 0  # Non applicable pour support_moments
            else:
                if area:
                    # Utiliser l'aire maximum depuis largest_moment_areas.json
                    max_areas_data = utils.open_json("largest_moment_areas", "analysis")
                    max_area_info = max_areas_data["plus_grande_aire"][0]
                    span = max_area_info["travee"]
                    section = max_area_info["section"]
                else:
                    # Utiliser le point maximum depuis max_span_moments.json
                    # Pour les autres courbes, on essaie d'utiliser le fichier max correspondant
                    try:
                        if curve == "span_moments":
                            max_point_data = utils.max_M_point
                        else:
                            # Essayer d'ouvrir le fichier max correspondant
                            max_point_data = utils.open_json(f"max_{curve}", "analysis")
                        span = max_point_data["index_travee"]
                        section = max_point_data["index_section"]
                    except (KeyError, FileNotFoundError, AttributeError):
                        # Si le fichier n'existe pas, utiliser max_span_moments comme fallback
                        max_point_data = utils.max_M_point
                        span = max_point_data["index_travee"]
                        section = max_point_data["index_section"]
            
            # Tracer directement avec les indices de maximum
            self.plot_directly(curve, span, section)
            self.canvas.draw()
            
        except Exception as e:
            QMessageBox.critical(self, "Erreur", f"Erreur lors de l'affichage du maximum: {e}")
            print(f"Erreur show_maximum: {e}")
            import traceback
            traceback.print_exc()
    
    def show_curve_selection_dialog(self):
        """Affiche une boîte de dialogue pour sélectionner le type de courbe"""
        dialog = QDialog(self)
        dialog.setWindowTitle("Type de Courbe")
        dialog.setModal(True)
        dialog.resize(350, 200)
        
        layout = QVBoxLayout(dialog)
        
        label = QLabel("Sélectionnez le type de courbe:")
        layout.addWidget(label)
        
        curve_combo = QComboBox()
        curve_combo.addItems([
            "span_moments - Moments de Travee",
            "span_shear_forces - Forces de Cisaillement",
            "span_rotations - Rotations",
            "span_deflections - Deflexions",
            "support_moments - Moments d'Appui",
            "support_reactions - Reactions d'Appui"
        ])
        # Trouver l'index de la courbe actuelle
        current_curve = self.get_current_curve_type()
        for i in range(curve_combo.count()):
            if curve_combo.itemText(i).startswith(current_curve):
                curve_combo.setCurrentIndex(i)
                break
        layout.addWidget(curve_combo)
        
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        button_box.accepted.connect(lambda: self.apply_curve_selection(dialog, curve_combo))
        button_box.rejected.connect(dialog.reject)
        layout.addWidget(button_box)
        
        dialog.exec()
    
    def apply_curve_selection(self, dialog, curve_combo):
        """Applique la sélection du type de courbe"""
        curve_text = curve_combo.currentText()
        curve_type = curve_text.split(" - ")[0]
        # Trouver l'index dans le ComboBox principal
        for i in range(self.curve_combo.count()):
            if self.curve_combo.itemText(i).startswith(curve_type):
                self.curve_combo.setCurrentIndex(i)
                break
        self.update_spinbox_ranges()
        dialog.accept()
    
    def show_span_section_dialog(self):
        """Affiche une boîte de dialogue pour sélectionner travée et section"""
        dialog = QDialog(self)
        dialog.setWindowTitle("Sélection Travée & Section")
        dialog.setModal(True)
        dialog.resize(300, 150)
        
        layout = QVBoxLayout(dialog)
        
        grid_layout = QGridLayout()
        
        grid_layout.addWidget(QLabel("Travée (Span):"), 0, 0)
        span_spin = QSpinBox()
        span_spin.setMinimum(0)
        span_spin.setMaximum(100)
        span_spin.setValue(self.span_spin.value())
        grid_layout.addWidget(span_spin, 0, 1)
        
        grid_layout.addWidget(QLabel("Section:"), 1, 0)
        section_spin = QSpinBox()
        section_spin.setMinimum(0)
        section_spin.setMaximum(100)
        section_spin.setValue(self.section_spin.value())
        grid_layout.addWidget(section_spin, 1, 1)
        
        layout.addLayout(grid_layout)
        
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        button_box.accepted.connect(lambda: self.apply_span_section(dialog, span_spin.value(), section_spin.value()))
        button_box.rejected.connect(dialog.reject)
        layout.addWidget(button_box)
        
        dialog.exec()
    
    def apply_span_section(self, dialog, span, section):
        """Applique la sélection de travée et section"""
        self.span_spin.setValue(span)
        self.section_spin.setValue(section)
        self.update_spinbox_ranges()
        dialog.accept()
    
    def toggle_config_panel(self):
        """Masque ou affiche le panneau de configuration"""
        if self.left_panel_visible:
            self.left_panel.hide()
            self.left_panel_visible = False
            self.toggle_panel_action.setText("👁️‍🗨️")
            self.toggle_panel_action.setToolTip("Afficher Panneau Configuration")
        else:
            self.left_panel.show()
            self.left_panel_visible = True
            self.toggle_panel_action.setText("👁️")
            self.toggle_panel_action.setToolTip("Masquer Panneau Configuration")
    
    def set_config_panel_width(self, width):
        """Définit la largeur du panneau de configuration en pixels"""
        if self.left_panel_visible and hasattr(self, 'splitter'):
            sizes = self.splitter.sizes()
            if sizes and len(sizes) >= 2:
                total_width = sum(sizes)
                # S'assurer que la largeur est dans des limites raisonnables
                width = max(200, min(width, total_width - 400))  # Min 200px, max total-400px
                new_sizes = [width, total_width - width]
                self.splitter.setSizes(new_sizes)
    
    def toggle_toolbar(self, checked):
        """Affiche ou masque la barre d'outils"""
        if hasattr(self, 'main_toolbar'):
            if checked:
                self.main_toolbar.show()
            else:
                self.main_toolbar.hide()
            self.toolbar_visible_action.setChecked(checked)
    
    def find_input_file(self):
        """Trouve le fichier input.txt dans les emplacements possibles"""
        possible_paths = [
            Path(__file__).resolve().with_name("input.txt"),  # Dans le même dossier que le script
            Path(__file__).resolve().parent.parent / "input.txt",  # Dans livrable/
            Path(__file__).resolve().parent.parent.parent / "Ligne d'influence" / "input.txt",  # Dans le projet principal
        ]
        
        for path in possible_paths:
            if path.exists():
                return path
        return None
    
    def edit_input_file(self):
        """Ouvre une boîte de dialogue pour éditer le fichier input.txt"""
        input_path = self.find_input_file()
        
        if not input_path:
            QMessageBox.warning(self, "Fichier introuvable", 
                              "Le fichier input.txt n'a pas été trouvé.\n"
                              "Veuillez vérifier qu'il existe dans le répertoire du projet.")
            return
        
        dialog = QDialog(self)
        dialog.setWindowTitle(f"Éditer input.txt - {input_path.name}")
        dialog.setModal(True)
        dialog.resize(700, 600)
        
        layout = QVBoxLayout(dialog)
        
        # Label avec le chemin du fichier
        path_label = QLabel(f"Fichier: {input_path}")
        path_label.setWordWrap(True)
        layout.addWidget(path_label)
        
        # Éditeur de texte
        text_editor = QTextEdit()
        text_editor.setFont(QFont("Courier New", 10))
        text_editor.setStyleSheet("""
            QTextEdit {
                background-color: #1e1e1e;
                color: #d4d4d4;
                border: 1px solid #3d3d3d;
            }
        """)
        
        # Charger le contenu du fichier
        try:
            with open(input_path, "r", encoding="utf-8") as file:
                content = file.read()
            text_editor.setPlainText(content)
        except Exception as e:
            QMessageBox.critical(self, "Erreur", f"Impossible de lire le fichier: {e}")
            dialog.reject()
            return
        
        layout.addWidget(text_editor)
        
        # Boutons
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Save | 
                                     QDialogButtonBox.StandardButton.Cancel)
        button_box.button(QDialogButtonBox.StandardButton.Save).setText("Sauvegarder")
        button_box.accepted.connect(lambda: self.save_input_file(dialog, input_path, text_editor.toPlainText()))
        button_box.rejected.connect(dialog.reject)
        layout.addWidget(button_box)
        
        dialog.exec()
    
    def save_input_file(self, dialog, file_path, content):
        """Sauvegarde le contenu modifié dans input.txt"""
        try:
            with open(file_path, "w", encoding="utf-8") as file:
                file.write(content)
            QMessageBox.information(self, "Succès", 
                                  f"Le fichier {file_path.name} a été sauvegardé avec succès.\n\n"
                                  "Note: Vous devrez peut-être relancer le calcul C++ pour que les changements prennent effet.")
            dialog.accept()
        except Exception as e:
            QMessageBox.critical(self, "Erreur", f"Impossible de sauvegarder le fichier: {e}")
    
    def find_executable(self):
        """Trouve l'exécutable C++ dans les emplacements possibles"""
        possible_paths = [
            Path(__file__).resolve().with_name("Ligne d'influence.exe"),  # Dans le même dossier que le script
            Path(__file__).resolve().parent.parent / "Ligne d'influence.exe",  # Dans livrable/
            Path(__file__).resolve().parent.parent.parent / "Ligne d'influence" / "Ligne d'influence.exe",  # Dans le projet principal
            Path(__file__).resolve().parent.parent.parent / "bin" / "x64" / "Debug" / "Ligne d'influence.exe",  # Dans bin/Debug
        ]
        
        for path in possible_paths:
            if path.exists():
                return path
        return None
    
    def run_calculation(self):
        """Lance l'exécutable C++ pour effectuer le calcul"""
        exe_path = self.find_executable()
        
        if not exe_path:
            QMessageBox.warning(self, "Exécutable introuvable", 
                              "L'exécutable 'Ligne d'influence.exe' n'a pas été trouvé.\n\n"
                              "Veuillez vérifier qu'il existe dans l'un des emplacements suivants:\n"
                              "- Dans le dossier plotting/\n"
                              "- Dans le dossier livrable/\n"
                              "- Dans le dossier du projet principal")
            return
        
        # Demander confirmation
        reply = QMessageBox.question(self, "Lancer le Calcul", 
                                    f"Voulez-vous lancer le calcul avec:\n{exe_path}\n\n"
                                    "Assurez-vous que input.txt est à jour.",
                                    QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
        
        if reply == QMessageBox.StandardButton.Yes:
            try:
                # Lancer l'exécutable dans le répertoire où se trouve input.txt
                input_path = self.find_input_file()
                working_dir = input_path.parent if input_path else exe_path.parent
                
                # Afficher un message de démarrage
                QMessageBox.information(self, "Calcul en Cours", 
                                      f"Le calcul a été lancé.\n\n"
                                      f"Exécutable: {exe_path.name}\n"
                                      f"Répertoire: {working_dir}\n\n"
                                      "Veuillez patienter, un message s'affichera lorsque le calcul sera terminé.")
                
                # Créer un thread Qt et un worker pour exécuter le calcul
                self.calc_thread = QThread()
                self.calc_worker = CalculationWorker()
                self.calc_worker.moveToThread(self.calc_thread)
                
                # Connecter les signaux
                self.calc_thread.started.connect(lambda: self.calc_worker.run_calculation(exe_path, working_dir))
                self.calc_worker.finished.connect(self._show_calculation_success)
                self.calc_worker.error.connect(self._show_calculation_error)
                self.calc_worker.finished.connect(self.calc_thread.quit)
                self.calc_worker.error.connect(self.calc_thread.quit)
                self.calc_thread.finished.connect(self.calc_thread.deleteLater)
                
                # Démarrer le thread
                self.calc_thread.start()
                
            except Exception as e:
                QMessageBox.critical(self, "Erreur", 
                                   f"Impossible de lancer l'exécutable:\n{e}")
    
    def _show_calculation_success(self, exe_name: str, working_dir: str):
        """Affiche le message de succès du calcul (appelé depuis le thread principal)"""
        # Recharger les données après le calcul
        self.reload_all_data()
        
        QMessageBox.information(self, "Calcul Terminé", 
                              f"✅ Le calcul s'est terminé avec succès!\n\n"
                              f"Exécutable: {exe_name}\n"
                              f"Répertoire: {working_dir}\n\n"
                              "Les résultats sont maintenant disponibles dans:\n"
                              "data/results/\n\n"
                              "Les données ont été rechargées automatiquement.\n"
                              "Vous pouvez maintenant visualiser les résultats.")
    
    def _show_calculation_warning(self, error_msg: str):
        """Affiche le message d'avertissement du calcul (appelé depuis le thread principal)"""
        QMessageBox.warning(self, "Calcul Terminé avec Avertissements", 
                          f"⚠️ Le calcul s'est terminé avec des avertissements.\n\n"
                          f"Erreurs:\n{error_msg}\n\n"
                          "Vérifiez les résultats dans data/results/")
    
    def _show_calculation_timeout(self):
        """Affiche le message de timeout du calcul (appelé depuis le thread principal)"""
        QMessageBox.warning(self, "Calcul Interrompu", 
                          "⏱️ Le calcul a pris trop de temps (>5 minutes) et a été interrompu.")
    
    def _show_calculation_error(self, error: str):
        """Affiche le message d'erreur du calcul (appelé depuis le thread principal)"""
        QMessageBox.critical(self, "Erreur lors du Calcul", 
                           f"❌ Une erreur s'est produite pendant le calcul:\n{error}")


def main():
    app = QApplication(sys.argv)
    window = StructuralAnalysisGUI()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()