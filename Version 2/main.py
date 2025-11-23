#!/usr/bin/env python3
"""
Point d'entrée principal - Ligne d'Influence
Application d'analyse structurelle pour ponts multi-travées
"""

import sys
import time
from pathlib import Path
from PyQt6.QtWidgets import (QApplication, QSplashScreen, QVBoxLayout, QLabel, QWidget)
from PyQt6.QtGui import QPixmap, QColor, QPainter, QFont
from PyQt6.QtCore import Qt, QTimer
from src.lñ import StructuralAnalysisGUI

class CircularProgress(QWidget):
    def __init__(self):
        super().__init__()
        self.value = 0
        self.width = 80
        self.height = 80
        self.progress_width = 8
        self.progress_rounded_cap = True
        self.max_value = 100
        self.progress_color = '#2a82da'
        self.text_color = '#FFFFFF'
        self.font_size = 12
        self.suffix = '%'
        
        # Désactiver le fond du widget
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.resize(self.width, self.height)
    
    def set_value(self, value):
        self.value = value
        self.repaint()
    
    def paintEvent(self, event):
        width = self.width - self.progress_width
        height = self.height - self.progress_width
        margin = self.progress_width / 2
        value = self.value * 3.6  # Convertir le pourcentage en degrés
        
        # Créer un objet QPainter
        paint = QPainter()
        paint.begin(self)
        paint.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Définir la police
        font = QFont()
        font.setFamily('Arial')
        font.setPointSize(self.font_size)
        paint.setFont(font)
        paint.setPen(QColor(self.text_color))
        
        # Dessiner le cercle de fond
        pen = paint.pen()
        pen.setColor(QColor(255, 255, 255, 30))
        pen.setWidth(self.progress_width)
        paint.setPen(pen)
        paint.drawEllipse(
            margin, margin, 
            self.width - (self.progress_width), 
            self.height - (self.progress_width)
        )
        
        # Dessiner l'arc de progression
        pen.setColor(QColor(self.progress_color))
        paint.setPen(pen)
        
        rect = self.rect().adjusted(margin, margin, -margin, -margin)
        paint.drawArc(rect, 90 * 16, -value * 16)  # *16 car les angles sont en 1/16 de degré
        
        # Afficher la valeur en pourcentage
        text = f"{self.value}%"
        text_rect = self.rect().adjusted(0, 0, 0, 0)
        paint.drawText(text_rect, Qt.AlignmentFlag.AlignCenter, text)
        
        paint.end()

class CustomSplashScreen(QSplashScreen):
    def __init__(self, pixmap):
        super().__init__(pixmap)
        self.progress = 0
        self.message = "Chargement de l'application..."
        
        # Créer un widget conteneur pour le cercle de progression
        self.progress_container = QWidget(self)
        self.progress_container.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        
        # Créer et configurer le cercle de progression
        self.circular_progress = CircularProgress()
        
        # Layout pour centrer le cercle de progression
        layout = QVBoxLayout(self.progress_container)
        layout.addWidget(self.circular_progress, 0, Qt.AlignmentFlag.AlignCenter)
        layout.setContentsMargins(0, 0, 0, 40)  # Marge en bas pour le message
        
        # Positionner le conteneur
        self.progress_container.setGeometry(
            (pixmap.width() - 100) // 2,  # Centrer horizontalement
            pixmap.height() - 150,         # Position verticale
            100, 120                       # Largeur et hauteur
        )
        
        # Style amélioré pour les messages
        self.setStyleSheet("""
            QLabel {
                color: #ffffff;
                font-size: 16px;
                font-weight: bold;
                background: rgba(0, 0, 0, 0.7);
                padding: 10px 20px;
                border-radius: 12px;
                border: 1px solid rgba(255, 255, 255, 0.2);
                text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.8);
                min-width: 350px;
                text-align: center;
                font-family: 'Segoe UI', Arial, sans-serif;
            }
        """)
    
    def set_progress(self, value, message=None):
        self.progress = value
        self.circular_progress.set_value(value)
        if message:
            self.message = message
            
        # Formater le message avec une majuscule et un point si nécessaire
        formatted_message = self.message
        if not formatted_message.endswith(('.', '!', '...')):
            formatted_message = formatted_message.capitalize() + '...'
            
        self.showMessage(
            formatted_message,
            Qt.AlignmentFlag.AlignBottom | Qt.AlignmentFlag.AlignHCenter,
            QColor("#ffffff")
        )
        
        # Forcer la mise à jour de l'interface
        QApplication.processEvents()
        
        # Délai supplémentaire pour les messages importants
        if any(keyword in self.message.lower() for keyword in ['prêt', 'terminé', 'erreur']):
            time.sleep(0.3)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    
    # Afficher le splash screen personnalisé
    splash_image_path = Path("assets/Chaotianmen_Bridge.jpg")
    if splash_image_path.exists():
        # Charger et redimensionner l'image de fond
        pixmap = QPixmap(str(splash_image_path))
        pixmap = pixmap.scaled(800, 500, Qt.AspectRatioMode.KeepAspectRatioByExpanding, 
                             Qt.TransformationMode.SmoothTransformation)
        
        # Créer et configurer le splash screen personnalisé
        splash = CustomSplashScreen(pixmap)
        splash.setWindowFlags(Qt.WindowType.FramelessWindowHint)
        splash.setEnabled(False)
        
        # Afficher le splash screen
        splash.show()
        
        # Messages de chargement personnalisés
        loading_steps = [
            (20, "Démarrage du système d'analyse..."),
            (40, "Chargement des modules de calcul..."),
            (60, "Configuration des outils d'ingénierie..."),
            (80, "Préparation de l'interface utilisateur..."),
            (95, "Optimisation des performances..."),
            (100, "Prêt à l'emploi !")
        ]
        
        # Simuler un chargement progressif avec des messages personnalisés
        current_step = 0
        for i in range(1, 101):
            # Mettre à jour le message si nécessaire
            if current_step < len(loading_steps) and i >= loading_steps[current_step][0]:
                splash.set_progress(i, loading_steps[current_step][1])
                current_step += 1
            else:
                splash.circular_progress.set_value(i)
            
            # Ajuster la vitesse de progression
            if i > 90:
                time.sleep(0.05)  # Ralentir vers la fin
            else:
                time.sleep(0.02)
        
        # Message de bienvenue animé
        welcome_messages = [
            "Bienvenue sur AEBTP - Logiciel d'Analyse Structurelle",
            "Conçu pour les ingénieurs et techniciens du BTP",
            "Solution complète pour l'analyse des structures",
            " 2025 AEBTP - Tous droits réservés"
        ]
        
        for msg in welcome_messages:
            splash.showMessage(
                msg,
                Qt.AlignmentFlag.AlignBottom | Qt.AlignmentFlag.AlignHCenter,
                QColor("#ffffff")
            )
            time.sleep(0.8)

    # Fermer le splash screen
    if 'splash' in locals():
        splash.finish(None)
    
    # Lancer l'application principale
    window = StructuralAnalysisGUI()
    window.show()
    
    sys.exit(app.exec())