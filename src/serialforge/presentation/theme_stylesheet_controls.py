"""Widget and native-control stylesheet rules for the SerialForge shell."""

from __future__ import annotations

from .theme_tokens import (
    ACCENT,
    ACCENT_BLUE,
    ACCENT_PINK,
    ACCENT_PURPLE,
    ACCENT_STRONG,
    BACKGROUND,
    BORDER,
    BORDER_STRONG,
    DISABLED_BORDER,
    DISABLED_SURFACE,
    DISABLED_TEXT,
    ERROR,
    ERROR_BORDER,
    ERROR_SURFACE,
    FOCUS,
    HISTORY_BORDER,
    HISTORY_SURFACE,
    INFO_BORDER,
    INFO_SURFACE,
    INTERACTIVE_HOVER,
    INTERACTIVE_PRESSED,
    NEUTRAL_BORDER,
    NEUTRAL_SURFACE,
    ON_ACCENT,
    SELECTION_BACKGROUND,
    SELECTION_TEXT,
    SUCCESS,
    SUCCESS_BORDER,
    SUCCESS_SURFACE,
    SURFACE,
    SURFACE_INPUT,
    SURFACE_RAISED,
    TERMINAL_BACKGROUND,
    TERMINAL_TEXT,
    TEXT,
    TEXT_MUTED,
    TEXT_SUBTLE,
    WARNING,
    WARNING_BORDER,
    WARNING_SURFACE,
)

COMBO_POPUP_STYLESHEET = f"""
QFrame[surfaceRole="comboPopup"] {{
    background: {SURFACE_RAISED};
    border: 1px solid {BORDER_STRONG};
    border-radius: 9px;
}}
"""

FILE_DIALOG_STYLESHEET = f"""
QFileDialog {{
    background: {SURFACE};
    color: {TEXT};
    border: 1px solid {BORDER};
}}

QFileDialog QWidget {{
    background: {SURFACE};
    color: {TEXT};
}}

QFileDialog QTreeView,
QFileDialog QListView {{
    background: {SURFACE_INPUT};
    alternate-background-color: {SURFACE};
    color: {TEXT};
    border: 1px solid {BORDER};
    selection-background-color: {SELECTION_BACKGROUND};
    selection-color: {SELECTION_TEXT};
}}

QFileDialog QTreeView::item,
QFileDialog QListView::item {{
    padding: 4px 6px;
}}

QFileDialog QTreeView::item:hover,
QFileDialog QListView::item:hover {{
    background: {INTERACTIVE_HOVER};
    color: {TEXT};
}}

QFileDialog QTreeView::item:selected,
QFileDialog QListView::item:selected {{
    background: {SELECTION_BACKGROUND};
    color: {SELECTION_TEXT};
}}

QFileDialog QHeaderView::section {{
    background: {SURFACE};
    color: {TEXT_MUTED};
    border: 1px solid {BORDER};
    padding: 4px 6px;
}}

QFileDialog QLineEdit,
QFileDialog QComboBox {{
    background: {SURFACE_INPUT};
    color: {TEXT};
    border: 1px solid {BORDER};
    border-radius: 7px;
    padding: 5px 8px;
}}

QFileDialog QLineEdit:focus,
QFileDialog QComboBox:focus {{
    border: 2px solid {FOCUS};
}}

QFileDialog QToolButton,
QFileDialog QPushButton {{
    background: {SURFACE};
    color: {TEXT};
    border: 1px solid {BORDER};
    border-radius: 7px;
    padding: 5px 9px;
}}

QFileDialog QToolButton:hover,
QFileDialog QPushButton:hover {{
    background: {INTERACTIVE_HOVER};
    border-color: {ACCENT_PINK};
}}

QFileDialog QToolButton:pressed,
QFileDialog QPushButton:pressed {{
    background: {INTERACTIVE_PRESSED};
    border-color: {ACCENT};
}}

QFileDialog QToolButton:disabled,
QFileDialog QPushButton:disabled {{
    background: {DISABLED_SURFACE};
    color: {DISABLED_TEXT};
    border-color: {DISABLED_BORDER};
}}

QFileDialog QDialogButtonBox QPushButton {{
    min-width: 76px;
}}

QFileDialog QSidebar {{
    background: {SURFACE};
    color: {TEXT};
    border-right: 1px solid {BORDER};
}}

QFileDialog QScrollBar {{
    background: {SURFACE_INPUT};
}}

QFileDialog QScrollBar::handle {{
    background: {BORDER_STRONG};
    border-radius: 4px;
    min-height: 24px;
    min-width: 24px;
}}

QFileDialog QScrollBar::handle:hover {{
    background: {ACCENT_PINK};
}}

QFileDialog QAbstractScrollArea::corner {{
    background: {SURFACE_INPUT};
}}
"""

