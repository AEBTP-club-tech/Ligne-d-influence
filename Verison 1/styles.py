"""
Module contenant les styles CSS pour l'application d'analyse structurelle.
"""

DARK_THEME_STYLESHEET = """
QMainWindow {
    background-color: #2b2b2b;
}
QWidget {
    background-color: #2b2b2b;
    color: #ffffff;
}
QGroupBox {
    border: 2px solid #3d3d3d;
    border-radius: 5px;
    margin-top: 10px;
    font-weight: bold;
    padding-top: 10px;
}
QGroupBox::title {
    color: #4a9eff;
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 5px;
}
QPushButton {
    background-color: #3d3d3d;
    color: #ffffff;
    border: none;
    padding: 8px;
    border-radius: 4px;
    font-weight: bold;
}
QPushButton:hover {
    background-color: #4a9eff;
}
QPushButton:pressed {
    background-color: #2d7acc;
}
QComboBox, QSpinBox {
    background-color: #3d3d3d;
    color: #ffffff;
    border: 1px solid #555555;
    padding: 5px;
    border-radius: 3px;
}
QCheckBox {
    spacing: 5px;
}
QCheckBox::indicator {
    width: 18px;
    height: 18px;
}
QCheckBox::indicator:unchecked {
    background-color: #3d3d3d;
    border: 2px solid #555555;
    border-radius: 3px;
}
QCheckBox::indicator:checked {
    background-color: #4a9eff;
    border: 2px solid #4a9eff;
    border-radius: 3px;
}
QSlider::groove:horizontal {
    height: 8px;
    background: #3d3d3d;
    border-radius: 4px;
}
QSlider::handle:horizontal {
    background: #4a9eff;
    width: 18px;
    margin: -5px 0;
    border-radius: 9px;
}
QLabel {
    color: #ffffff;
}
QMenuBar {
    background-color: #2b2b2b;
    color: #ffffff;
    border-bottom: 1px solid #3d3d3d;
}
QMenuBar::item {
    background-color: transparent;
    padding: 8px 12px;
}
QMenuBar::item:selected {
    background-color: #4a9eff;
}
QMenu {
    background-color: #2b2b2b;
    color: #ffffff;
    border: 1px solid #3d3d3d;
}
QMenu::item {
    padding: 6px 30px 6px 20px;
}
QMenu::item:selected {
    background-color: #4a9eff;
}
QMenu::separator {
    height: 1px;
    background: #3d3d3d;
    margin: 4px 0;
}
QToolBar {
    background-color: #2b2b2b;
    border: 1px solid #3d3d3d;
    spacing: 3px;
}
QToolButton {
    background-color: #3d3d3d;
    border: 1px solid #555555;
    border-radius: 3px;
    padding: 4px;
    margin: 2px;
    min-width: 24px;
    min-height: 24px;
    font-size: 16px;
}
QToolButton:hover {
    background-color: #4a9eff;
    border-color: #4a9eff;
}
QToolButton:pressed {
    background-color: #2d7acc;
}
QToolBar::separator {
    background: #3d3d3d;
    width: 1px;
    margin: 4px 2px;
}
"""
