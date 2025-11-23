# Ligne d'Influence - Application d'Analyse Structurelle

## Description

Application PyQt6 pour l'analyse et la visualisation des lignes d'influence pour ponts multi-travées. Permet de :
- Visualiser les lignes d'influence
- Générer des animations
- Créer des rapports de calcul automatiques
- Exporter les résultats en différents formats

## Installation

### Prérequis
- Python 3.8+
- PyQt6
- Matplotlib
- Pandas
- NumPy

### Installation des dépendances

**Automatique (recommandé) :**
```bash
pip install -r requirements.txt
```

**Manuel :**
```bash
pip install PyQt6 matplotlib pandas numpy
```

## Démarrage

### Option 1 : Lancer avec le script principal (Recommandé)
```bash
python main.py
```

### Option 2 : Lancer avec le script de démarrage
**Windows :**
```bash
run.bat
```

**Linux/Mac :**
```bash
bash run.sh
```

### Option 3 : Lancer directement depuis src/
```bash
python src/lñ.py
```

### Organiser les fichiers (première fois)
```bash
python organize_files.py
```

## Utilisation

### 1. Éditer les données d'entrée
- Menu **Fichier → Édition → Éditer input.txt**
- Ou cliquez sur l'icône 📝 dans la barre d'outils

### 2. Lancer le calcul
- Menu **Fichier → Calcul → Lancer le Calcul C++**
- Ou cliquez sur l'icône ▶️ dans la barre d'outils
- Le rapport est généré automatiquement après le calcul

### 3. Visualiser les résultats
- Sélectionnez une courbe et une travée/section
- Cliquez sur 📊 pour tracer
- Cliquez sur 🎬 pour animer

### 4. Consulter le rapport
- Menu **Aide → Rapport de Calcul (Texte)**
- Ou cliquez sur l'icône 📊 dans la barre d'outils
- Affiche les tableaux pandas avec tous les résultats

## Barre d'Outils

| Icône | Fonction | Raccourci |
|-------|----------|-----------|
| 📈 | Type de Courbe | - |
| 📍 | Sélection Travée & Section | - |
| 📊 | Tracer | Ctrl+P |
| 🎬 | Animer Courbe | Ctrl+A |
| 🎞️ | Animer Complet | Ctrl+Shift+A |
| ⬆ | Afficher Maximum | Ctrl+M |
| 💾 | Sauvegarder Configuration | - |
| 📤 | Exporter Animation (GIF) | - |
| 👁️ | Masquer/Afficher Panneau | - |
| ⚙️ | Options Configuration | - |
| 📝 | Éditer input.txt | - |
| ▶️ | Lancer Calcul C++ | - |
| 📊 | Rapport de Calcul | - |
| 🔲 | Mode Plein Écran | F11 |

## Menu Aide

- **📚 Guide d'Utilisation** (F1) - Documentation complète
- **⌨️ Raccourcis Clavier** - Liste des raccourcis
- **📊 Rapport de Calcul (Texte)** - Rapport avec tableaux
- **ℹ️ À propos** - Informations sur l'application

## Configuration

### Fichiers de Configuration
- `config/Configuration.json` - Paramètres de l'application
- `config/automation_config.py` - Automatisation après calcul
- `config/input.txt` - Données d'entrée pour calcul C++

### Options d'Automatisation
Menu **⚙️ Configuration → ⚡ Automatisation**
- ✓ Générer Rapport Automatiquement
- ✓ Recharger Données Automatiquement

## Rapports

### Génération Automatique
Les rapports sont générés automatiquement après chaque calcul :
- `reports/RAPPORT_CALCUL.txt` - Format texte avec tableaux pandas
- `reports/RAPPORT_CALCUL.html` - Format HTML formaté

### Génération Manuelle
```bash
python reports/generate_report.py
```

## Structure des Fichiers

```
plotting/
├── src/                    # Code source
├── config/                 # Configuration
├── reports/                # Rapports générés
├── assets/                 # Ressources (icônes)
├── utils_modules/          # Modules utilitaires
├── data/                   # Données de calcul
├── Ligne d'influence.exe   # Exécutable C++
└── ORGANISATION.md         # Guide d'organisation
```

Voir `ORGANISATION.md` pour plus de détails.

## Fonctionnalités Principales

### Visualisation
- Affichage des lignes d'influence
- Grille, travées, nœuds, légende configurables
- Inversion de l'axe Y
- Style matplotlib par défaut

### Animation
- Animation de courbe sélectionnée
- Animation complète de toutes les courbes
- Export en format GIF

### Rapports
- Génération automatique après calcul
- Tableaux pandas formatés
- Données structurelles et résultats d'analyse
- Export en texte et HTML

### Configuration
- Sauvegarde automatique des paramètres
- Thème sombre PyQt6
- Interface personnalisable

## Raccourcis Clavier

| Raccourci | Action |
|-----------|--------|
| F1 | Guide d'Utilisation |
| Ctrl+P | Tracer |
| Ctrl+A | Animer Courbe |
| Ctrl+Shift+A | Animer Complet |
| Ctrl+M | Afficher Maximum |
| Ctrl+Q | Quitter |
| F11 | Mode Plein Écran |

## Dépannage

### Le calcul ne se lance pas
- Vérifiez que `Ligne d'influence.exe` existe
- Vérifiez que `config/input.txt` est valide
- Consultez la console pour les messages d'erreur

### Le rapport n'est pas généré
- Vérifiez que les données JSON existent dans `data/results/`
- Vérifiez que `reports/generate_report.py` est accessible
- Vérifiez les permissions d'écriture dans `reports/`

### L'application plante
- Vérifiez les dépendances Python
- Vérifiez la version de PyQt6
- Consultez la console pour les tracebacks

## Support

Pour plus d'informations, consultez :
- `ORGANISATION.md` - Structure des fichiers
- `src/lñ.py` - Code source commenté
- `config/Configuration.json` - Paramètres disponibles

## Licence

Propriétaire - AEBTP Club Tech

## Historique des Versions

### v2.0 (22/11/2025)
- ✓ Refactorisation complète avec modularisation
- ✓ Extraction HTML/CSS dans fichiers séparés
- ✓ Amélioration des menus avec sous-menus
- ✓ Ajout de l'icône application
- ✓ Génération automatique de rapports
- ✓ Automatisation après calcul
- ✓ Organisation des fichiers

### v1.0
- Version initiale
