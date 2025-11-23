#!/bin/bash
# Script de démarrage - Ligne d'Influence
# Lance l'application PyQt6

echo "Demarrage de l'application Ligne d'Influence..."
echo ""

# Vérifier que Python est installé
if ! command -v python3 &> /dev/null; then
    echo "ERREUR: Python 3 n'est pas installé"
    echo "Veuillez installer Python 3.8+ depuis https://www.python.org"
    exit 1
fi

# Vérifier que les dépendances sont installées
python3 -c "import PyQt6" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Installation des dépendances..."
    pip3 install PyQt6 matplotlib pandas numpy
fi

# Lancer l'application
python3 main.py

if [ $? -ne 0 ]; then
    echo ""
    echo "ERREUR: L'application s'est terminée avec une erreur"
    exit 1
fi
