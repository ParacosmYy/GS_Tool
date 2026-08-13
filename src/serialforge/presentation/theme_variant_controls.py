"""Theme-aware native-control selectors for the presentation layer."""

from __future__ import annotations

from .theme_tokens import ThemeSpec


def build_combo_popup_theme_override(theme: ThemeSpec) -> str:
    """Render the top-level frame around a native combo popup."""

    return f"""
QFrame[surfaceRole="comboPopup"] {{
    background: {theme.surface};
    border-color: {theme.border};
}}
"""


def build_file_dialog_theme_override(theme: ThemeSpec) -> str:
    """Render the theme-specific rules for the Qt file-dialog surface."""

    return f"""
QFileDialog {{
    background: {theme.surface};
    color: {theme.text};
    border-color: {theme.border};
}}

QFileDialog QWidget {{
    background: {theme.surface};
    color: {theme.text};
}}

QFileDialog QTreeView,
QFileDialog QListView {{
    background: {theme.surface_input};
    alternate-background-color: {theme.surface};
    color: {theme.text};
    border-color: {theme.border};
    selection-background-color: {theme.selection_background};
    selection-color: {theme.selection_text};
}}

QFileDialog QTreeView::item:hover,
QFileDialog QListView::item:hover {{
    background: {theme.interactive_hover};
    color: {theme.text};
}}

QFileDialog QTreeView::item:selected,
QFileDialog QListView::item:selected {{
    background: {theme.selection_background};
    color: {theme.selection_text};
}}

QFileDialog QHeaderView::section {{
    background: {theme.surface};
    color: {theme.text_muted};
    border-color: {theme.border};
}}

QFileDialog QLineEdit,
QFileDialog QComboBox {{
    background: {theme.surface_input};
    color: {theme.text};
    border-color: {theme.border};
}}

QFileDialog QLineEdit:focus,
QFileDialog QComboBox:focus {{
    border-color: {theme.focus};
}}

QFileDialog QToolButton,
QFileDialog QPushButton {{
    background: {theme.surface};
    color: {theme.text};
    border-color: {theme.border};
}}

QFileDialog QToolButton:hover,
QFileDialog QPushButton:hover {{
    background: {theme.interactive_hover};
    border-color: {theme.accent_pink};
}}

QFileDialog QToolButton:pressed,
QFileDialog QPushButton:pressed {{
    background: {theme.interactive_pressed};
    border-color: {theme.accent};
}}

QFileDialog QToolButton:disabled,
QFileDialog QPushButton:disabled {{
    background: {theme.disabled_surface};
    color: {theme.disabled_text};
    border-color: {theme.disabled_border};
}}

QFileDialog QSidebar {{
    background: {theme.surface};
    color: {theme.text};
    border-right-color: {theme.border};
}}

QFileDialog QScrollBar {{
    background: {theme.surface_input};
}}

QFileDialog QScrollBar::handle {{
    background: {theme.border};
}}

QFileDialog QScrollBar::handle:hover {{
    background: {theme.accent_pink};
}}

QFileDialog QAbstractScrollArea::corner {{
    background: {theme.surface_input};
}}
"""


