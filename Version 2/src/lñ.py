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
                             QSplitter, QTextEdit, QFrame)
from PyQt6.QtCore import Qt, QSize, QThread, pyqtSignal, QMetaObject, QTimer, QObject
from PyQt6.QtGui import QFont, QShortcut, QKeySequence, QAction, QIcon, QPixmap
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure
import numpy as np
import os 
import src.utils as utils
from utils_modules.help_content import HELP_CONTENT_HTML, ABOUT_CONTENT_HTML
from utils_modules.styles import DARK_THEME_STYLESHEET

# Charger les thèmes depuis le fichier JSON
def load_themes():
    """Charge les thèmes depuis le fichier themes.json"""
    try:
        themes_path = Path(os.getcwd() + "/config/themes.json")
        if themes_path.exists():
            with open(themes_path, "r", encoding="utf-8") as f:
                return json.load(f)
    except Exception as e:
        print(f"Erreur lors du chargement des thèmes: {e}")
    return None



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
        
        # Ajouter l'icône de l'application
        icon_path = Path(os.getcwd() + "/assets/istockphoto-1077739826-1024x1024.ico" )
        if icon_path.exists():
            self.setWindowIcon(QIcon(str(icon_path)))
        
        self.config_path = Path(os.getcwd() + "/config/Configuration.json")
        self.config = self.load_configuration()
        
        # Charger les thèmes
        self.themes_data = load_themes()
        self.current_theme = "dark"
        
        self.is_playing = False
        self.animation = None
        self.animation_line = None
        self.data_cache = {}
        self.is_fullscreen = False
        
        # Variables pour l'animation de transition
        self.transition_timer = None
        self.transition_alpha = 1.0
        self.transition_elements = []
        
        self.setup_ui()
        self.apply_theme(self.current_theme)
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
        
        # ===== Menu Fichier =====
        file_menu = menubar.addMenu("📁 Fichier")
        
        # Sous-menu Édition
        edit_submenu = file_menu.addMenu("✏️ Édition")
        edit_input_action = edit_submenu.addAction("Éditer input.txt")
        edit_input_action.triggered.connect(self.edit_input_file)
        
        # Sous-menu Calcul
        calc_submenu = file_menu.addMenu("⚙️ Calcul")
        run_calc_action = calc_submenu.addAction("Lancer le Calcul C++")
        run_calc_action.triggered.connect(self.run_calculation)
        
        file_menu.addSeparator()
        
        # Sous-menu Sauvegarde/Export
        save_submenu = file_menu.addMenu("💾 Sauvegarde & Export")
        save_config_action = save_submenu.addAction("Sauvegarder Configuration")
        save_config_action.setShortcut("Ctrl+S")
        save_config_action.triggered.connect(self.save_configuration)
        save_submenu.addSeparator()
        export_action = save_submenu.addAction("Exporter Animation (GIF)")
        export_action.setShortcut("Ctrl+E")
        export_action.triggered.connect(self.export_animation)
        
        file_menu.addSeparator()
        quit_action = file_menu.addAction("❌ Quitter")
        quit_action.setShortcut("Ctrl+Q")
        quit_action.triggered.connect(self.close)
        
        # ===== Menu Visualisation =====
        view_menu = menubar.addMenu("📊 Visualisation")
        
        plot_action = view_menu.addAction("📈 Tracer")
        plot_action.setShortcut("Ctrl+P")
        plot_action.triggered.connect(self.plot_current_selection)
        
        view_menu.addSeparator()
        
        # Sous-menu Animation
        animate_submenu = view_menu.addMenu("🎬 Animation")
        animate_action = animate_submenu.addAction("Animer (Courbe Sélectionnée)")
        animate_action.setShortcut("Ctrl+A")
        animate_action.triggered.connect(self.animate_current_selection)
        animate_full_action = animate_submenu.addAction("Animer (Complet)")
        animate_full_action.setShortcut("Ctrl+Shift+A")
        animate_full_action.triggered.connect(self.animate_full_curve)
        
        view_menu.addSeparator()
        
        max_action = view_menu.addAction("⭐ Afficher Maximum")
        max_action.setShortcut("Ctrl+M")
        max_action.triggered.connect(self.show_maximum)
        
        # ===== Menu Configuration =====
        config_menu = menubar.addMenu("⚙️ Configuration")
        
        # Sous-menu Affichage
        display_submenu = config_menu.addMenu("👁️ Affichage")
        
        self.grid_action = display_submenu.addAction("Grille")
        self.grid_action.setCheckable(True)
        self.grid_action.setChecked(self.config["grid"])
        self.grid_action.triggered.connect(lambda checked: self.toggle_option("grid", checked))
        
        self.travee_action = display_submenu.addAction("Travées")
        self.travee_action.setCheckable(True)
        self.travee_action.setChecked(self.config["travee"])
        self.travee_action.triggered.connect(lambda checked: self.toggle_option("travee", checked))
        
        self.noeud_action = display_submenu.addAction("Nœuds")
        self.noeud_action.setCheckable(True)
        self.noeud_action.setChecked(self.config["noeud"])
        self.noeud_action.triggered.connect(lambda checked: self.toggle_option("noeud", checked))
        
        self.legend_action = display_submenu.addAction("Légende")
        self.legend_action.setCheckable(True)
        self.legend_action.setChecked(self.config["legend"])
        self.legend_action.triggered.connect(lambda checked: self.toggle_option("legend", checked))
        
        # Sous-menu Axes
        axes_submenu = config_menu.addMenu("📐 Axes")
        
        self.invert_action = axes_submenu.addAction("Inverser l'Axe Y")
        self.invert_action.setCheckable(True)
        self.invert_action.setChecked(self.config["axe_y_inverser"])
        self.invert_action.triggered.connect(lambda checked: self.toggle_option("axe_y_inverser", checked))
        
        # Sous-menu Style
        style_submenu = config_menu.addMenu("🎨 Style")
        
        self.style_action = style_submenu.addAction("Style Matplotlib par Défaut")
        self.style_action.setCheckable(True)
        self.style_action.setChecked(self.config["default_matplotlib_style"])
        self.style_action.triggered.connect(lambda checked: self.toggle_option("default_matplotlib_style", checked))
        
        # Sous-menu Thèmes
        theme_submenu = config_menu.addMenu("🎭 Thèmes")
        if self.themes_data and "themes" in self.themes_data:
            for theme_name, theme_data in self.themes_data["themes"].items():
                theme_action = theme_submenu.addAction(theme_data.get("name", theme_name))
                theme_action.triggered.connect(lambda checked, t=theme_name: self.apply_theme(t))
                theme_action.setCheckable(True)
                theme_action.setChecked(theme_name == self.current_theme)
        
        # Sous-menu Automatisation
        automation_submenu = config_menu.addMenu("⚡ Automatisation")
        
        self.auto_report_action = automation_submenu.addAction("Générer Rapport Automatiquement")
        self.auto_report_action.setCheckable(True)
        self.auto_report_action.setChecked(True)
        self.auto_report_action.triggered.connect(self.toggle_auto_report)
        
        self.auto_reload_action = automation_submenu.addAction("Recharger Données Automatiquement")
        self.auto_reload_action.setCheckable(True)
        self.auto_reload_action.setChecked(True)
        self.auto_reload_action.triggered.connect(self.toggle_auto_reload)
        
        # ===== Menu Vue =====
        window_menu = menubar.addMenu("🪟 Vue")
        
        self.toolbar_visible_action = window_menu.addAction("Afficher la Barre d'Outils")
        self.toolbar_visible_action.setCheckable(True)
        self.toolbar_visible_action.setChecked(True)
        self.toolbar_visible_action.triggered.connect(self.toggle_toolbar)
        
        window_menu.addSeparator()
        
        fullscreen_action = window_menu.addAction("Mode Plein Écran (F11)")
        fullscreen_action.setShortcut("F11")
        fullscreen_action.triggered.connect(self.toggle_fullscreen)
        self.fullscreen_action = fullscreen_action
        
        # ===== Menu Outils =====
        tools_menu = menubar.addMenu("🔧 Outils")
        
        # Sous-menu Calcul avancé
        advanced_calc_submenu = tools_menu.addMenu("⚡ Calcul Avancé")
        
        reload_data_action = advanced_calc_submenu.addAction("🔄 Recharger les Données")
        reload_data_action.triggered.connect(self.reload_all_data)
        
        advanced_calc_submenu.addSeparator()
        
        clear_cache_action = advanced_calc_submenu.addAction("🗑️ Vider le Cache")
        clear_cache_action.triggered.connect(self.clear_data_cache)
        
        tools_menu.addSeparator()
        
        # Sous-menu Visualisation avancée
        advanced_viz_submenu = tools_menu.addMenu("🎨 Visualisation Avancée")
        
        reset_view_action = advanced_viz_submenu.addAction("🔄 Réinitialiser la Vue")
        reset_view_action.triggered.connect(self.reset_view)
        
        advanced_viz_submenu.addSeparator()
        
        zoom_in_action = advanced_viz_submenu.addAction("🔍 Zoom +")
        zoom_in_action.setShortcut("Ctrl+Plus")
        zoom_in_action.triggered.connect(self.zoom_in)
        
        zoom_out_action = advanced_viz_submenu.addAction("🔍 Zoom -")
        zoom_out_action.setShortcut("Ctrl+Minus")
        zoom_out_action.triggered.connect(self.zoom_out)
        
        tools_menu.addSeparator()
        
        # Sous-menu Données
        data_submenu = tools_menu.addMenu("📊 Données")
        
        export_data_action = data_submenu.addAction("💾 Exporter Données")
        export_data_action.triggered.connect(self.export_data)
        
        import_data_action = data_submenu.addAction("📂 Importer Données")
        import_data_action.triggered.connect(self.import_data)
        
        # ===== Menu Aide =====
        help_menu = menubar.addMenu("❓ Aide")
        
        help_action = help_menu.addAction("📚 Guide d'Utilisation")
        help_action.setShortcut(QKeySequence("F1"))
        help_action.triggered.connect(self.show_help)
        
        help_menu.addSeparator()
        
        shortcuts_action = help_menu.addAction("⌨️ Raccourcis Clavier")
        shortcuts_action.triggered.connect(self.show_shortcuts)
        
        help_menu.addSeparator()
        
        report_action = help_menu.addAction("📊 Rapport de Calcul (Texte)")
        report_action.triggered.connect(self.show_text_report)
        
        help_menu.addSeparator()
        
        about_action = help_menu.addAction("ℹ️ À propos")
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
        return HELP_CONTENT_HTML
    
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
        try:
            # Chemin absolu vers le fichier aide.html dans le même répertoire que le script
            help_file = os.path.join(os.getcwd() + "/assets/aide.html")
            with open(help_file, 'r', encoding='utf-8') as f:
                return f.read()
        except Exception as e:
            print(f"Erreur lors de la lecture du fichier d'aide : {e}")
            return "<h1>Erreur</h1><p>Impossible de charger le contenu d'aide.</p>"
    
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
        return ABOUT_CONTENT_HTML
    
    def show_text_report(self):
        """Affiche le rapport de calcul en format texte avec tableaux pandas"""
        report_dialog = QDialog(self)
        report_dialog.setWindowTitle("Rapport de Calcul (Format Texte)")
        report_dialog.setModal(False)
        report_dialog.resize(1200, 800)
        
        # Appliquer le thème sombre à la fenêtre
        report_dialog.setStyleSheet("""
            QDialog {
                background-color: #2b2b2b;
                color: #ffffff;
            }
            QTextEdit {
                background-color: #1e1e1e;
                color: #00ff00;
                border: 1px solid #555555;
                font-family: 'Courier New', monospace;
                font-size: 9pt;
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
        
        layout = QVBoxLayout(report_dialog)
        
        # Zone de texte avec contenu du rapport
        report_text = QTextEdit()
        report_text.setReadOnly(True)
        
        # Charger le rapport texte
        report_path = Path(os.getcwd() + "/reports/RAPPORT_CALCUL.txt")
        if report_path.exists():
            with open(report_path, "r", encoding="utf-8") as f:
                report_content = f.read()
            report_text.setPlainText(report_content)
        else:
            report_text.setPlainText("""
RAPPORT NON TROUVÉ

Le fichier RAPPORT_CALCUL.txt n'a pas été trouvé.
Veuillez d'abord générer le rapport en exécutant:
  python generate_report.py

Ou utilisez le menu Fichier > Calcul > Lancer le Calcul C++
""")
        
        layout.addWidget(report_text)
        
        # Boutons d'action
        button_layout = QHBoxLayout()
        
        # Bouton pour régénérer le rapport
        regenerate_button = QPushButton("🔄 Régénérer le Rapport")
        regenerate_button.clicked.connect(lambda: self.regenerate_text_report())
        button_layout.addWidget(regenerate_button)
        
        # Bouton pour ouvrir le fichier
        open_button = QPushButton("📂 Ouvrir le Fichier")
        open_button.clicked.connect(lambda: self.open_report_file())
        button_layout.addWidget(open_button)
        
        # Bouton de fermeture
        close_button = QPushButton("Fermer")
        close_button.clicked.connect(report_dialog.close)
        button_layout.addWidget(close_button)
        
        layout.addLayout(button_layout)
        
        report_dialog.exec()
    
    def regenerate_text_report(self):
        """Régénère le rapport texte"""
        try:
            from reports.generate_report import save_report
            save_report()
            QMessageBox.information(self, "Succès", "Rapport régénéré avec succès!")
        except Exception as e:
            QMessageBox.critical(self, "Erreur", f"Erreur lors de la génération du rapport:\n{str(e)}")
    
    def open_report_file(self):
        """Ouvre le fichier rapport dans l'éditeur par défaut"""
        import subprocess
        report_path = Path(os.getcwd() + "/reports/RAPPORT_CALCUL.txt")
        if report_path.exists():
            try:
                subprocess.Popen(['notepad', str(report_path)])
            except Exception as e:
                QMessageBox.warning(self, "Erreur", f"Impossible d'ouvrir le fichier:\n{str(e)}")
        else:
            QMessageBox.warning(self, "Erreur", "Le rapport de calcul n'a pas été trouvé.")
    
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
        curve_action = QAction("📉", self)
        curve_action.setToolTip("Type de Courbe (Ctrl+T)")
        curve_action.setShortcut("Ctrl+T")
        curve_action.triggered.connect(self.show_curve_selection_dialog)
        toolbar.addAction(curve_action)
        
        # Action Sélection Travée & Section
        span_section_action = QAction("🔢", self)
        span_section_action.setToolTip("Sélection Travée & Section (Ctrl+N)")
        span_section_action.setShortcut("Ctrl+N")
        span_section_action.triggered.connect(self.show_span_section_dialog)
        toolbar.addAction(span_section_action)
        
        toolbar.addSeparator()
        
        # Action Tracer
        plot_action = QAction("✏️", self)
        plot_action.setToolTip("Tracer (Ctrl+P)")
        plot_action.triggered.connect(self.plot_current_selection)
        toolbar.addAction(plot_action)
        
        toolbar.addSeparator()
        
        # Action Animer Courbe
        animate_action = QAction("▶️", self)
        animate_action.setToolTip("Animer Courbe (Ctrl+A)")
        animate_action.triggered.connect(self.animate_current_selection)
        toolbar.addAction(animate_action)
        
        # Action Animer Complet
        animate_full_action = QAction("⏩", self)
        animate_full_action.setToolTip("Animer Complet (Ctrl+Shift+A)")
        animate_full_action.triggered.connect(self.animate_full_curve)
        toolbar.addAction(animate_full_action)
        
        toolbar.addSeparator()
        
        # Action Maximum
        max_action = QAction("⭐", self)
        max_action.setToolTip("Afficher Maximum (Ctrl+M)")
        max_action.triggered.connect(self.show_maximum)
        toolbar.addAction(max_action)
        
        toolbar.addSeparator()
        
        # Action Sauvegarder
        save_action = QAction("💿", self)
        save_action.setToolTip("Sauvegarder Configuration (Ctrl+S)")
        save_action.setShortcut("Ctrl+S")
        save_action.triggered.connect(self.save_configuration)
        toolbar.addAction(save_action)
        
        # Action Exporter
        export_action = QAction("🎁", self)
        export_action.setToolTip("Exporter Animation (GIF) (Ctrl+E)")
        export_action.setShortcut("Ctrl+E")
        export_action.triggered.connect(self.export_animation)
        toolbar.addAction(export_action)
        
        toolbar.addSeparator()
        
        # Action Masquer/Afficher Panneau Configuration
        self.toggle_panel_action = QAction("👀", self)
        self.toggle_panel_action.setToolTip("Masquer/Afficher Panneau Configuration (Ctrl+H)")
        self.toggle_panel_action.setShortcut("Ctrl+H")
        self.toggle_panel_action.triggered.connect(self.toggle_config_panel)
        toolbar.addAction(self.toggle_panel_action)
        
        # Action Configuration
        config_action = QAction("🔧", self)
        config_action.setToolTip("Options de Configuration (Ctrl+Alt+C)")
        config_action.setShortcut("Ctrl+Alt+C")
        config_action.triggered.connect(self.show_config_dialog)
        toolbar.addAction(config_action)
        
        toolbar.addSeparator()
        
        # Action Éditer input.txt
        edit_input_action = QAction("✍️", self)
        edit_input_action.setToolTip("Éditer input.txt (Ctrl+I)")
        edit_input_action.setShortcut("Ctrl+I")
        edit_input_action.triggered.connect(self.edit_input_file)
        toolbar.addAction(edit_input_action)
        
        # Action Lancer le calcul
        run_calc_action = QAction("🚀", self)
        run_calc_action.setToolTip("Lancer le Calcul C++ (Ctrl+R)")
        run_calc_action.setShortcut("Ctrl+R")
        run_calc_action.triggered.connect(self.run_calculation)
        toolbar.addAction(run_calc_action)
        
        toolbar.addSeparator()
        
        # Action Générer Rapport de Calcul
        report_action = QAction("📋", self)
        report_action.setToolTip("Générer/Afficher Rapport de Calcul (Ctrl+Alt+R)")
        report_action.setShortcut("Ctrl+Alt+R")
        report_action.triggered.connect(self.show_text_report)
        toolbar.addAction(report_action)
        
        toolbar.addSeparator()
        
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
    
    def apply_theme(self, theme_name="dark"):
        """Applique un thème à l'application"""
        if self.themes_data and "themes" in self.themes_data:
            themes = self.themes_data["themes"]
            if theme_name in themes:
                theme = themes[theme_name]
                stylesheet = theme.get("stylesheet", DARK_THEME_STYLESHEET)
                self.setStyleSheet(stylesheet)
                self.current_theme = theme_name
                print(f"Thème appliqué: {theme.get('name', theme_name)}")
            else:
                print(f"Thème '{theme_name}' non trouvé, utilisation du thème par défaut")
                self.setStyleSheet(DARK_THEME_STYLESHEET)
        else:
            self.setStyleSheet(DARK_THEME_STYLESHEET)

    def get_available_themes(self):
        """Retourne la liste des thèmes disponibles"""
        if self.themes_data and "themes" in self.themes_data:
            return list(self.themes_data["themes"].keys())
        return ["dark"]

    def get_theme_display_name(self, theme_name):
        """Retourne le nom d'affichage d'un thème"""
        if self.themes_data and "themes" in self.themes_data:
            theme = self.themes_data["themes"].get(theme_name, {})
            return theme.get("name", theme_name)
        return theme_name

    def setup_shortcuts(self):
        """Configure les raccourcis clavier avec QShortcut pour éviter les conflits"""
        # Utiliser QShortcut pour les raccourcis qui pourraient causer des conflits
        # Cela évite les messages "Ambiguous shortcut overload"
        
        # F11 - Plein écran (déjà défini dans le menu)
        # Les autres raccourcis sont définis dans create_menu_bar()
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
    
    def _animate_transition_step(self):
        """Étape d'animation de transition (fade-in)"""
        self.transition_alpha += 0.1
        if self.transition_alpha >= 1.0:
            self.transition_alpha = 1.0
            if self.transition_timer:
                self.transition_timer.stop()
                self.transition_timer = None
        
        # Mettre à jour l'alpha de tous les éléments
        for element in self.transition_elements:
            if hasattr(element, 'set_alpha'):
                element.set_alpha(self.transition_alpha)
            elif hasattr(element, 'set_color'):
                # Pour les scatter plots
                colors = element.get_facecolors()
                if len(colors) > 0:
                    colors[:, 3] = self.transition_alpha
                    element.set_facecolors(colors)
        
        self.canvas.draw_idle()
    
    def _start_transition_animation(self):
        """Démarre l'animation de transition"""
        self.transition_alpha = 0.0
        if self.transition_timer is None:
            self.transition_timer = QTimer()
            self.transition_timer.timeout.connect(self._animate_transition_step)
        self.transition_timer.start(30)  # 30ms par frame
    
    def plot_directly(self, curve, span, section, with_transition=True):
        """Trace directement avec self.ax sans passer par func_plot.py"""
        self.ax.clear()
        
        # Configurer les labels de l'axe X
        self.ax.set_xticks(utils.neouds)
        self.ax.set_xticklabels(utils.distances)
        plt.setp(self.ax.get_xticklabels(), rotation=45)
        
        # Tracer la courbe
        x_values, y_values = self.get_curve_xy(curve, span, section)
        
        # Générer le label approprié selon le type de courbe
        if curve == "support_moments":
            label = f"M_{span}"
        elif curve == "support_reactions":
            label = f"R_{span}"
        else:
            label = f"Travee : {span+1}\nSection : {section}"
        
        # Alpha initial pour l'animation de transition
        initial_alpha = 0.0 if with_transition else 1.0
        
        # Dessiner les éléments structurels
        self.transition_elements = []
        if self.config.get("travee"):
            line, = self.ax.plot(utils.x_normal, [0]*len(utils.x_normal), 'k--', alpha=initial_alpha, label='Travées')
            self.transition_elements.append(line)
        if self.config.get("noeud"):
            scatter = self.ax.scatter(utils.neouds, [0]*len(utils.neouds), color=self.config["style"].get("noeud_color", "#FF4444"),
                          s=self.config["style"].get("noeud_size", 60), zorder=5, label='Noeuds', alpha=initial_alpha)
            self.transition_elements.append(scatter)
        
        # Tracer la courbe
        curve_line, = self.ax.plot(x_values, y_values, label=label, alpha=initial_alpha)
        self.transition_elements.append(curve_line)
        
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
        
        # Démarrer l'animation de transition
        if with_transition:
            self._start_transition_animation()
        else:
            self.canvas.draw()
                
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
            area = self.max_area_check.isChecked()
           
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
    
    def toggle_auto_report(self, checked):
        """Active/désactive la génération automatique du rapport"""
        from automation_config import set_automation_setting
        set_automation_setting("auto_generate_text_report", checked)
        self.auto_report_action.setChecked(checked)
        status = "activée" if checked else "désactivée"
        print(f"[INFO] Génération automatique du rapport: {status}")
    
    def toggle_auto_reload(self, checked):
        """Active/désactive le rechargement automatique des données"""
        from config.automation_config import set_automation_setting
        set_automation_setting("auto_reload_data", checked)
        self.auto_reload_action.setChecked(checked)
        status = "activé" if checked else "désactivé"
        print(f"[INFO] Rechargement automatique des données: {status}")
    
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
        from config.automation_config import get_automation_setting
        
        # Recharger les données après le calcul si activé
        if get_automation_setting("auto_reload_data", True):
            self.reload_all_data()
            reload_msg = "Les données ont été rechargées automatiquement."
        else:
            reload_msg = "Rechargement automatique désactivé."
        
        # Générer automatiquement le rapport texte si activé
        report_msg = ""
        if get_automation_setting("auto_generate_text_report", True):
            try:
                from reports.generate_report import save_report
                save_report()
                report_msg = "\n✓ Rapport de calcul généré automatiquement!"
            except Exception as e:
                print(f"Erreur lors de la génération du rapport: {e}")
                report_msg = f"\n⚠️ Erreur lors de la génération du rapport"
        else:
            report_msg = "\n⚠️ Génération automatique du rapport désactivée"
        
        QMessageBox.information(self, "Calcul Terminé", 
                              f"✅ Le calcul s'est terminé avec succès!\n\n"
                              f"Exécutable: {exe_name}\n"
                              f"Répertoire: {working_dir}\n\n"
                              "Les résultats sont maintenant disponibles dans:\n"
                              "data/results/\n\n"
                              f"{reload_msg}"
                              f"{report_msg}\n\n"
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
    
    def clear_data_cache(self):
        """Vide le cache des données"""
        self.data_cache.clear()
        QMessageBox.information(self, "Cache Vidé", "Le cache des données a été vidé avec succès.")
    
    def reset_view(self):
        """Réinitialise la vue du graphique"""
        if hasattr(self, 'ax'):
            self.ax.clear()
            self.canvas.draw()
            QMessageBox.information(self, "Vue Réinitialisée", "La vue du graphique a été réinitialisée.")
    
    def zoom_in(self):
        """Zoom avant sur le graphique"""
        if hasattr(self, 'ax'):
            self.ax.set_xlim(self.ax.get_xlim()[0] * 1.2, self.ax.get_xlim()[1] * 0.8)
            self.ax.set_ylim(self.ax.get_ylim()[0] * 1.2, self.ax.get_ylim()[1] * 0.8)
            self.canvas.draw()
    
    def zoom_out(self):
        """Zoom arrière sur le graphique"""
        if hasattr(self, 'ax'):
            self.ax.set_xlim(self.ax.get_xlim()[0] * 0.8, self.ax.get_xlim()[1] * 1.2)
            self.ax.set_ylim(self.ax.get_ylim()[0] * 0.8, self.ax.get_ylim()[1] * 1.2)
            self.canvas.draw()
    
    def export_data(self):
        """Exporte les données actuelles"""
        file_path, _ = QFileDialog.getSaveFileName(self, "Exporter les Données", "donnees.json", "JSON (*.json);;CSV (*.csv)")
        if file_path:
            try:
                curve = self.get_current_curve_type()
                data = self.load_curve_data(curve)
                with open(file_path, "w", encoding="utf-8") as f:
                    json.dump(data, f, ensure_ascii=False, indent=2)
                QMessageBox.information(self, "Export Réussi", f"Les données ont été exportées vers:\n{file_path}")
            except Exception as e:
                QMessageBox.critical(self, "Erreur", f"Erreur lors de l'export:\n{e}")
    
    def import_data(self):
        """Importe des données"""
        file_path, _ = QFileDialog.getOpenFileName(self, "Importer les Données", "", "JSON (*.json);;CSV (*.csv)")
        if file_path:
            try:
                with open(file_path, "r", encoding="utf-8") as f:
                    data = json.load(f)
                QMessageBox.information(self, "Import Réussi", f"Les données ont été importées depuis:\n{file_path}")
            except Exception as e:
                QMessageBox.critical(self, "Erreur", f"Erreur lors de l'import:\n{e}")


def main():
    app = QApplication(sys.argv)
    window = StructuralAnalysisGUI()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()