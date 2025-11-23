# Code Map - Ligne d'Influence Project

## Project Overview
**Ligne d'Influence** is a structural analysis application for multi-span bridges. It provides a GUI for visualizing influence lines, moments, shear forces, rotations, and deflections with animation capabilities.

---

## Directory Structure

```
plotting/
├── src/                          # Main source code
│   ├── lñ.py                    # Main GUI application (PyQt6)
│   ├── utils.py                 # Utility functions and default config
│   └── snake_game.py            # Bonus snake game
│
├── config/                       # Configuration files
│   ├── Configuration.json        # App settings (grid, legend, speed, etc.)
│   ├── automation_config.py      # Automation configuration
│   └── input.txt                 # Input data for C++ calculation
│
├── reports/                      # Generated reports
│   ├── generate_report.py        # Report generation script
│   ├── RAPPORT_CALCUL.txt       # Text report with pandas tables
│   └── RAPPORT_CALCUL.html      # HTML formatted report
│
├── utils_modules/               # Utility modules
│   ├── help_content.py          # HTML help and about content
│   └── styles.py                # Dark theme stylesheet (QSS)
│
├── assets/                       # Resources (icons, images)
│   ├── istockphoto-1077739826-1024x1024.ico
│   └── gLigne d'influence.ico
│
├── data/                         # Calculation data
│   └── results/
│       ├── analysis/            # Analysis results (JSON)
│       ├── boundary_conditions/ # Boundary conditions
│       ├── influence_lines/     # Influence lines data
│       └── properties/          # Structural properties (JSON)
│
├── Ligne d'influence.exe         # C++ calculation executable
├── main.py                       # Entry point
├── run.bat / run.sh             # Run scripts
├── requirements.txt              # Python dependencies
├── ORGANISATION.md               # File organization guide
├── README.md                     # Project documentation
└── CODE_MAP.md                   # This file
```

---

## Core Modules

### 1. **main.py** - Entry Point
**Location:** `/main.py`  
**Purpose:** Application entry point  
**Key Functions:**
- Imports and launches the main GUI from `src.lñ`

```python
from src.lñ import main
main()
```

---

### 2. **src/lñ.py** - Main GUI Application
**Location:** `/src/lñ.py`  
**Size:** ~1957 lines  
**Purpose:** PyQt6 GUI for structural analysis visualization

#### Key Classes:

**CalculationWorker (QObject)**
- Runs C++ calculations in a separate thread
- Signals: `finished(exe_name, working_dir)`, `error(error_message)`
- Methods:
  - `run_calculation(exe_path, working_dir)` - Executes calculation in worker thread

**StructuralAnalysisGUI (QMainWindow)**
- Main application window
- Configuration management
- Plot and animation rendering
- Menu and toolbar creation

#### Key Methods:

| Method | Purpose |
|--------|---------|
| `__init__()` | Initialize GUI, load config, setup UI |
| `load_configuration()` | Load settings from Configuration.json |
| `sync_utils_config()` | Sync config with utils module |
| `persist_configuration()` | Save config to JSON |
| `setup_ui()` | Create main UI layout with splitter |
| `create_menu_bar()` | Create menu with File, View, Config options |
| `create_toolbar()` | Create toolbar with action buttons |
| `create_left_panel()` | Create left control panel |
| `create_right_panel()` | Create right plot panel |
| `create_curve_selection()` | Curve type selector (moments, shear, etc.) |
| `create_span_section_selection()` | Span and section spinboxes |
| `create_animation_controls()` | Play/pause/stop buttons |
| `plot_current_selection()` | Plot selected curve |
| `animate_current_selection()` | Animate selected curve |
| `animate_full_curve()` | Animate complete curve |
| `show_maximum()` | Display maximum values |
| `run_calculation()` | Launch C++ calculation |
| `edit_input_file()` | Open input.txt for editing |
| `export_animation()` | Export animation as GIF |
| `toggle_option()` | Toggle display options (grid, legend, etc.) |
| `show_preferences()` | Open preferences dialog |

#### Configuration Options:
```json
{
  "grid": boolean,
  "travee": boolean,
  "noeud": boolean,
  "legend": boolean,
  "axe_y_inverser": boolean,
  "default_matplotlib_style": boolean,
  "vitesse_bridge": float,
  "max_area": boolean,
  "style": { ... }
}
```

#### Curve Types:
- `span_moments` - Moments de Travée
- `span_shear_forces` - Forces de Cisaillement
- `span_rotations` - Rotations
- `span_deflections` - Deflexions
- `support_moments` - Moments d'Appui
- `support_reactions` - Réactions d'Appui