def build_controls_theme_override(theme: ThemeSpec) -> str:
    """Render inputs, navigation, tables, scroll surfaces, and motion controls."""

    return build_combo_popup_theme_override(theme) + build_file_dialog_theme_override(theme) + f"""
QLabel#pausedState[state="idle"],
QLabel#recordState[state="idle"] {{
    color: {theme.text_subtle};
    background: transparent;
    border-color: transparent;
}}

QLabel#pausedState[state="paused"],
QLabel#recordState[state="stopping"] {{
    color: {theme.warning};
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
}}

QLabel#recordState[state="active"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#recordState[state="error"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QWidget[role="surface"] {{
    background: {theme.surface};
    border-color: {theme.border};
}}

QWidget#errorBar {{
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QLineEdit,
QPlainTextEdit,
QComboBox,
QSpinBox,
QDoubleSpinBox {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {theme.surface}, stop:1 {theme.surface_input});
    color: {theme.text};
    border-color: {theme.border};
    selection-background-color: {theme.selection_background};
    selection-color: {theme.selection_text};
}}

QLineEdit:focus, QPlainTextEdit:focus, QComboBox:focus,
QSpinBox:focus, QDoubleSpinBox:focus {{
    border-color: {theme.focus};
}}

QLineEdit:disabled, QPlainTextEdit:disabled, QComboBox:disabled,
QSpinBox:disabled, QDoubleSpinBox:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QSpinBox::up-button,
QSpinBox::down-button,
QDoubleSpinBox::up-button,
QDoubleSpinBox::down-button {{
    background: {theme.surface_input};
    border-color: transparent;
}}

QSpinBox::up-button:hover,
QSpinBox::down-button:hover,
QDoubleSpinBox::up-button:hover,
QDoubleSpinBox::down-button:hover {{
    background: {theme.interactive_hover};
}}

QSpinBox::up-button:disabled,
QSpinBox::down-button:disabled,
QDoubleSpinBox::up-button:disabled,
QDoubleSpinBox::down-button:disabled {{
    background: {theme.disabled_surface};
}}

QSpinBox::up-arrow,
QDoubleSpinBox::up-arrow {{
    border-bottom-color: {theme.text_muted};
}}

QSpinBox::down-arrow,
QDoubleSpinBox::down-arrow {{
    border-top-color: {theme.text_muted};
}}

QSpinBox::up-arrow:disabled,
QSpinBox::down-arrow:disabled,
QDoubleSpinBox::up-arrow:disabled,
QDoubleSpinBox::down-arrow:disabled {{
    border-bottom-color: {theme.disabled_text};
    border-top-color: {theme.disabled_text};
}}

QComboBox::drop-down {{
    border-left-color: {theme.border};
}}

QComboBox::drop-down:disabled {{
    background: {theme.disabled_surface};
    border-left-color: {theme.disabled_border};
}}

QComboBox::drop-down:hover {{
    background: {theme.interactive_hover};
    border-left-color: {theme.accent_pink};
}}

QComboBox QAbstractItemView {{
    background: {theme.surface};
    color: {theme.text};
    border-color: {theme.border};
    selection-background-color: {theme.selection_background};
    selection-color: {theme.selection_text};
}}

QComboBox QAbstractItemView::item {{
    background: transparent;
    color: {theme.text};
}}

QComboBox QAbstractItemView::item:hover {{
    background: {theme.interactive_hover};
    color: {theme.text};
}}

QComboBox QAbstractItemView::item:selected {{
    background: {theme.selection_background};
    color: {theme.selection_text};
}}

QComboBox QAbstractItemView::item:disabled {{
    background: transparent;
    color: {theme.disabled_text};
}}

QMenu {{
    background: {theme.surface};
    color: {theme.text};
    border-color: {theme.border};
}}

QMenu::item:selected {{
    background: {theme.interactive_hover};
    color: {theme.text};
}}

QMenu::item:disabled {{
    color: {theme.disabled_text};
}}

QMenu::separator {{
    background: {theme.border};
}}

QMessageBox[surfaceRole="confirmation"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.warning_surface}, stop:1 {theme.surface});
    color: {theme.text};
    border-color: {theme.warning_border};
}}

QMessageBox[surfaceRole="confirmation"] QLabel {{
    color: {theme.text};
}}

QMessageBox[surfaceRole="confirmation"] QLabel#qt_msgbox_informativelabel {{
    color: {theme.text_muted};
}}

QMessageBox[surfaceRole="confirmation"] QPushButton {{
    min-width: 76px;
}}

QWidget#datasetCurve {{
    background: {theme.surface};
    border-color: {theme.border};
}}

QWidget#datasetCurve:focus {{
    border-color: {theme.focus};
}}

QPushButton, QToolButton {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface});
    color: {theme.text};
    border-color: {theme.history_border};
}}

QPushButton:hover, QToolButton:hover {{
    background: {theme.interactive_hover};
    border-color: {theme.accent_pink};
}}

QPushButton:pressed, QToolButton:pressed {{
    background: {theme.interactive_pressed};
    border-color: {theme.focus};
}}

QPushButton:focus, QToolButton:focus,
QTabWidget#workspaceTabs QTabBar::tab:focus {{
    border-color: {theme.focus};
}}

QPushButton:disabled, QToolButton:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QPushButton#primaryButton {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.accent}, stop:0.58 {theme.accent_purple},
                                stop:1 {theme.accent_pink});
    color: {theme.on_accent};
    border-color: {theme.success_border};
}}

QPushButton#primaryButton:hover {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.accent}, stop:0.55 {theme.success},
                                stop:1 {theme.accent_pink});
}}

QPushButton#primaryButton:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QPushButton#dangerButton {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QPushButton#dangerButton:focus {{
    border-color: {theme.focus};
}}

QPushButton#dangerButton:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QCheckBox::indicator {{
    border-color: {theme.history_border};
    background: {theme.surface_input};
}}

QCheckBox::indicator:hover {{
    border-color: {theme.accent_pink};
}}

QCheckBox::indicator:checked {{
    background: {theme.accent_pink};
    border-color: {theme.selection_text};
}}

QCheckBox::indicator:disabled {{
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QCheckBox::indicator:disabled:checked {{
    background: {theme.neutral_border};
    border-color: {theme.text_subtle};
}}

QCheckBox:focus {{
    color: {theme.accent_pink};
}}

QCheckBox:disabled {{
    color: {theme.disabled_text};
}}

QCheckBox::indicator:focus {{
    border-color: {theme.focus};
}}

QTabWidget#workspaceTabs::pane {{
    background: {theme.surface};
    border-color: {theme.border};
}}

QFrame#workspaceShell {{
    background: {theme.surface};
    border-color: {theme.border};
}}

QFrame#workspaceRouteStrip {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.history_surface}, stop:1 {theme.info_surface});
    border-top-color: {theme.neutral_border};
    border-bottom-color: {theme.border};
}}

QFrame#workspaceShell[mode="focus"] QFrame#workspaceRouteStrip {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.info_surface}, stop:0.58 {theme.history_surface},
                                stop:1 {theme.surface});
    border-top: 2px solid {theme.accent_blue};
}}

QFrame#workspaceShell[mode="overview"] QFrame#workspaceRouteStrip {{
    border-top-color: {theme.neutral_border};
}}

QToolButton#workspaceFocusButton {{
    background: {theme.history_surface};
    color: {theme.text_muted};
    border-color: {theme.history_border};
}}

QToolButton#workspaceFocusButton:hover {{
    background: {theme.interactive_hover};
    color: {theme.text};
    border-color: {theme.accent_pink};
}}

QToolButton#workspaceFocusButton:checked {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.info_surface}, stop:1 {theme.history_surface});
    color: {theme.text};
    border-color: {theme.accent};
}}

QToolButton#workspaceFocusButton:pressed {{
    background: {theme.interactive_pressed};
    border-color: {theme.focus};
}}

QToolButton#workspaceFocusButton:focus {{
    border: 2px solid {theme.focus};
}}

QLabel#workspaceScrollHint {{
    background: {theme.history_surface};
    color: {theme.text_muted};
    border-color: {theme.history_border};
}}

QLabel#workspaceContextLabel {{
    background: transparent;
    color: {theme.text_muted};
    border-left-color: {theme.history_border};
}}

QLabel#workspaceContextLabel[state="connection"] {{
    color: {theme.accent_blue};
    border-left-color: {theme.info_border};
}}

QLabel#workspaceContextLabel[state="protocol"] {{
    color: {theme.accent};
    border-left-color: {theme.success_border};
}}

QLabel#workspaceContextLabel[state="commands"] {{
    color: {theme.accent_pink};
    border-left-color: {theme.accent_pink};
}}

QLabel#workspaceContextLabel[state="extension"] {{
    color: {theme.accent_purple};
    border-left-color: {theme.history_border};
}}

QLabel#workspaceScrollHint[state="top"] {{
    color: {theme.accent_blue};
    border-color: {theme.info_border};
}}

QLabel#workspaceScrollHint[state="middle"] {{
    color: {theme.accent_pink};
    border-color: {theme.accent_pink};
}}

QLabel#workspaceScrollHint[state="bottom"] {{
    color: {theme.accent_purple};
    border-color: {theme.history_border};
}}

QLabel#workspaceScrollHint[state="complete"] {{
    color: {theme.accent};
    border-color: {theme.success_border};
}}

QTabWidget#workspaceTabs,
QTabWidget#workspaceTabs > QStackedWidget {{
    background: transparent;
}}

QTabWidget#workspaceTabs QTabBar::tab {{
    background: {theme.neutral_surface};
    color: {theme.text_muted};
}}

QTabWidget#workspaceTabs QTabBar::tab:hover {{
    color: {theme.text};
    background: {theme.interactive_hover};
}}

QTabWidget#workspaceTabs QTabBar::tab:selected {{
    color: {theme.text};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.history_surface}, stop:1 {theme.info_surface});
    border-color: {theme.history_border};
}}

QTabWidget#workspaceTabs QTabBar::tab:selected:focus {{
    border-color: {theme.focus};
}}

QTabWidget#workspaceTabs QTabBar QToolButton {{
    background: {theme.neutral_surface};
    color: {theme.text_muted};
    border-color: {theme.neutral_border};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:hover {{
    background: {theme.interactive_hover};
    color: {theme.text};
    border-color: {theme.accent_pink};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:pressed {{
    background: {theme.interactive_pressed};
    color: {theme.accent};
    border-color: {theme.focus};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:focus {{
    border-color: {theme.focus};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:disabled {{
    background: {theme.disabled_surface};
    color: {theme.disabled_text};
    border-color: {theme.disabled_border};
}}

QScrollArea#settingsScroll {{
    background: transparent;
    border-color: transparent;
}}

QScrollArea#settingsScroll > QWidget#settingsViewport,
QScrollArea#settingsScroll > QWidget[role="scrollViewport"] {{
    background: {theme.surface};
}}

QWidget#connectionPage,
QWidget#protocolPage,
QWidget#commandPage,
QScrollArea#settingsScroll > QWidget#settingsViewport > QWidget[role="scrollContent"],
QScrollArea#settingsScroll > QWidget[role="scrollViewport"] > QWidget[role="scrollContent"] {{
    background: transparent;
}}

QPlainTextEdit#terminal {{
    background: {theme.terminal_background};
    color: {theme.terminal_text};
    border-color: {theme.history_border};
    selection-background-color: {theme.selection_background};
}}

QFrame#terminalSurface {{
    background: transparent;
    border: 0;
}}

QFrame#terminalEmptyState {{
    background: transparent;
}}

QFrame#terminalEmptyCard {{
    background: {theme.info_surface};
    border-color: {theme.info_border};
    border-left-color: {theme.accent_blue};
}}

QFrame#terminalEmptyCard[state="waiting"] {{
    background: {theme.success_surface};
    border-color: {theme.success_border};
    border-left-color: {theme.accent};
}}

QFrame#terminalEmptyCard[state="transition"],
QFrame#terminalEmptyCard[state="paused"] {{
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
    border-left-color: {theme.warning};
}}

QFrame#terminalEmptyCard[state="history"] {{
    background: {theme.history_surface};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_purple};
}}

QLabel#terminalEmptyEyebrow {{
    color: {theme.accent_blue};
}}

QLabel#terminalEmptyTitle {{
    color: {theme.text};
}}

QLabel#terminalEmptyHint {{
    color: {theme.text_muted};
}}

QFrame#commandBatchEmptyState {{
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
    border-left-color: {theme.accent_purple};
}}

QLabel#commandBatchEmptyEyebrow {{
    color: {theme.accent_purple};
}}

QLabel#commandBatchEmptyTitle {{
    color: {theme.text};
}}

QLabel#commandBatchEmptyHint {{
    color: {theme.text_muted};
}}

QFrame#commandBatchStep {{
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#commandBatchStepIndex {{
    color: {theme.accent};
}}

QLabel#commandBatchStepTitle {{
    color: {theme.text};
}}

QLabel#commandBatchStepHint {{
    color: {theme.text_muted};
}}

QTableWidget {{
    background: {theme.surface_input};
    color: {theme.text};
    alternate-background-color: {theme.neutral_surface};
    gridline-color: {theme.border};
    border-color: {theme.border};
    selection-background-color: {theme.selection_background};
    selection-color: {theme.selection_text};
}}

QTableWidget:focus {{
    border-color: {theme.focus};
}}

QTableWidget::item:hover {{
    background: {theme.interactive_hover};
}}

QHeaderView::section {{
    background: {theme.history_surface};
    color: {theme.text};
    border-bottom-color: {theme.history_border};
}}

QStatusBar {{
    background: {theme.background};
    color: {theme.text_muted};
    border-top-color: {theme.history_border};
}}

QStatusBar::item {{
    background: transparent;
    border-color: transparent;
}}

QStatusBar QLabel {{
    background: transparent;
    border-color: transparent;
    color: {theme.text_muted};
}}

QScrollBar:vertical,
QScrollBar:horizontal,
QAbstractScrollArea::corner {{
    background: {theme.surface_input};
}}

QScrollBar::handle:vertical,
QScrollBar::handle:horizontal {{
    background: {theme.accent_purple};
    border-color: {theme.surface_input};
}}

QScrollBar::handle:vertical:hover,
QScrollBar::handle:horizontal:hover {{
    background: {theme.accent_pink};
}}

QScrollBar::handle:vertical:pressed,
QScrollBar::handle:horizontal:pressed {{
    background: {theme.accent};
}}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical,
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {{
    background: transparent;
}}

QToolTip {{
    background: {theme.surface};
    color: {theme.text};
    border-color: {theme.history_border};
}}
"""


__all__ = [
    "build_combo_popup_theme_override",
    "build_controls_theme_override",
    "build_file_dialog_theme_override",
]
