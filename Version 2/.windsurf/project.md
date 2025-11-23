# Ligne d'Influence - Structural Analysis

## Project Overview
Multi-span bridge structural analysis application with PyQt6 GUI for visualizing influence lines, moments, shear forces, rotations, and deflections.

## Quick Start
```bash
python main.py
```

## Project Structure

### Core Application
- **main.py** - Entry point
- **src/lñ.py** - Main GUI application (PyQt6)
- **src/utils.py** - Utility functions and configuration

### Configuration
- **config/Configuration.json** - App settings
- **config/input.txt** - Input for C++ calculation
- **config/automation_config.py** - Automation settings

### Reports
- **reports/generate_report.py** - Report generation
- **reports/RAPPORT_CALCUL.txt** - Generated text report
- **reports/RAPPORT_CALCUL.html** - Generated HTML report

### Utilities
- **utils_modules/help_content.py** - Help and about dialogs
- **utils_modules/styles.py** - Dark theme stylesheet

### Data
- **data/results/analysis/** - Analysis results (JSON)
- **data/results/properties/** - Structural properties (JSON)

## Key Features

### 1. Visualization
- Plot influence lines, moments, shear forces, rotations, deflections
- Support reactions and support moments
- Configurable display options (grid, legend, spans, nodes)

### 2. Animation
- Animate selected curves
- Animate complete curves
- Configurable animation speed
- Export to GIF format

### 3. Configuration
- Persistent settings in Configuration.json
- Display options toggle
- Style preferences
- Animation speed control

### 4. Calculation
- Launch C++ calculation engine
- Load results from JSON files
- Dynamic report generation

### 5. Reporting
- Automatic report generation from JSON data
- Text format with pandas tables
- HTML formatted reports

## Curve Types
- `span_moments` - Moments de Travée
- `span_shear_forces` - Forces de Cisaillement
- `span_rotations` - Rotations
- `span_deflections` - Deflexions
- `support_moments` - Moments d'Appui
- `support_reactions` - Réactions d'Appui

## Menu Structure

### 📁 Fichier (File)
- ✏️ Édition - Edit input.txt
- ⚙️ Calcul - Launch C++ calculation
- 💾 Sauvegarde & Export - Save config, export GIF
- ❌ Quitter - Quit

### 📊 Visualisation (View)
- 📈 Tracer - Plot selected curve
- 🎬 Animation - Animate curves
- ⭐ Afficher Maximum - Show max values

### ⚙️ Configuration (Config)
- 👁️ Affichage - Display options
- 📐 Axes - Axis options
- 🎨 Style - Style preferences

### ❓ Aide (Help)
- 📖 Guide - User guide
- ℹ️ À Propos - About dialog
- 📊 Rapport - Text report

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+Q | Quit application |
| Ctrl+P | Plot current selection |
| Ctrl+A | Animate current selection |
| Ctrl+Shift+A | Animate full curve |
| Ctrl+M | Show maximum |
| F11 | Toggle fullscreen |
| F1 | Show help |

## Dependencies

### Python Packages
- PyQt6 - GUI framework
- matplotlib - Plotting and animation
- numpy - Numerical computations
- pandas - Data manipulation and tables
- Pillow - Image processing

### External
- Ligne d'influence.exe - C++ calculation engine

## Data Flow

1. **User Configuration** - Select options in GUI (lñ.py)
2. **Input/Calculation** - Edit input.txt or run C++ calculation
3. **Results Storage** - Results saved to data/results/
4. **Data Loading** - Load JSON via utils.py
5. **Rendering** - Plot with matplotlib or generate reports
6. **Output** - Display in GUI or export as GIF/text/HTML

## Configuration Options

```json
{
  "grid": true,
  "travee": true,
  "noeud": true,
  "legend": true,
  "axe_y_inverser": false,
  "default_matplotlib_style": false,
  "vitesse_bridge": 0.050,
  "max_area": false
}
```

## File I/O

### Reading
- Configuration.json - App settings
- input.txt - Calculation input
- data/results/analysis/*.json - Analysis results
- data/results/properties/*.json - Structural properties
- assets/*.ico - Application icons

### Writing
- Configuration.json - Save settings
- RAPPORT_CALCUL.txt - Generated report
- animation.gif - Exported animation

## Threading Model

- **Main Thread** - PyQt event loop for UI rendering and user input
- **Worker Thread** - CalculationWorker for C++ calculation execution

## Performance Notes

- Data caching in `self.data_cache` to avoid repeated file I/O
- Threading keeps UI responsive during calculations
- FuncAnimation uses efficient matplotlib rendering
- Configuration loaded once at startup

## Key Classes

### CalculationWorker (QObject)
Runs C++ calculations in a separate thread
- Signals: `finished(exe_name, working_dir)`, `error(error_message)`
- Methods: `run_calculation(exe_path, working_dir)`

### StructuralAnalysisGUI (QMainWindow)
Main application window
- Configuration management
- Plot and animation rendering
- Menu and toolbar creation

## Key Methods

| Method | Purpose |
|--------|---------|
| `load_configuration()` | Load settings from Configuration.json |
| `setup_ui()` | Create main UI layout with splitter |
| `create_menu_bar()` | Create menu system |
| `plot_current_selection()` | Plot selected curve |
| `animate_current_selection()` | Animate selected curve |
| `run_calculation()` | Launch C++ calculation |
| `export_animation()` | Export animation as GIF |

## Development Notes

### Adding New Curve Types
1. Add to curve selection in `create_curve_selection()`
2. Add corresponding JSON file in `data/results/analysis/`
3. Update `utils.py` to handle new data format
4. Update report generation in `generate_report.py`

### Modifying Configuration
1. Update `DEFAULT_CONFIG` in `utils.py`
2. Update Configuration.json structure
3. Update UI controls in `create_left_panel()`
4. Update preferences dialog

### Adding Display Options
1. Add checkbox in left panel
2. Add menu action in `create_menu_bar()`
3. Add toggle handler in `toggle_option()`
4. Update plot rendering logic

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

## Related Files

- `CODE_MAP.md` - Detailed code documentation
- `ORGANISATION.md` - File organization guide
- `README.md` - Project documentation