CONTROLS_STYLESHEET = COMBO_POPUP_STYLESHEET + FILE_DIALOG_STYLESHEET + f"""

QLabel#pausedState[state="idle"],
QLabel#recordState[state="idle"] {{
    color: {TEXT_SUBTLE};
    background: transparent;
    border-color: transparent;
}}

QLabel#pausedState[state="paused"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QLabel#recordState[state="active"] {{
    color: {SUCCESS};
    background: {SUCCESS_SURFACE};
    border-color: {SUCCESS_BORDER};
}}

QLabel#recordState[state="stopping"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QLabel#recordState[state="error"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QWidget[role="surface"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {SURFACE_RAISED}, stop:1 {SURFACE});
    border: 1px solid {BORDER};
    border-radius: 12px;
}}

QWidget#errorBar {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {ERROR_SURFACE}, stop:1 {SURFACE});
    border: 1px solid {ERROR_BORDER};
    border-radius: 10px;
}}

QLineEdit, QPlainTextEdit, QComboBox, QSpinBox, QDoubleSpinBox {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {SURFACE}, stop:1 {SURFACE_INPUT});
    color: {TEXT};
    border: 1px solid {BORDER};
    border-radius: 9px;
    padding: 7px 10px;
    selection-background-color: {SELECTION_BACKGROUND};
    selection-color: {SELECTION_TEXT};
}}

QLineEdit:focus, QPlainTextEdit:focus, QComboBox:focus,
QSpinBox:focus, QDoubleSpinBox:focus {{
    border: 2px solid {ACCENT_PINK};
}}

QLineEdit:disabled, QPlainTextEdit:disabled, QComboBox:disabled,
QSpinBox:disabled, QDoubleSpinBox:disabled {{
    color: {DISABLED_TEXT};
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QSpinBox::up-button,
QSpinBox::down-button,
QDoubleSpinBox::up-button,
QDoubleSpinBox::down-button {{
    subcontrol-origin: border;
    width: 18px;
    border: 0;
    background: {SURFACE_INPUT};
}}

QSpinBox::up-button,
QDoubleSpinBox::up-button {{
    subcontrol-position: top right;
}}

QSpinBox::down-button,
QDoubleSpinBox::down-button {{
    subcontrol-position: bottom right;
}}

QSpinBox::up-button:hover,
QSpinBox::down-button:hover,
QDoubleSpinBox::up-button:hover,
QDoubleSpinBox::down-button:hover {{
    background: {INTERACTIVE_HOVER};
}}

QSpinBox::up-button:disabled,
QSpinBox::down-button:disabled,
QDoubleSpinBox::up-button:disabled,
QDoubleSpinBox::down-button:disabled {{
    background: {DISABLED_SURFACE};
}}

QSpinBox::up-arrow,
QDoubleSpinBox::up-arrow {{
    width: 0;
    height: 0;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-bottom: 4px solid {TEXT_MUTED};
}}

QSpinBox::down-arrow,
QDoubleSpinBox::down-arrow {{
    width: 0;
    height: 0;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 4px solid {TEXT_MUTED};
}}

QSpinBox::up-arrow:disabled,
QSpinBox::down-arrow:disabled,
QDoubleSpinBox::up-arrow:disabled,
QDoubleSpinBox::down-arrow:disabled {{
    border-bottom-color: {DISABLED_TEXT};
    border-top-color: {DISABLED_TEXT};
}}

QComboBox::drop-down {{
    width: 24px;
    border: 0;
    border-left: 1px solid {BORDER};
}}

QComboBox::drop-down:disabled {{
    background: {DISABLED_SURFACE};
    border-left-color: {DISABLED_BORDER};
}}

QComboBox::drop-down:hover {{
    background: {INTERACTIVE_HOVER};
    border-left-color: {ACCENT_PINK};
}}

QComboBox QAbstractItemView {{
    background: {SURFACE_RAISED};
    color: {TEXT};
    border: 1px solid {HISTORY_BORDER};
    selection-background-color: {SELECTION_BACKGROUND};
    selection-color: {SELECTION_TEXT};
}}

QComboBox QAbstractItemView::item {{
    background: transparent;
    color: {TEXT};
    padding: 6px 10px;
    border-radius: 5px;
}}

QComboBox QAbstractItemView::item:hover {{
    background: {INTERACTIVE_HOVER};
    color: {SELECTION_TEXT};
}}

QComboBox QAbstractItemView::item:selected {{
    background: {SELECTION_BACKGROUND};
    color: {SELECTION_TEXT};
}}

QComboBox QAbstractItemView::item:disabled {{
    background: transparent;
    color: {DISABLED_TEXT};
}}

QMenu {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    color: {TEXT};
    border: 1px solid {HISTORY_BORDER};
    padding: 6px;
}}

QMenu::item {{
    padding: 7px 24px 7px 11px;
    border-radius: 7px;
}}

QMenu::item:selected {{
    background: {INTERACTIVE_HOVER};
    color: {SELECTION_TEXT};
}}

QMenu::item:disabled {{
    color: {DISABLED_TEXT};
}}

QMenu::separator {{
    height: 1px;
    background: {BORDER};
    margin: 4px 6px;
}}

QMessageBox[surfaceRole="confirmation"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {WARNING_SURFACE}, stop:1 {SURFACE});
    color: {TEXT};
    border: 1px solid {WARNING_BORDER};
    border-radius: 14px;
}}

QMessageBox[surfaceRole="confirmation"] QLabel {{
    background: transparent;
    color: {TEXT};
}}

QMessageBox[surfaceRole="confirmation"] QLabel#qt_msgbox_informativelabel {{
    color: {TEXT_MUTED};
}}

QMessageBox[surfaceRole="confirmation"] QPushButton {{
    min-width: 76px;
}}

QWidget#datasetCurve {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {SURFACE_RAISED}, stop:1 {SURFACE});
    border: 1px solid {BORDER};
    border-radius: 10px;
}}

QWidget#datasetCurve:focus {{
    border: 2px solid {ACCENT_PINK};
}}

QPushButton, QToolButton {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    color: {TEXT};
    border: 1px solid {HISTORY_BORDER};
    border-radius: 9px;
    padding: 7px 13px;
    min-height: 20px;
}}

QPushButton:hover, QToolButton:hover {{
    background: {INTERACTIVE_HOVER};
    border-color: {ACCENT_PINK};
}}

QPushButton:pressed, QToolButton:pressed {{
    background: {INTERACTIVE_PRESSED};
    border-color: {ACCENT};
}}

QPushButton:focus, QToolButton:focus,
QTabWidget#workspaceTabs QTabBar::tab:focus {{
    border: 2px solid {ACCENT};
}}

QPushButton:disabled, QToolButton:disabled {{
    color: {DISABLED_TEXT};
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QPushButton#primaryButton {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {ACCENT_STRONG}, stop:0.58 {ACCENT},
                                stop:1 {ACCENT_PURPLE});
    color: {ON_ACCENT};
    border-color: {SUCCESS_BORDER};
    font-weight: 700;
}}

QPushButton#primaryButton:hover {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {ACCENT}, stop:0.55 {SUCCESS},
                                stop:1 {ACCENT_PINK});
}}

QPushButton#primaryButton:disabled {{
    color: {DISABLED_TEXT};
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QPushButton#dangerButton {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QPushButton#dangerButton:focus {{
    border: 2px solid {ACCENT};
}}

QPushButton#dangerButton:disabled {{
    color: {DISABLED_TEXT};
    border-color: {DISABLED_BORDER};
}}

QCheckBox {{
    spacing: 7px;
}}

QCheckBox::indicator {{
    width: 15px;
    height: 15px;
    border: 1px solid {HISTORY_BORDER};
    border-radius: 6px;
    background: {SURFACE_INPUT};
}}

QCheckBox::indicator:hover {{
    border-color: {ACCENT_PINK};
}}

QCheckBox::indicator:checked {{
    background: {ACCENT_PINK};
    border-color: {SELECTION_TEXT};
}}

QCheckBox::indicator:disabled {{
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QCheckBox::indicator:disabled:checked {{
    background: {NEUTRAL_BORDER};
    border-color: {TEXT_SUBTLE};
}}

QCheckBox:focus {{
    color: {ACCENT_PINK};
}}

QCheckBox:disabled {{
    color: {DISABLED_TEXT};
}}

QCheckBox::indicator:focus {{
    border: 2px solid {ACCENT_PINK};
}}

QTabWidget#workspaceTabs::pane {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {SURFACE_RAISED}, stop:1 {SURFACE});
    border: 1px solid {BORDER};
    border-radius: 12px;
    top: -1px;
}}

QFrame#workspaceShell {{
    background: {SURFACE};
    border: 1px solid {BORDER};
    border-radius: 14px;
}}

QFrame#workspaceRouteStrip {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {INFO_SURFACE});
    border-top: 1px solid {NEUTRAL_BORDER};
    border-bottom: 1px solid {BORDER};
    border-bottom-left-radius: 12px;
    border-bottom-right-radius: 12px;
}}

QFrame#workspaceShell[mode="focus"] QFrame#workspaceRouteStrip {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {INFO_SURFACE}, stop:0.58 {HISTORY_SURFACE},
                                stop:1 {SURFACE_RAISED});
    border-top: 2px solid {ACCENT_BLUE};
}}

QFrame#workspaceShell[mode="overview"] QFrame#workspaceRouteStrip {{
    border-top-color: {NEUTRAL_BORDER};
}}

QToolButton#workspaceFocusButton {{
    background: {HISTORY_SURFACE};
    color: {TEXT_MUTED};
    border: 1px solid {HISTORY_BORDER};
    border-radius: 7px;
    padding: 2px 8px;
    min-height: 20px;
}}

QToolButton#workspaceFocusButton:hover {{
    background: {INTERACTIVE_HOVER};
    color: {TEXT};
    border-color: {ACCENT_PINK};
}}

QToolButton#workspaceFocusButton:checked {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {INFO_SURFACE}, stop:1 {HISTORY_SURFACE});
    color: {TEXT};
    border-color: {ACCENT};
}}

QToolButton#workspaceFocusButton:pressed {{
    background: {INTERACTIVE_PRESSED};
    border-color: {FOCUS};
}}

QToolButton#workspaceFocusButton:focus {{
    border: 2px solid {FOCUS};
}}

QLabel#workspaceScrollHint {{
    background: {HISTORY_SURFACE};
    color: {TEXT_MUTED};
    border: 1px solid {HISTORY_BORDER};
    border-radius: 8px;
    padding: 1px 8px;
    font-size: 8pt;
    font-weight: 600;
}}

QLabel#workspaceContextLabel {{
    background: transparent;
    color: {TEXT_MUTED};
    border-left: 1px solid {HISTORY_BORDER};
    padding: 1px 10px;
    font-size: 8pt;
}}

QLabel#workspaceContextLabel[state="connection"] {{
    color: {ACCENT_BLUE};
    border-left-color: {INFO_BORDER};
}}

QLabel#workspaceContextLabel[state="protocol"] {{
    color: {ACCENT};
    border-left-color: {SUCCESS_BORDER};
}}

QLabel#workspaceContextLabel[state="commands"] {{
    color: {ACCENT_PINK};
    border-left-color: {ACCENT_PINK};
}}

QLabel#workspaceContextLabel[state="extension"] {{
    color: {ACCENT_PURPLE};
    border-left-color: {HISTORY_BORDER};
}}

QLabel#workspaceScrollHint[state="top"] {{
    color: {ACCENT_BLUE};
    border-color: {INFO_BORDER};
}}

QLabel#workspaceScrollHint[state="middle"] {{
    color: {ACCENT_PINK};
    border-color: {ACCENT_PINK};
}}

QLabel#workspaceScrollHint[state="bottom"] {{
    color: {ACCENT_PURPLE};
    border-color: {HISTORY_BORDER};
}}

QLabel#workspaceScrollHint[state="complete"] {{
    color: {ACCENT};
    border-color: {SUCCESS_BORDER};
}}

QTabWidget#workspaceTabs,
QTabWidget#workspaceTabs > QStackedWidget {{
    background: transparent;
    border: 0;
}}

QTabWidget#workspaceTabs QTabBar::tab {{
    background: {NEUTRAL_SURFACE};
    color: {TEXT_MUTED};
    border: 1px solid transparent;
    border-bottom: 0;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 9px 17px;
    margin-right: 4px;
}}

QTabWidget#workspaceTabs QTabBar::tab:hover {{
    color: {TEXT};
    background: {INTERACTIVE_HOVER};
}}

QTabWidget#workspaceTabs QTabBar::tab:selected {{
    color: {TEXT};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {INFO_SURFACE});
    border-color: {HISTORY_BORDER};
    font-weight: 700;
}}

QTabWidget#workspaceTabs QTabBar::tab:selected:focus {{
    border: 2px solid {ACCENT};
}}

QTabWidget#workspaceTabs QTabBar QToolButton {{
    background: {NEUTRAL_SURFACE};
    color: {TEXT_MUTED};
    border: 1px solid {NEUTRAL_BORDER};
    border-radius: 7px;
    padding: 2px 5px;
    min-width: 20px;
    min-height: 20px;
}}

QTabWidget#workspaceTabs QTabBar QToolButton:hover {{
    background: {INTERACTIVE_HOVER};
    color: {TEXT};
    border-color: {ACCENT_PINK};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:pressed {{
    background: {INTERACTIVE_PRESSED};
    color: {ACCENT};
    border-color: {ACCENT};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:focus {{
    border: 2px solid {ACCENT};
}}

QTabWidget#workspaceTabs QTabBar QToolButton:disabled {{
    background: {DISABLED_SURFACE};
    color: {DISABLED_TEXT};
    border-color: {DISABLED_BORDER};
}}

QScrollArea#settingsScroll {{
    background: transparent;
    border: 0;
}}

QScrollArea#settingsScroll > QWidget#settingsViewport,
QScrollArea#settingsScroll > QWidget[role="scrollViewport"] {{
    background: {SURFACE};
    border: 0;
}}

QWidget#connectionPage,
QWidget#protocolPage,
QWidget#commandPage,
QScrollArea#settingsScroll > QWidget#settingsViewport > QWidget[role="scrollContent"],
QScrollArea#settingsScroll > QWidget[role="scrollViewport"] > QWidget[role="scrollContent"] {{
    background: transparent;
}}

QPlainTextEdit#terminal {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {TERMINAL_BACKGROUND}, stop:0.55 {BACKGROUND},
                                stop:1 {SURFACE});
    color: {TERMINAL_TEXT};
    border: 1px solid {HISTORY_BORDER};
    border-radius: 12px;
    padding: 12px;
    selection-background-color: {SELECTION_BACKGROUND};
    font-family: "Cascadia Mono", "Consolas", monospace;
    font-size: 10pt;
}}

QFrame#terminalSurface {{
    background: transparent;
    border: 0;
}}

QFrame#terminalEmptyState {{
    background: transparent;
}}

QFrame#terminalEmptyCard {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
    border: 1px solid {INFO_BORDER};
    border-left: 3px solid {ACCENT_BLUE};
    border-radius: 14px;
}}

QFrame#terminalEmptyCard[state="waiting"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {SURFACE});
    border-color: {SUCCESS_BORDER};
    border-left-color: {ACCENT};
}}

QFrame#terminalEmptyCard[state="transition"] {{
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QFrame#terminalEmptyCard[state="paused"] {{
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QFrame#terminalEmptyCard[state="history"] {{
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PURPLE};
}}

QLabel#terminalEmptyEyebrow {{
    color: {ACCENT_BLUE};
    font-size: 8pt;
    font-weight: 700;
    letter-spacing: 1px;
}}

QLabel#terminalEmptyTitle {{
    color: {TEXT};
    font-size: 14pt;
    font-weight: 700;
}}

QLabel#terminalEmptyHint {{
    color: {TEXT_MUTED};
    font-size: 9pt;
}}

QFrame#commandBatchEmptyState {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {NEUTRAL_SURFACE}, stop:1 {SURFACE});
    border: 1px dashed {NEUTRAL_BORDER};
    border-left: 3px solid {ACCENT_PURPLE};
    border-radius: 11px;
}}

QLabel#commandBatchEmptyEyebrow {{
    color: {ACCENT_PURPLE};
    font-size: 8pt;
    font-weight: 700;
    letter-spacing: 1px;
}}

QLabel#commandBatchEmptyTitle {{
    color: {TEXT};
    font-size: 11pt;
    font-weight: 700;
}}

QLabel#commandBatchEmptyHint {{
    color: {TEXT_MUTED};
    font-size: 9pt;
}}

QFrame#commandBatchStep {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border: 1px solid {HISTORY_BORDER};
    border-radius: 9px;
}}

QLabel#commandBatchStepIndex {{
    color: {ACCENT};
    font-size: 8pt;
    font-weight: 700;
    letter-spacing: 1px;
}}

QLabel#commandBatchStepTitle {{
    color: {TEXT};
    font-size: 10pt;
    font-weight: 700;
}}

QLabel#commandBatchStepHint {{
    color: {TEXT_MUTED};
    font-size: 8.5pt;
}}

QTableWidget {{
    background: {SURFACE_INPUT};
    color: {TEXT};
    alternate-background-color: {SURFACE};
    gridline-color: {BORDER};
    border: 1px solid {BORDER};
    border-radius: 9px;
    selection-background-color: {SELECTION_BACKGROUND};
    selection-color: {SELECTION_TEXT};
}}

QTableWidget:focus {{
    border-color: {ACCENT_PINK};
}}

QTableWidget::item:hover {{
    background: {INTERACTIVE_HOVER};
}}

QHeaderView::section {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {INTERACTIVE_HOVER}, stop:1 {HISTORY_SURFACE});
    color: {TEXT};
    border: 0;
    border-bottom: 1px solid {HISTORY_BORDER};
    padding: 7px 8px;
    font-weight: 700;
}}

QStatusBar {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {BACKGROUND}, stop:1 {SURFACE});
    color: {TEXT_MUTED};
    border-top: 1px solid {HISTORY_BORDER};
}}

QStatusBar::item {{
    background: transparent;
    border: 0;
}}

QStatusBar QLabel {{
    background: transparent;
    border: 0;
    color: {TEXT_MUTED};
    padding: 0 4px;
}}

QScrollBar:vertical {{
    background: {SURFACE_INPUT};
    width: 12px;
    margin: 2px;
}}

QScrollBar::handle:vertical {{
    background: {BORDER_STRONG};
    min-height: 30px;
    border-radius: 6px;
    border: 1px solid {SURFACE_INPUT};
}}

QScrollBar::handle:vertical:hover {{
    background: {ACCENT_PINK};
}}

QScrollBar::handle:vertical:pressed {{
    background: {ACCENT};
}}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {{
    background: transparent;
    height: 0;
}}

QScrollBar:horizontal {{
    background: {SURFACE_INPUT};
    height: 12px;
    margin: 2px;
}}

QScrollBar::handle:horizontal {{
    background: {BORDER_STRONG};
    min-width: 30px;
    border-radius: 6px;
    border: 1px solid {SURFACE_INPUT};
}}

QScrollBar::handle:horizontal:hover {{
    background: {ACCENT_PINK};
}}

QScrollBar::handle:horizontal:pressed {{
    background: {ACCENT};
}}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {{
    background: transparent;
    width: 0;
}}

QAbstractScrollArea::corner {{
    background: {SURFACE_INPUT};
}}

QToolTip {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    color: {TEXT};
    border: 1px solid {HISTORY_BORDER};
    padding: 6px 8px;
}}
"""

__all__ = ["COMBO_POPUP_STYLESHEET", "CONTROLS_STYLESHEET", "FILE_DIALOG_STYLESHEET"]