---

### 3. **src/utils.py** - Utility Functions
**Location:** `/src/utils.py`  
**Purpose:** Shared utilities and default configuration

#### Key Components:
- `DEFAULT_CONFIG` - Default application settings
- `current_config` - Global config reference
- Data loading and processing functions
- Plot styling and formatting functions
- Animation helper functions

---

### 4. **reports/generate_report.py** - Report Generation
**Location:** `/reports/generate_report.py`  
**Size:** 194 lines  
**Purpose:** Generate calculation reports from JSON data

#### Key Functions:

| Function | Purpose |
|----------|---------|
| `load_json(filename)` | Load JSON file with error handling |
| `get_all_json_files(directory)` | Get all JSON files from directory |
| `generate_report()` | Generate complete report as string |
| `save_report(filename)` | Save report to text file |

#### Report Sections:
1. **Structural Properties** - Bridge dimensions, materials
2. **Analysis Results** - Moments, shear forces, rotations, deflections
3. **Summary & Recommendations** - Key findings and next steps

#### Data Sources:
- **Analysis:** `/data/results/analysis/*.json`
  - `max_span_moments.json`
  - `max_span_shear_forces.json`
  - `largest_moment_areas.json`
  - etc.

- **Properties:** `/data/results/properties/*.json`
  - `span_lengths.json`
  - `young_modulus.json`
  - `moment_of_inertia.json`
  - etc.

---

### 5. **utils_modules/help_content.py** - Help Content
**Location:** `/utils_modules/help_content.py`  
**Purpose:** HTML content for help and about dialogs

#### Key Variables:
- `HELP_CONTENT_HTML` - Help documentation in HTML format
- `ABOUT_CONTENT_HTML` - About dialog content

---

### 6. **utils_modules/styles.py** - Styling
**Location:** `/utils_modules/styles.py`  
**Purpose:** Dark theme stylesheet for PyQt6

#### Key Variables:
- `DARK_THEME_STYLESHEET` - QSS stylesheet for dark theme

---

### 7. **config/Configuration.json** - Application Settings
**Location:** `/config/Configuration.json`  
**Purpose:** Persistent application configuration

#### Structure:
```json
{
  "grid": true,
  "travee": true,
  "noeud": true,
  "legend": true,
  "axe_y_inverser": false,
  "default_matplotlib_style": false,
  "vitesse_bridge": 0.050,
  "max_area": false,
  "style": { ... }
}
```

---

### 8. **config/input.txt** - Calculation Input
**Location:** `/config/input.txt`  
**Purpose:** Input data for C++ calculation engine

