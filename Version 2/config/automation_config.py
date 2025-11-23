"""
Configuration pour l'automatisation des tâches après calcul
"""

# Configuration d'automatisation
AUTOMATION_CONFIG = {
    # Générer automatiquement le rapport texte après calcul
    "auto_generate_text_report": True,
    
    # Générer automatiquement le rapport HTML après calcul
    "auto_generate_html_report": False,
    
    # Recharger automatiquement les données après calcul
    "auto_reload_data": True,
    
    # Afficher automatiquement le rapport après génération
    "auto_show_report": False,
    
    # Exporter automatiquement les animations
    "auto_export_animations": False,
    
    # Créer automatiquement des graphiques
    "auto_create_plots": False,
    
    # Sauvegarder automatiquement la configuration
    "auto_save_config": True,
    
    # Notifications
    "show_notifications": True,
    "notification_duration": 5000,  # millisecondes
}

def get_automation_setting(key, default=None):
    """Récupère un paramètre d'automatisation"""
    return AUTOMATION_CONFIG.get(key, default)

def set_automation_setting(key, value):
    """Définit un paramètre d'automatisation"""
    AUTOMATION_CONFIG[key] = value

def enable_automation(feature):
    """Active une fonctionnalité d'automatisation"""
    set_automation_setting(feature, True)

def disable_automation(feature):
    """Désactive une fonctionnalité d'automatisation"""
    set_automation_setting(feature, False)

def get_all_settings():
    """Retourne tous les paramètres d'automatisation"""
    return AUTOMATION_CONFIG.copy()

def reset_to_defaults():
    """Réinitialise les paramètres par défaut"""
    global AUTOMATION_CONFIG
    AUTOMATION_CONFIG = {
        "auto_generate_text_report": True,
        "auto_generate_html_report": False,
        "auto_reload_data": True,
        "auto_show_report": False,
        "auto_export_animations": False,
        "auto_create_plots": False,
        "auto_save_config": True,
        "show_notifications": True,
        "notification_duration": 5000,
    }
