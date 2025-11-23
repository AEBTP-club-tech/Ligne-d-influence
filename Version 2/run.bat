@echo off
REM Script de démarrage - Ligne d'Influence
REM Lance l'application PyQt6

echo Demarrage de l'application Ligne d'Influence...
echo.

REM Vérifier que Python est installé
python --version >nul 2>&1
if errorlevel 1 (
    echo ERREUR: Python n'est pas installé ou n'est pas dans le PATH
    echo Veuillez installer Python 3.8+ depuis https://www.python.org
    pause
    exit /b 1
)

REM Vérifier que les dépendances sont installées
python -c "import PyQt6" >nul 2>&1
if errorlevel 1 (
    echo Installation des dépendances...
    pip install PyQt6 matplotlib pandas numpy
)

REM Lancer l'application
python main.py

if errorlevel 1 (
    echo.
    echo ERREUR: L'application s'est terminée avec une erreur
    pause
)