#### Contents:
- Bridge geometry (span lengths, supports)
- Material properties (Young's modulus, moment of inertia)
- Load cases and boundary conditions

---

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│ User Interface (lñ.py)                                      │
│ - Configuration selection                                   │
│ - Curve type selection                                      │
│ - Span/section selection                                    │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ├─→ [Edit Input] → config/input.txt
                 │
                 ├─→ [Run Calculation] → Ligne d'influence.exe
                 │                       ↓
                 │                   data/results/
                 │                   ├── analysis/*.json
                 │                   └── properties/*.json
                 │
                 ├─→ [Plot] → Load JSON → utils.py → matplotlib
                 │
                 ├─→ [Animate] → FuncAnimation → GIF export
                 │
                 └─→ [Generate Report] → generate_report.py
                                         ↓
                                    RAPPORT_CALCUL.txt
```

---

## Key Features

### 1. **Visualization**
- Plot influence lines, moments, shear forces, rotations, deflections
- Support reactions and support moments
- Grid, span markers, node markers, legend display

### 2. **Animation**
- Animate selected curves
- Animate complete curves
- Configurable animation speed
- Export to GIF format

### 3. **Configuration**
- Persistent settings in Configuration.json
- Display options (grid, legend, axes)
- Style preferences
- Animation speed control

### 4. **Calculation**
- Launch C++ calculation engine
- Load results from JSON files
- Dynamic report generation
- Support for multiple analysis types

### 5. **Reporting**
- Automatic report generation from JSON data
- Text format with pandas tables
- HTML formatted reports
- Structural properties and analysis results

---

## Dependencies

### Python Packages (requirements.txt):
- PyQt6 - GUI framework
- matplotlib - Plotting and animation
- numpy - Numerical computations
- pandas - Data manipulation and tables
- Pillow - Image processing (for GIF export)

### External:
- Ligne d'influence.exe - C++ calculation engine

---

## Configuration Workflow

```
1. User opens application
   ↓
2. Load Configuration.json
   ↓
3. Sync with utils.current_config
   ↓
4. Display in GUI controls
   ↓
5. User modifies settings
   ↓
6. Apply changes to config dict
   ↓
7. Persist to Configuration.json
   ↓
8. Sync with utils module
```

---

## File I/O Operations

### Reading:
- `Configuration.json` - App settings
- `input.txt` - Calculation input
- `data/results/analysis/*.json` - Analysis results
- `data/results/properties/*.json` - Structural properties
- `assets/*.ico` - Application icons

### Writing:
- `Configuration.json` - Save settings
- `RAPPORT_CALCUL.txt` - Generated report
- `animation.gif` - Exported animation

---

## Menu Structure

```
📁 Fichier (File)
├── ✏️ Édition (Edit)
│   └── Éditer input.txt
├── ⚙️ Calcul (Calculation)
│   └── Lancer le Calcul C++
├── 💾 Sauvegarde & Export (Save & Export)
│   ├── Sauvegarder Configuration
│   └── Exporter Animation (GIF)
└── ❌ Quitter (Quit)

📊 Visualisation (View)
├── 📈 Tracer (Plot)
├── 🎬 Animation (Animate)
│   ├── Animer (Courbe Sélectionnée)
│   └── Animer (Complet)
└── ⭐ Afficher Maximum (Show Maximum)

⚙️ Configuration (Config)
├── 👁️ Affichage (Display)
│   ├── Grille (Grid)
│   ├── Travées (Spans)
│   ├── Nœuds (Nodes)
│   └── Légende (Legend)
├── 📐 Axes (Axes)
│   └── Inverser l'Axe Y (Invert Y Axis)
└── 🎨 Style (Style)
    └── Style Matplotlib par Défaut

❓ Aide (Help)
├── 📖 Aide (Help)
├── ℹ️ À Propos (About)
└── 🎮 Jeu (Game)
```

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Q` | Quit application |
| `Ctrl+P` | Plot current selection |
| `Ctrl+A` | Animate current selection |
| `Ctrl+Shift+A` | Animate full curve |
| `Ctrl+M` | Show maximum |
| `F11` | Toggle fullscreen |

---

## Error Handling

### Configuration Loading:
- Fallback to `DEFAULT_CONFIG` if JSON load fails
- Merge loaded config with defaults
- Sync with utils module

### JSON Loading (Reports):
- Try/except with error messages
- Return None on failure
- Continue processing other files

### Calculation Execution:
- Run in separate thread to prevent UI freeze
- Emit signals on completion or error
- Display error dialogs to user

---

## Threading Model

```
Main Thread (PyQt Event Loop)
├── UI Rendering
├── User Input Handling
└── Signal/Slot Connections

Worker Thread (CalculationWorker)
└── C++ Calculation Execution
    └── Emit finished/error signals back to main thread
```

---

## Notes for Developers

### Adding New Curve Types:
1. Add to curve selection in `create_curve_selection()`
2. Add corresponding JSON file in `data/results/analysis/`
3. Update `utils.py` to handle new data format
4. Update report generation in `generate_report.py`

### Modifying Configuration:
1. Update `DEFAULT_CONFIG` in `utils.py`
2. Update Configuration.json structure
3. Update UI controls in `create_left_panel()`
4. Update preferences dialog in `show_preferences()`

### Adding New Display Options:
1. Add checkbox in left panel
2. Add menu action in `create_menu_bar()`
3. Add toggle handler in `toggle_option()`
4. Update plot rendering logic

---

## Performance Considerations

- **Data Caching:** `self.data_cache` stores loaded JSON to avoid repeated file I/O
- **Threading:** Calculations run in separate thread to keep UI responsive
- **Animation:** FuncAnimation uses efficient matplotlib rendering
- **Configuration:** Loaded once at startup, synced with utils module

---

## Testing Checklist

- [ ] Configuration save/load
- [ ] Plot rendering for all curve types
- [ ] Animation playback and export
- [ ] Calculation execution
- [ ] Report generation
- [ ] Menu and keyboard shortcuts
- [ ] Display option toggles
- [ ] Preferences dialog
- [ ] Error handling and recovery

---

**Last Updated:** November 22, 2025  
**Project:** Ligne d'Influence - Structural Analysis for Multi-Span Bridges
