# Organisation des Fichiers - Ligne d'Influence

## Structure des Dossiers

```
plotting/
├── src/                          # Code source principal
│   ├── lñ.py                    # Application principale (GUI)
│   ├── utils.py                 # Utilitaires et fonctions communes
│   └── snake_game.py            # Jeu bonus
│
├── config/                       # Fichiers de configuration
│   ├── Configuration.json        # Configuration de l'application
│   ├── automation_config.py      # Configuration d'automatisation
│   └── input.txt                 # Données d'entrée pour calcul C++
│
├── reports/                      # Rapports générés
│   ├── RAPPORT_CALCUL.txt       # Rapport texte avec tableaux pandas
│   ├── RAPPORT_CALCUL.html      # Rapport HTML formaté
│   └── generate_report.py        # Script de génération de rapports
│
├── assets/                       # Ressources (icônes, images)
│   ├── istockphoto-1077739826-1024x1024.ico  # Icône application
│   └── gLigne d'influence.ico   # Autre icône
│
├── utils_modules/               # Modules utilitaires
│   ├── help_content.py          # Contenu HTML de l'aide
│   └── styles.py                # Styles CSS/QSS
│
├── data/                         # Données de calcul
│   └── results/
│       ├── analysis/            # Résultats d'analyse (JSON)
│       ├── boundary_conditions/ # Conditions aux limites
│       ├── influence_lines/     # Lignes d'influence
│       └── properties/          # Propriétés structurelles (JSON)
│
├── Ligne d'influence.exe         # Exécutable C++ (calcul)
├── animation.gif                 # Animation générée
├── ORGANISATION.md               # Ce fichier
└── README.md                     # Documentation

```

## Description des Dossiers

### `src/` - Code Source
- **lñ.py** : Application principale PyQt6 avec interface graphique
- **utils.py** : Fonctions utilitaires, configuration par défaut
- **snake_game.py** : Mini-jeu bonus

### `config/` - Configuration
- **Configuration.json** : Paramètres de l'application (grille, légende, etc.)
- **automation_config.py** : Configuration d'automatisation après calcul
- **input.txt** : Données d'entrée pour l'exécutable C++

### `reports/` - Rapports
- **RAPPORT_CALCUL.txt** : Rapport texte avec tableaux pandas (généré automatiquement)
- **RAPPORT_CALCUL.html** : Rapport HTML formaté (généré automatiquement)
- **generate_report.py** : Script pour générer les rapports à partir des données JSON

### `assets/` - Ressources
- **istockphoto-1077739826-1024x1024.ico** : Icône principale de l'application
- **gLigne d'influence.ico** : Icône alternative

### `utils_modules/` - Modules Utilitaires
- **help_content.py** : Contenu HTML pour les dialogues d'aide et À propos
- **styles.py** : Thème sombre PyQt6 (CSS/QSS)

### `data/` - Données de Calcul
- **results/analysis/** : Fichiers JSON des résultats d'analyse
  - `max_span_moments.json` : Moments maximaux
  - `max_span_shear_forces.json` : Forces de cisaillement
  - `largest_moment_areas.json` : Top 10 des surfaces
  - etc.
  
- **results/properties/** : Propriétés structurelles
  - `span_lengths.json` : Longueurs des travées
  - `young_modulus.json` : Module d'Young
  - `moment_of_inertia.json` : Moments d'inertie
  - etc.

## Fichiers à la Racine

- **Ligne d'influence.exe** : Exécutable C++ pour les calculs
- **animation.gif** : Animation générée par le script
- **ORGANISATION.md** : Ce fichier (guide d'organisation)

## Flux de Travail

1. **Édition** : Modifiez `config/input.txt`
2. **Calcul** : Lancez le calcul via le menu ou l'icône ▶️
3. **Génération** : Le rapport est généré automatiquement dans `reports/`
4. **Visualisation** : Consultez les rapports via l'icône 📊

## Chemins Relatifs dans le Code

Les chemins sont configurés pour fonctionner avec cette structure :

```python
# Exemple : accéder aux données
base_path = Path(__file__).resolve().parent
analysis_path = base_path / "data" / "results" / "analysis"
properties_path = base_path / "data" / "results" / "properties"

# Exemple : accéder aux rapports
report_path = base_path / "reports" / "RAPPORT_CALCUL.txt"

# Exemple : accéder aux icônes
icon_path = base_path / "assets" / "istockphoto-1077739826-1024x1024.ico"
```

## Notes Importantes

- ✅ Les chemins sont configurés pour chercher les fichiers dans les nouveaux dossiers
- ✅ Les fichiers Python utilisent `Path(__file__).resolve()` pour la portabilité
- ✅ Les fichiers de configuration restent accessibles au démarrage
- ✅ Les rapports sont générés automatiquement après chaque calcul

## Maintenance

- Nettoyez régulièrement les anciens rapports dans `reports/`
- Sauvegardez les configurations importantes dans `config/`
- Gardez les données JSON dans `data/` pour traçabilité
