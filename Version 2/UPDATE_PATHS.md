# Mise à Jour des Chemins - Guide de Migration

## Résumé des Changements

Après l'organisation des fichiers, les chemins suivants doivent être mis à jour dans le code :

## Fichiers à Mettre à Jour

### 1. `src/lñ.py` (Application Principale)

#### Imports
```python
# AVANT
from help_content import HELP_CONTENT_HTML, ABOUT_CONTENT_HTML
from styles import DARK_THEME_STYLESHEET

# APRÈS
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from utils_modules.help_content import HELP_CONTENT_HTML, ABOUT_CONTENT_HTML
from utils_modules.styles import DARK_THEME_STYLESHEET
```

#### Chemins de Fichiers
```python
# Configuration
self.config_path = Path(__file__).resolve().parent.parent / "config" / "Configuration.json"

# Icône
icon_path = Path(__file__).resolve().parent.parent / "assets" / "istockphoto-1077739826-1024x1024.ico"

# Input
input_path = Path(__file__).resolve().parent.parent / "config" / "input.txt"

# Rapports
report_path = Path(__file__).resolve().parent.parent / "reports" / "RAPPORT_CALCUL.txt"
```

### 2. `reports/generate_report.py`

```python
# AVANT
base_path = Path(__file__).resolve().parent
analysis_path = base_path / "data" / "results" / "analysis"
properties_path = base_path / "data" / "results" / "properties"

# APRÈS
base_path = Path(__file__).resolve().parent.parent
analysis_path = base_path / "data" / "results" / "analysis"
properties_path = base_path / "data" / "results" / "properties"

# Sortie du rapport
output_path = Path(__file__).resolve().parent / filename
```

### 3. `config/automation_config.py`

Aucun changement nécessaire (fichier de configuration pure).

### 4. `utils_modules/help_content.py` et `styles.py`

Aucun changement nécessaire (fichiers de contenu statique).

## Étapes de Migration

### Option 1 : Automatique (Recommandé)
```bash
python organize_files.py
```
Cela déplace automatiquement tous les fichiers dans les bons dossiers.

### Option 2 : Manuel

1. Déplacez les fichiers :
   ```
   lñ.py → src/
   utils.py → src/
   snake_game.py → src/
   
   Configuration.json → config/
   automation_config.py → config/
   input.txt → config/
   
   RAPPORT_CALCUL.txt → reports/
   RAPPORT_CALCUL.html → reports/
   generate_report.py → reports/
   
   istockphoto-1077739826-1024x1024.ico → assets/
   gLigne d'influence.ico → assets/
   
   help_content.py → utils_modules/
   styles.py → utils_modules/
   ```

2. Mettez à jour les imports dans `src/lñ.py`

3. Mettez à jour les chemins dans `reports/generate_report.py`

## Vérification

Après la migration, vérifiez que :
- ✓ L'application démarre sans erreur
- ✓ La configuration se charge correctement
- ✓ Les icônes s'affichent
- ✓ Le calcul se lance
- ✓ Les rapports se génèrent
- ✓ Les données se chargent

## Chemins Relatifs Recommandés

Utilisez toujours `Path(__file__).resolve()` pour la portabilité :

```python
from pathlib import Path

# Depuis src/lñ.py
base_path = Path(__file__).resolve().parent.parent
config_path = base_path / "config" / "Configuration.json"
assets_path = base_path / "assets"
data_path = base_path / "data"

# Depuis reports/generate_report.py
base_path = Path(__file__).resolve().parent.parent
analysis_path = base_path / "data" / "results" / "analysis"
output_path = Path(__file__).resolve().parent / "RAPPORT_CALCUL.txt"
```

## Rollback

Si vous avez besoin de revenir à la structure précédente :

```bash
# Déplacer les fichiers vers la racine
move src\*.py .
move config\*.json .
move config\*.py .
move config\*.txt .
move assets\*.ico .
move utils_modules\*.py .
move reports\*.txt .
move reports\*.html .
move reports\*.py .

# Supprimer les dossiers vides
rmdir src config assets utils_modules reports
```

## Notes Importantes

- Les chemins utilisent `Path` pour la portabilité Windows/Linux/Mac
- Les fichiers de données dans `data/` ne sont pas déplacés
- L'exécutable `Ligne d'influence.exe` reste à la racine
- Les fichiers `.gitignore` doivent être mis à jour si nécessaire

## Fichiers Affectés

- ✓ `src/lñ.py` - Imports et chemins
- ✓ `reports/generate_report.py` - Chemins de base
- ✓ Tous les autres fichiers restent compatibles

## Support

En cas de problème lors de la migration, consultez :
- `ORGANISATION.md` - Structure complète
- `README.md` - Guide d'utilisation
- Code source commenté dans `src/lñ.py`
