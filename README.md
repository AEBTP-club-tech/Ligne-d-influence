# Ligne d'influence

Suite logicielle de calcul et de visualisation des lignes d'influence d'un tablier isostatique ou hyperstatique.  
Le cœur du projet est un solveur C++17 qui lit un fichier `input.txt`, assemble les propriétés des travées (béton, acier ou bois), calcule les efforts internes puis exporte les résultats en JSON. Des scripts Python dédiés exploitent ces données pour tracer, animer ou analyser les diagrammes.

## Fonctionnalités
- Calcul automatique des lignes d'influence (moments, efforts tranchants, flèches, rotations, réactions).
- Prise en charge de matériaux multiples (béton, acier, bois) avec modules de Young adaptés et classes normatives EC5.
- Inerties constantes ou variables par travée, avec maillage configurable (`Precision`).
- Export structuré en JSON (`data/results/...`) et historisation des runs dans `data/history.json`.
- Scripts Python pour tracer, animer et configurer les diagrammes (`plotting/`).
- Livrable prêt à l'emploi (`livrable/`) incluant exécutables, données d'exemple et scripts.

## Organisation du dépôt
- `Ligne d_influence/` – projet Visual Studio (`.vcxproj`) et code C++ (`src/`, `headers/`).
  - `src/main.cpp` : point d'entrée, charge la configuration et orchestre les calculs/export.
  - `src/logic/` : parsing, configuration, gestion d'historique, export JSON.
  - `src/materiau/` et `headers/` : modèles matériaux (béton, acier, bois).
  - `src/plotting/` : scripts Python de visualisation (matplotlib + numpy).
- `livrable/` – distribution contenant `input.txt`, données d'exemple (`data/results/`), exécutables et scripts de plotting configurés.
- `bin/` – artéfacts de compilation (Debug x64).
- `README.md` – ce document.

## Prérequis
### Calcul C++
- Windows 10+, Visual Studio 2019/2022 (toolset v143 recommandé).
- C++17, `nlohmann/json` (déjà vendangée dans `headers/nlohmann/`).
- Facultatif : Python 3.10+ pour post-traitements.

### Visualisation Python
Installez les dépendances dans un environnement virtuel :
```bash
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt  # à créer si besoin
pip install matplotlib numpy
```
Les scripts n'utilisent que la bibliothèque standard, `matplotlib` et `numpy`.

## Compilation et exécution
1. **Ouvrir la solution**  
   - `File > Open > Project/Solution` puis sélectionner `Ligne d_influence/Ligne d_influence.vcxproj` (ou `Ligne d_influence.slnx`).
2. **Configurer**  
   - Plateforme `x64`, configuration `Debug` ou `Release`.
   - Vérifier que le répertoire de travail pointe vers `Ligne d_influence/` (pour que `input.txt` et `data/` soient résolus).
3. **Construire**  
   - `Build > Build Solution` ou `Ctrl+Shift+B`.
4. **Fournir un `input.txt`**  
   - Copier votre fichier dans le même dossier que l'exécutable (`Ligne d_influence/` ou `livrable/` selon vos besoins).
   - Un exemple complet est disponible dans `livrable/input.txt`.
5. **Lancer**  
   - Depuis Visual Studio (`Debug > Start Debugging`) ou en exécutant `Ligne d_influence.exe`.  
   - Les résultats sont exportés sous `data/results/` (créé automatiquement).

## Paramétrage (`input.txt`)
Chaque ligne suit `Clé: valeur`. Les commentaires commencent par `#`. Principales clés :

- `Longueur` : liste d'entiers/doubles séparés par des espaces (longueurs de travées en m).
- `Precision` : pas de discrétisation (10=1 m, 100=1 cm, 1000=1 mm).
- `CHOICHE` : `Concrete`, `Steel` ou `Wood`.
- Paramètres matériaux spécifiques (`Beton`, `preference`, `prise`, `temps`, `condition`, `Steel`, `SteelCondition`, `Wood`, `WoodServiceClass`, `WoodLoadDuration`).
- Inertie :
  - `Inertie_variable: y|n`.
  - `I` pour inertie constante, ou paires `Iv_n` / `Xv_n` pour chaque travée si variable.

⚠️ Assurez-vous que le nombre de valeurs `Iv_n` correspond à celui de `Xv_n` et que `n` couvre toutes les travées (`0` → `len(Longueur)-1`).

## Résultats et historique
- `data/results/` décompose les sorties en sous-dossiers (`analysis/`, `influence_lines/`, `boundary_conditions/`, `properties/`, `static_analysis/`).  
  Exemple : `influence_lines/span_moments.json`, `analysis/max_span_moments.json`.
- `data/history.json` conserve un log horodaté des configurations et temps d’exécution (voir `HistoryLogger`).

## Visualisation & post-traitement
1. Positionnez-vous dans `Ligne d_influence/src/plotting/` ou `livrable/plotting/`.
2. Activez votre environnement Python.
3. Lancez, par exemple :
   ```bash
   python func_plot.py          # tracés statiques
   python animate.py            # animation (génère FIG.gif)
   python one_animate.py        # scénario paramétré
   ```
4. `Configuration.json` (créé manuellement si absent) permet d'ajuster thèmes, vitesse et options (`DEFAULT_CONFIG` dans `utils.py`).

Les scripts consomment les JSON présents dans `../data/results`. Vérifiez que le chemin correspond à votre organisation (cf. `open_json()`).

## Livrable prêt à l'emploi
Le dossier `livrable/` contient :
- `Ligne d_influence.exe` et `gLigne d_influence.exe` (interfaces/frontal graphique).
- `input.txt` exemplaire.
- `data/results/` remplis pour tests/sandboxes.
- Scripts `plotting/` avec `Configuration.json`.
Copiez ce dossier sur une autre machine Windows pour lancer un calcul sans recompilation.

## Dépannage rapide
- **Erreur “Impossible d'ouvrir input.txt”** : vérifier le répertoire de travail et la présence du fichier.
- **Résultats vides** : assurez-vous que `Longueur` n'est pas vide et que `Precision` > 0.
- **Plots introuvables** : confirmer que `data/results/...` existe et que les scripts sont exécutés depuis un chemin compatible (voir `open_json()`).
- **Modules Python manquants** : `pip install matplotlib numpy`.

## Contribution
1. Créez une branche (`git checkout -b feature/ma-fonction`).
2. Appliquez vos modifications.
3. Ajoutez des tests/données si nécessaire.
4. Exécutez les scripts ou recompilation locale.
5. Proposez une Pull Request en détaillant l’impact sur les calculs/export.

## Licence
Licence à préciser (non fournie). Ajoutez un fichier `LICENSE` si vous décidez d’en publier une.


