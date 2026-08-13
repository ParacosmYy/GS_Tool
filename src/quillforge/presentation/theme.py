"""Central visual system for the QuillForge desktop shell."""

from __future__ import annotations

from PyQt6.QtGui import QColor, QPalette
from PyQt6.QtWidgets import QApplication

from ..domain.models import AppearanceSettings
from .font_style import qss_font_style, qss_font_weight
from .icons import application_icon
from .theme_tokens import (
    EditorColorTokens,
    ThemeColors,
    best_on_accent,
    contrast_ratio,
    editor_color_tokens,
    qss_foreground_tokens,
    readable_edge_foreground,
    readable_edge_foreground_for_surfaces,
    readable_foreground,
    theme_colors,
)

# Keep the historical private names as a compatibility façade for the QSS
# renderer and existing diagnostics while the pure resolution owner lives in
# theme_tokens.py.
_theme_colors = theme_colors
_best_on_accent = best_on_accent
_readable_foreground = readable_foreground
_contrast_ratio = contrast_ratio

__all__ = [
    "EditorColorTokens",
    "ThemeColors",
    "apply_editor_palette",
    "apply_theme",
    "editor_color_tokens",
    "preview_stylesheet",
    "theme_colors",
]


def apply_theme(
    application: QApplication,
    appearance: AppearanceSettings | None = None,
) -> None:
    """Install the selected shell theme, interface font, and authored icon."""
    selected = appearance or AppearanceSettings()
    colors = _theme_colors(selected.theme, selected.accent)
    application.setStyle("Fusion")
    application.setPalette(_application_palette(colors))
    application.setStyleSheet(
        _stylesheet(
            colors,
            selected.ui_font_family,
            selected.ui_font_size,
            selected.ui_font_style,
        )
    )
    icon = application_icon()
    if not icon.isNull():
        application.setWindowIcon(icon)


def apply_editor_palette(
    editor: object,
    theme_id: str = "sakura-pop",
    accent_id: str = "rose",
) -> None:
    """Apply editor canvas, caret, margin, and syntax colors through an adapter protocol."""
    colors = _theme_colors(theme_id, accent_id)
    editor_colors = editor_color_tokens(theme_id, accent_id)
    set_palette = getattr(editor, "setPalette", None)
    if not callable(set_palette):
        return
    palette = editor.palette()
    palette.setColor(QPalette.ColorRole.Base, QColor(editor_colors.canvas))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(colors.surface_1))
    palette.setColor(QPalette.ColorRole.Text, QColor(colors.text_primary))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(editor_colors.selection))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(editor_colors.selection_text))
    set_palette(palette)

    set_paper = getattr(editor, "setPaper", None)
    if callable(set_paper):
        set_paper(QColor(editor_colors.canvas))
    for method_name, color in (
        ("setSelectionBackgroundColor", editor_colors.selection),
        ("setSelectionForegroundColor", editor_colors.selection_text),
        ("setMarginsBackgroundColor", editor_colors.gutter),
        ("setMarginsForegroundColor", editor_colors.gutter_text),
        ("setCaretForegroundColor", editor_colors.caret),
        ("setCaretLineBackgroundColor", editor_colors.current_line),
        ("setMatchedBraceBackgroundColor", colors.surface_2),
        ("setMatchedBraceForegroundColor", editor_colors.caret),
    ):
        method = getattr(editor, method_name, None)
        if callable(method):
            method(QColor(color))
    set_margin_background = getattr(editor, "setMarginBackgroundColor", None)
    if callable(set_margin_background):
        set_margin_background(0, QColor(editor_colors.gutter))
    set_caret_width = getattr(editor, "setCaretWidth", None)
    if callable(set_caret_width):
        set_caret_width(2)
    lexer_getter = getattr(editor, "lexer", None)
    lexer = lexer_getter() if callable(lexer_getter) else None
    if lexer is not None:
        for method_name, color in (
            ("setDefaultColor", QColor(colors.text_primary)),
            ("setDefaultPaper", QColor(editor_colors.canvas)),
        ):
            method = getattr(lexer, method_name, None)
            if callable(method):
                method(color)
        set_color = getattr(lexer, "setColor", None)
        set_lexer_paper = getattr(lexer, "setPaper", None)
        if callable(set_color):
            for style_name, color in (
                ("Comment", editor_colors.comment),
                ("CommentBlock", editor_colors.comment),
                ("Number", editor_colors.number),
                ("DoubleQuotedString", editor_colors.string),
                ("SingleQuotedString", editor_colors.string),
                ("TripleSingleQuotedString", editor_colors.triple_string),
                ("TripleDoubleQuotedString", editor_colors.triple_string),
                ("Keyword", editor_colors.keyword),
                ("ClassName", editor_colors.type_name),
                ("FunctionMethodName", editor_colors.function),
                ("Operator", editor_colors.operator),
                ("UnclosedString", editor_colors.error),
                ("Decorator", editor_colors.decorator),
            ):
                style = getattr(lexer, style_name, getattr(type(lexer), style_name, None))
                if not isinstance(style, int):
                    continue
                set_color(QColor(color), style)
                if callable(set_lexer_paper):
                    set_lexer_paper(QColor(editor_colors.canvas), style)
    caret_line = getattr(editor, "setCaretLineVisible", None)
    if callable(caret_line):
        caret_line(True)


def preview_stylesheet(colors: ThemeColors) -> str:
    """Return token-bound styles for the pending-settings preview card."""
    return f"""
    QFrame#settingsPreview {{
        background: {colors.surface_3};
        border: 1px solid {colors.border_strong};
        border-left: 3px solid {colors.accent};
        border-radius: 10px;
    }}
    QLabel#settingsPreviewTitle {{
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QLabel#settingsPreviewAccent {{
        background: {colors.accent};
        border: 1px solid {colors.accent};
        border-radius: 6px;
        color: {colors.on_accent};
        font-weight: 700;
        padding: 4px 8px;
    }}
    QLabel#settingsPreviewSample {{
        background: {colors.surface_0};
        border: 1px solid {colors.border};
        border-radius: 7px;
        color: {colors.text_primary};
        font-weight: 600;
        padding: 10px;
    }}
    QLabel#settingsPreviewEditorSample {{
        background: {colors.surface_0};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_alt};
        border-radius: 7px;
        color: {colors.text_primary};
        padding: 8px 10px;
    }}
    QLabel#settingsPreviewEditorMeta {{
        color: {colors.text_secondary};
        font-size: 9pt;
    }}
    QLabel#settingsPreviewCanvas,
    QLabel#settingsPreviewPanel,
    QLabel#settingsPreviewSelection {{
        border-radius: 5px;
        font-size: 9pt;
        font-weight: 700;
        padding: 5px 7px;
    }}
    QLabel#settingsPreviewCanvas {{
        background: {colors.surface_0};
        border: 1px solid {colors.border};
        color: {colors.text_secondary};
    }}
    QLabel#settingsPreviewPanel {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        color: {colors.text_secondary};
    }}
    QLabel#settingsPreviewSelection {{
        background: {colors.selection};
        border: 1px solid {colors.accent};
        color: {colors.text_primary};
    }}
    QLabel#settingsPreviewMeta {{
        color: {colors.text_secondary};
        font-size: 9pt;
    }}
    """


def _application_palette(colors: ThemeColors) -> QPalette:
    palette = QPalette()
    foregrounds = qss_foreground_tokens(colors)
    palette.setColor(QPalette.ColorRole.Window, QColor(colors.surface_1))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(colors.text_primary))
    palette.setColor(QPalette.ColorRole.Base, QColor(colors.surface_0))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(colors.surface_2))
    palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(colors.surface_3))
    palette.setColor(QPalette.ColorRole.ToolTipText, QColor(colors.text_primary))
    palette.setColor(QPalette.ColorRole.Text, QColor(colors.text_primary))
    palette.setColor(QPalette.ColorRole.Button, QColor(colors.surface_3))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(colors.text_primary))
    palette.setColor(QPalette.ColorRole.BrightText, QColor(colors.on_accent))
    palette.setColor(QPalette.ColorRole.Link, QColor(foregrounds.accent_alt_text))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(colors.selection))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(colors.text_primary))
    palette.setColor(
        QPalette.ColorGroup.Disabled, QPalette.ColorRole.Text, QColor(colors.text_muted)
    )
    palette.setColor(
        QPalette.ColorGroup.Disabled,
        QPalette.ColorRole.ButtonText,
        QColor(colors.text_muted),
    )
    return palette


def _stylesheet(
    colors: ThemeColors,
    ui_font_family: str,
    ui_font_size: int,
    ui_font_style: object = "regular",
) -> str:
    """Return the shared QSS with all visual tokens bound to one theme."""
    foregrounds = qss_foreground_tokens(colors)
    warning_foreground = foregrounds.warning
    success_foreground = foregrounds.success
    working_foreground = foregrounds.working
    error_foreground = foregrounds.error
    accent_alt_text_foreground = foregrounds.accent_alt_text
    accent_alt_foreground = foregrounds.accent_alt_fill
    typography_interface_edge = readable_edge_foreground(
        colors.accent_alt,
        colors.pressed,
        colors.accent_gold,
    )
    typography_editor_edge = readable_edge_foreground(
        colors.accent_pink,
        colors.pressed,
        colors.accent_gold,
    )
    behavior_edge_surfaces = (
        colors.surface_0,
        colors.surface_hover,
        colors.pressed,
        colors.surface_2,
    )
    behavior_interface_edge = readable_edge_foreground_for_surfaces(
        colors.accent_alt,
        behavior_edge_surfaces,
        colors.accent_gold,
    )
    behavior_editor_edge = readable_edge_foreground_for_surfaces(
        colors.accent_pink,
        behavior_edge_surfaces,
        colors.accent_gold,
    )
    tooltip_edge = readable_edge_foreground_for_surfaces(
        colors.accent_alt,
        (colors.surface_3,),
        colors.text_primary,
    )
    separator_edge = readable_edge_foreground_for_surfaces(
        colors.border,
        (colors.surface_0, colors.surface_1, colors.surface_2),
        colors.text_primary,
    )
    scrollbar_surfaces = (colors.surface_1, colors.surface_2)
    scrollbar_handle = readable_edge_foreground_for_surfaces(
        colors.border_strong,
        scrollbar_surfaces,
        colors.text_primary,
    )
    scrollbar_hover = readable_edge_foreground_for_surfaces(
        colors.accent_alt,
        scrollbar_surfaces,
        colors.text_primary,
    )
    scrollbar_pressed = readable_edge_foreground_for_surfaces(
        colors.accent,
        scrollbar_surfaces,
        colors.text_primary,
    )
    spinbox_stepper_surface = colors.surface_2
    spinbox_stepper_hover = colors.surface_hover
    spinbox_stepper_pressed = colors.pressed
    spinbox_stepper_foreground = readable_foreground(
        colors.text_primary,
        spinbox_stepper_surface,
        colors.text_primary,
    )
    spinbox_stepper_hover_foreground = readable_foreground(
        colors.text_primary,
        spinbox_stepper_hover,
        colors.text_primary,
    )
    spinbox_stepper_pressed_foreground = readable_foreground(
        colors.text_primary,
        spinbox_stepper_pressed,
        colors.text_primary,
    )
    checkbox_checked_edge = readable_edge_foreground_for_surfaces(
        colors.accent,
        (colors.pressed, colors.surface_hover),
        colors.text_primary,
    )
    checkbox_checked_hover_edge = readable_edge_foreground_for_surfaces(
        colors.accent_pink,
        (colors.surface_hover,),
        colors.text_primary,
    )
    checkbox_checked_focus_edge = readable_edge_foreground_for_surfaces(
        colors.accent_alt,
        (colors.pressed,),
        colors.text_primary,
    )
    checkbox_disabled_edge = readable_edge_foreground_for_surfaces(
        colors.border_strong,
        (colors.surface_2,),
        colors.text_muted,
    )
    tab_close_hover_edge = readable_edge_foreground_for_surfaces(
        colors.danger,
        (colors.error_bg,),
        colors.text_primary,
    )
    tab_close_focus_edge = readable_edge_foreground_for_surfaces(
        colors.accent_alt,
        (colors.surface_hover,),
        colors.text_primary,
    )
    tab_close_pressed_edge = readable_edge_foreground_for_surfaces(
        colors.danger,
        (colors.pressed,),
        colors.text_primary,
    )
    locale_choice_edge = readable_edge_foreground_for_surfaces(
        colors.accent_alt,
        (colors.surface_2, colors.surface_hover, colors.pressed),
        colors.accent_gold,
    )
    return f"""
    * {{
        font-family: "{ui_font_family}";
        font-size: {ui_font_size}pt;
        font-style: {qss_font_style(ui_font_style)};
        font-weight: {qss_font_weight(ui_font_style)};
        color: {colors.text_primary};
    }}
    QAbstractItemView {{
        outline: 0;
        border: 1px solid {colors.border};
        border-radius: 10px;
        alternate-background-color: {colors.surface_1};
        selection-background-color: {colors.selection};
        selection-color: {colors.text_primary};
    }}
    QAbstractItemView:focus {{
        border-color: {colors.accent_alt};
    }}
    QAbstractItemView::item:alternate {{
        background: {colors.surface_1};
    }}
    QAbstractItemView::item:selected {{
        background: {colors.selection};
        color: {colors.text_primary};
    }}
    QAbstractItemView::item:selected:disabled {{
        background: {colors.surface_2};
        color: {colors.text_muted};
        border-color: {colors.border};
    }}
    QAbstractItemView::item:disabled {{
        color: {colors.text_muted};
    }}
    QToolTip {{
        background: {colors.surface_3};
        border: 1px solid {colors.border_strong};
        border-left: 3px solid {tooltip_edge};
        border-radius: 6px;
        color: {colors.text_primary};
        font-size: {max(8, ui_font_size - 1)}pt;
        font-weight: 600;
        padding: 6px 9px;
    }}
    QMainWindow#mainWindow {{
        background: {colors.surface_0};
    }}
    QMainWindow::separator,
    QDockWidget::separator {{
        background: {separator_edge};
        height: 1px;
        width: 1px;
    }}
    QMainWindow::separator:hover,
    QDockWidget::separator:hover {{
        background: {colors.accent_alt};
    }}
    QScrollBar:vertical,
    QScrollBar:horizontal {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        margin: 0;
    }}
    QScrollBar:vertical {{
        width: 12px;
    }}
    QScrollBar:horizontal {{
        height: 12px;
    }}
    QScrollBar::handle:vertical,
    QScrollBar::handle:horizontal {{
        background: {scrollbar_handle};
        border: 1px solid {scrollbar_handle};
        border-radius: 5px;
        min-height: 28px;
        min-width: 28px;
    }}
    QScrollBar::handle:vertical:hover,
    QScrollBar::handle:horizontal:hover {{
        background: {scrollbar_hover};
        border-color: {scrollbar_hover};
    }}
    QScrollBar::handle:vertical:pressed,
    QScrollBar::handle:horizontal:pressed {{
        background: {scrollbar_pressed};
        border-color: {scrollbar_pressed};
    }}
    QScrollBar::add-line,
    QScrollBar::sub-line {{
        background: transparent;
        border: 0;
        height: 0;
        width: 0;
    }}
    QScrollBar::add-page,
    QScrollBar::sub-page {{
        background: {colors.surface_1};
    }}
    QWidget#editorShell {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        border-top: 2px solid {colors.accent_pink};
        border-radius: 14px;
    }}
    QsciScintilla#editor {{
        background: {colors.surface_0};
        border: 1px solid {colors.border};
        border-radius: 12px;
        selection-background-color: {colors.selection};
        selection-color: {foregrounds.selection};
    }}
    QsciScintilla#editor:focus {{
        border-color: {colors.accent_alt};
    }}
    QsciScintilla#editor:disabled {{
        background: {colors.surface_1};
        border-color: {colors.border_strong};
        selection-background-color: {colors.pressed};
    }}
    QToolBar#commandBar {{
        background: {colors.surface_2};
        border: 1px solid {colors.border_strong};
        border-top: 2px solid {colors.accent_pink};
        border-radius: 10px;
        margin: 8px 12px 6px 12px;
        padding: 5px 7px 6px 7px;
        spacing: 6px;
    }}
    QToolBar#commandBar::separator {{
        background: {colors.border_strong};
        margin: 6px 5px;
        width: 1px;
    }}
    QLabel#toolbarBrand {{
        background: {colors.surface_3};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_pink};
        border-radius: 10px;
        color: {colors.text_primary};
        font-size: {max(8, ui_font_size - 1)}pt;
        font-weight: 800;
        margin-right: 3px;
        padding: 5px 12px 5px 10px;
    }}
    QToolBar#commandBar QToolButton {{
        background: transparent;
        border: 1px solid transparent;
        border-radius: 9px;
        color: {colors.text_secondary};
        font-weight: 600;
        min-height: 32px;
        padding: 6px 10px;
    }}
    QToolBar#commandBar QToolButton:hover {{
        background: {colors.surface_3};
        border-color: {colors.border_strong};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent};
    }}
    QToolBar#commandBar QToolButton:checked,
    QToolButton:checked {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QToolBar#commandBar QToolButton:checked:hover,
    QToolButton:checked:hover {{
        background: {colors.surface_hover};
        border-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton:disabled,
    QToolButton:disabled {{
        background: transparent;
        border-color: transparent;
        color: {colors.text_muted};
    }}
    QToolBar#commandBar QToolButton:focus,
    QPushButton:focus,
    QToolButton:focus,
    QLineEdit:focus,
    QSpinBox:focus,
    QComboBox:focus {{
        border-color: {colors.accent_alt};
    }}
    QToolBar#commandBar QToolButton:focus,
    QToolButton:focus {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton[commandRole="primary"] {{
        background: {colors.accent};
        border-color: {colors.accent};
        color: {colors.on_accent};
        font-weight: 700;
    }}
    QToolBar#commandBar QToolButton[commandRole="primary"]:hover,
    QToolBar#commandBar QToolButton[commandRole="primary"]:focus {{
        background: {colors.accent_alt};
        border-color: {colors.accent_alt};
        color: {accent_alt_foreground};
    }}
    QToolBar#commandBar QToolButton[commandRole="primary"]:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton[commandRole="primary"]:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QToolBar#commandBar QToolButton[commandRole="quiet"] {{
        color: {colors.text_muted};
        font-weight: 500;
        padding-left: 8px;
        padding-right: 8px;
    }}
    QToolBar#commandBar QToolButton[commandRole="quiet"]:hover,
    QToolBar#commandBar QToolButton[commandRole="quiet"]:focus {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton[commandRole="quiet"]:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton[commandRole="context"] {{
        background: {colors.surface_3};
        border-left-color: {colors.accent_alt};
        color: {colors.text_secondary};
        font-weight: 700;
    }}
    QToolBar#commandBar QToolButton[commandRole="context"]:hover,
    QToolBar#commandBar QToolButton[commandRole="context"]:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton[commandRole="context"]:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QToolBar#commandBar QToolButton[commandRole="context"]:disabled,
    QToolBar#commandBar QToolButton[commandRole="quiet"]:disabled {{
        background: transparent;
        border-color: transparent;
        color: {colors.text_muted};
    }}
    QCheckBox:focus {{
        background: {colors.surface_hover};
        border-radius: 4px;
        color: {colors.text_primary};
    }}
    QCheckBox:focus::indicator {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
    }}
    QLabel#toolbarContext {{
        background: {colors.surface_3};
        border: 0;
        border-left: 2px solid {colors.accent_alt};
        border-radius: 7px;
        color: {colors.text_secondary};
        font-size: {max(8, ui_font_size - 2)}pt;
        font-weight: 700;
        margin-right: 2px;
        padding: 5px 11px 5px 10px;
    }}
    QMenuBar {{
        background: {colors.surface_1};
        border-bottom: 1px solid {colors.border};
        padding: 3px 10px;
        spacing: 2px;
    }}
    QMenuBar::item {{
        background: transparent;
        border-radius: 6px;
        padding: 5px 9px;
    }}
    QMenuBar::item:selected {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
    }}
    QMenuBar::item:pressed {{
        background: {colors.pressed};
        color: {colors.text_primary};
    }}
    QMenu {{
        background: {colors.surface_3};
        border: 1px solid {colors.border_strong};
        border-radius: 7px;
        padding: 4px;
    }}
    QMenu::item {{
        border-radius: 4px;
        padding: 7px 28px 7px 10px;
    }}
    QMenu::item:selected {{
        background: {colors.accent};
        color: {colors.on_accent};
    }}
    QMenu::item:checked {{
        background: {colors.selection};
        border-left: 3px solid {colors.accent_alt};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QMenu::item:selected:checked {{
        background: {colors.accent};
        border-left-color: {colors.accent_alt};
        color: {colors.on_accent};
    }}
    QMenu::item:disabled {{
        color: {colors.text_muted};
    }}
    QMenu::item:selected:disabled,
    QMenu::item:checked:disabled {{
        background: {colors.surface_2};
        border-left: 3px solid {colors.border_strong};
        color: {colors.text_muted};
        font-weight: 600;
    }}
    QMenu::indicator {{
        background: transparent;
        border: 1px solid {colors.border_strong};
        border-radius: 4px;
        height: 14px;
        margin-left: 3px;
        width: 14px;
    }}
    QMenu::indicator:hover {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
    }}
    QMenu::indicator:checked {{
        background: {colors.accent_alt};
        border-color: {colors.accent_alt};
    }}
    QMenu::indicator:checked:hover {{
        background: {colors.accent};
        border-color: {colors.accent};
    }}
    QMenu::indicator:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
    }}
    QStatusBar {{
        background: {colors.surface_1};
        border-top: 1px solid {colors.border};
        color: {colors.text_secondary};
        padding: 5px 12px;
    }}
    QStatusBar::item {{
        border: 0;
    }}
    QLabel#statusMessage {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.border_strong};
        border-radius: 10px;
        color: {colors.text_primary};
        font-size: {max(8, ui_font_size - 1)}pt;
        font-weight: 600;
        margin: 2px 6px;
        padding: 4px 11px;
    }}
    QLabel#statusMessage[state="info"] {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left-color: {colors.border_strong};
        color: {colors.text_primary};
    }}
    QLabel#statusMessage[state="success"] {{
        background: {colors.success_bg};
        border-color: {colors.success};
        border-left-width: 3px;
        color: {success_foreground};
    }}
    QLabel#statusMessage[state="warning"] {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        border-left-width: 3px;
        color: {warning_foreground};
    }}
    QLabel#statusMessage[state="error"] {{
        background: {colors.error_bg};
        border-color: {colors.danger};
        border-left-width: 3px;
        color: {error_foreground};
    }}
    QWidget#statusRail {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 2px solid {colors.accent_alt};
        border-radius: 9px;
        margin: 2px 6px;
        padding: 2px 4px;
    }}
    QLabel#statusContext {{
        border-right: 1px solid {colors.border};
        color: {colors.text_muted};
        font-size: {max(8, ui_font_size - 2)}pt;
        font-weight: 700;
        padding: 1px 9px 1px 2px;
    }}
    QLabel#statusPhase {{
        border-radius: 6px;
        font-size: {max(8, ui_font_size - 2)}pt;
        font-weight: 700;
        padding: 4px 9px;
    }}
    QLabel#statusPhase[state="ready"] {{
        background: {colors.success_bg};
        border: 1px solid {colors.success};
        color: {success_foreground};
    }}
    QLabel#statusPhase[state="working"] {{
        background: {colors.pressed};
        border: 1px solid {colors.accent_alt};
        color: {working_foreground};
    }}
    QLabel#statusPhase[state="attention"] {{
        background: {colors.warning_bg};
        border: 1px solid {colors.accent_gold};
        color: {warning_foreground};
    }}
    QLabel#statusPhase[state="error"] {{
        background: {colors.error_bg};
        border: 1px solid {colors.danger};
        color: {error_foreground};
    }}
    QTabWidget#documentTabs {{
        background: {colors.surface_1};
        border: 0;
    }}
    QTabWidget#documentTabs::pane {{
        background: {colors.surface_0};
        border: 0;
        border-radius: 0;
        top: 0;
    }}
    QTabBar {{
        background: {colors.surface_0};
        border-bottom: 1px solid {colors.border};
    }}
    QTabBar::tab {{
        background: transparent;
        border: 1px solid transparent;
        border-radius: 5px;
        color: {colors.text_muted};
        min-width: 88px;
        margin: 3px 2px 0;
        padding: 7px 11px;
    }}
    QTabBar::tab:hover {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_primary};
    }}
    QTabBar::tab:selected {{
        background: {colors.surface_2};
        color: {colors.text_primary};
        border-color: transparent;
        border-left: 2px solid {colors.accent_alt};
        border-bottom: 2px solid {colors.accent};
        font-weight: 700;
    }}
    QTabBar::tab:selected:hover {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
        border-color: transparent;
        border-left-color: {colors.accent_alt};
        border-bottom-color: {colors.accent};
    }}
    QTabBar::tab:focus {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
        border-color: {colors.accent_alt};
        border-bottom: 3px solid {colors.accent_alt};
    }}
    QTabBar::tab:disabled {{
        background: transparent;
        color: {colors.text_muted};
        border-color: transparent;
    }}
    QTabBar#documentTabBar {{
        background: {colors.surface_1};
        border: 0;
        border-bottom: 1px solid {colors.border};
        border-radius: 0;
        margin: 2px 8px 0;
        padding: 2px 2px 0;
    }}
    QTabBar#documentTabBar::tab {{
        background: transparent;
        border: 1px solid transparent;
        border-bottom: 2px solid transparent;
        border-radius: 9px;
        color: {colors.text_secondary};
        margin: 2px 3px 1px;
        min-width: 96px;
        padding: 8px 12px 8px;
    }}
    QTabBar#documentTabBar::tab:hover {{
        background: {colors.surface_hover};
        border-color: {colors.border};
        color: {colors.text_primary};
    }}
    QTabBar#documentTabBar::tab:selected {{
        background: {colors.surface_3};
        border-color: transparent;
        border-left-color: {colors.accent_alt};
        border-bottom-color: {colors.accent};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QTabBar#documentTabBar::tab:selected:hover {{
        background: {colors.surface_hover};
        border-color: transparent;
        border-left-color: {colors.accent_alt};
        border-bottom-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QTabBar#documentTabBar::tab:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
        border-bottom-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QTabBar#documentTabBar::tab:disabled {{
        background: transparent;
        border-color: transparent;
        color: {colors.text_muted};
    }}
    QTabBar#documentTabBar::close-button {{
        background: transparent;
        border: 1px solid transparent;
        border-radius: 6px;
        margin: 2px 2px 2px 5px;
        min-height: 20px;
        min-width: 20px;
        padding: 2px;
    }}
    QTabBar#documentTabBar::close-button:hover {{
        background: {colors.error_bg};
        border-color: {tab_close_hover_edge};
    }}
    QTabBar#documentTabBar::close-button:focus {{
        background: {colors.surface_hover};
        border-color: {tab_close_focus_edge};
    }}
    QTabBar#documentTabBar::close-button:pressed {{
        background: {colors.pressed};
        border-color: {tab_close_pressed_edge};
    }}
    QTabBar#documentTabBar::close-button:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
    }}
    QDockWidget#WorkspaceDock {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        color: {colors.text_primary};
    }}
    QDockWidget#WorkspaceDock::title {{
        background: {colors.surface_2};
        border-bottom: 1px solid {colors.border};
        border-left: 2px solid {colors.accent_alt};
        border-top-left-radius: 10px;
        border-top-right-radius: 10px;
        color: {colors.text_secondary};
        font-size: {max(9, ui_font_size - 1)}pt;
        font-weight: 700;
        padding: 9px 10px 9px 12px;
    }}
    QDockWidget#WorkspaceDock::close-button,
    QDockWidget#WorkspaceDock::float-button {{
        background: transparent;
        border: 1px solid transparent;
        border-radius: 6px;
        height: 20px;
        margin: 2px 3px 2px 0;
        padding: 2px;
        width: 20px;
    }}
    QDockWidget#WorkspaceDock::close-button {{
        subcontrol-origin: padding;
        subcontrol-position: top right;
    }}
    QDockWidget#WorkspaceDock::float-button {{
        subcontrol-origin: padding;
        subcontrol-position: top right;
        right: 24px;
    }}
    QDockWidget#WorkspaceDock::close-button:hover,
    QDockWidget#WorkspaceDock::float-button:hover {{
        background: {colors.surface_hover};
        border-color: {colors.border_strong};
    }}
    QDockWidget#WorkspaceDock::close-button:pressed,
    QDockWidget#WorkspaceDock::float-button:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
    }}
    QDockWidget#WorkspaceDock::close-button:disabled,
    QDockWidget#WorkspaceDock::float-button:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
    }}
    QWidget#workspacePanel {{
        background: {colors.surface_0};
    }}
    QLabel#workspaceEyebrow {{
        color: {accent_alt_text_foreground};
        font-size: {max(8, ui_font_size - 2)}pt;
        font-weight: 700;
    }}
    QLabel#workspaceEmpty {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        border-top: 2px solid {colors.accent_pink};
        border-radius: 10px;
        color: {colors.text_secondary};
        font-weight: 600;
        padding: 18px 14px;
    }}
    QLabel#workspacePath, QLabel#workspaceSearchRoot {{
        background: {colors.surface_3};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_alt};
        border-radius: 6px;
        color: {colors.text_secondary};
        font-size: {max(9, ui_font_size - 1)}pt;
        font-weight: 600;
        padding: 7px 9px;
    }}
    QLabel#workspaceStatus, QLabel#workspaceSearchStatus {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.border_strong};
        border-radius: 6px;
        color: {colors.text_secondary};
        padding: 5px 8px;
    }}
    QLabel#workspaceStatus[state="info"], QLabel#workspaceSearchStatus[state="info"] {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left-color: {colors.border_strong};
        color: {colors.text_secondary};
    }}
    QLabel#workspaceStatus[state="working"], QLabel#workspaceSearchStatus[state="working"] {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        border-left-color: {colors.accent_alt};
        color: {working_foreground};
        font-weight: 600;
    }}
    QLabel#workspaceStatus[state="success"], QLabel#workspaceSearchStatus[state="success"] {{
        background: {colors.success_bg};
        border-color: {colors.success};
        border-left-color: {colors.success};
        color: {success_foreground};
        font-weight: 600;
    }}
    QLabel#workspaceStatus[state="warning"], QLabel#workspaceSearchStatus[state="warning"] {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        border-left-color: {colors.accent_gold};
        color: {warning_foreground};
        font-weight: 600;
    }}
    QLabel#workspaceStatus[state="error"], QLabel#workspaceSearchStatus[state="error"] {{
        background: {colors.error_bg};
        border-color: {colors.danger};
        border-left-color: {colors.danger};
        color: {error_foreground};
        font-weight: 600;
    }}
    QTreeWidget#workspaceTree, QListWidget {{
        background: {colors.surface_0};
        border: 1px solid {colors.border};
        border-radius: 8px;
        padding: 5px;
    }}
    QTreeWidget#workspaceTree:focus, QListWidget:focus {{
        border-color: {colors.accent_alt};
    }}
    QTreeWidget#workspaceTree:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QTreeWidget#workspaceTree::item, QListWidget::item {{
        border-radius: 6px;
        padding: 6px 8px;
    }}
    QTreeWidget#workspaceTree::item:alternate {{
        background: {colors.surface_1};
    }}
    QTreeWidget#workspaceTree::item:hover:alternate:!selected {{
        background: {colors.surface_hover};
    }}
    QTreeWidget#workspaceTree::item:hover, QListWidget::item:hover {{
        background: {colors.surface_3};
    }}
    QTreeWidget#workspaceTree::item:selected, QListWidget::item:selected {{
        background: {colors.selection};
        color: {colors.text_primary};
        border: 1px solid {colors.accent};
        border-left: 4px solid {colors.accent_alt};
        font-weight: 700;
    }}
    QTreeWidget#workspaceTree::item:selected:!active, QListWidget::item:selected:!active {{
        background: {colors.pressed};
        color: {colors.text_primary};
        border-color: {colors.border_strong};
        border-left-color: {colors.accent_alt};
    }}
    QTreeWidget#workspaceTree::item:selected:disabled,
    QListWidget::item:selected:disabled {{
        background: {colors.surface_2};
        color: {colors.text_muted};
        border-color: {colors.border};
        border-left-color: {colors.border_strong};
        font-weight: 600;
    }}
    QTreeWidget#workspaceTree::item:disabled {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.border_strong};
        color: {colors.text_muted};
        font-weight: 600;
    }}
    QPushButton {{
        background: {colors.surface_3};
        border: 1px solid {colors.border_strong};
        border-radius: 6px;
        color: {colors.text_primary};
        min-height: 30px;
        padding: 5px 11px;
    }}
    QPushButton:hover {{
        background: {colors.surface_hover};
        border-color: {colors.accent};
    }}
    QPushButton:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
    }}
    QPushButton:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QPushButton:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QDialogButtonBox#dialogActions {{
        background: transparent;
        border-top: 1px solid {colors.border};
        padding-top: 8px;
    }}
    QWidget#dialogActionRail {{
        background: transparent;
        border-top: 1px solid {colors.border};
        margin-top: 2px;
    }}
    QWidget#dialogActionRail QPushButton {{
        min-width: 88px;
    }}
    QPushButton#quietAction {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_secondary};
        min-height: 30px;
    }}
    QPushButton#quietAction:hover,
    QPushButton#quietAction:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QPushButton#quietAction:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QPushButton#quietAction:disabled {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QWidget#findBar QPushButton#findPrevious,
    QWidget#findBar QPushButton#findNext {{
        background: {colors.surface_3};
        border-color: {colors.border};
        color: {colors.text_secondary};
        min-width: 34px;
        padding: 5px 8px;
    }}
    QWidget#findBar QPushButton#findPrevious:hover,
    QWidget#findBar QPushButton#findNext:hover,
    QWidget#findBar QPushButton#findPrevious:focus,
    QWidget#findBar QPushButton#findNext:focus {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QWidget#findBar QPushButton#findPrevious:disabled,
    QWidget#findBar QPushButton#findNext:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QWidget#findBar QPushButton#findCancel {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_secondary};
        min-width: 64px;
    }}
    QWidget#findBar QPushButton#findCancel:hover,
    QWidget#findBar QPushButton#findCancel:focus {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        color: {warning_foreground};
    }}
    QWidget#findBar QPushButton#findClose {{
        background: transparent;
        border-color: transparent;
        color: {colors.text_muted};
        min-height: 30px;
        min-width: 30px;
        padding-left: 5px;
        padding-right: 5px;
    }}
    QWidget#findBar QPushButton#findClose:hover,
    QWidget#findBar QPushButton#findClose:focus {{
        background: {colors.error_bg};
        border-color: {colors.danger};
        color: {colors.text_primary};
    }}
    QWidget#findBar QPushButton#findClose:pressed {{
        background: {colors.pressed};
        border-color: {colors.danger};
        color: {colors.text_primary};
    }}
    QWidget#findBar QPushButton#findClose:disabled {{
        background: transparent;
        border-color: transparent;
        color: {colors.text_muted};
    }}
    QPushButton:checked {{
        background: {colors.selection};
        border-color: {colors.accent};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QPushButton#workspaceBack {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_secondary};
        font-weight: 600;
        min-height: 28px;
        padding: 5px 10px;
    }}
    QPushButton#workspaceBack:hover,
    QPushButton#workspaceBack:focus:!disabled {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QPushButton#workspaceBack:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QPushButton#workspaceBack:disabled {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QWidget#workspacePanel QPushButton#workspaceFileAction {{
        background: {colors.surface_3};
        border-color: {colors.border_strong};
        border-left: 2px solid {colors.accent_alt};
        color: {colors.text_secondary};
        font-weight: 600;
        min-height: 28px;
        padding: 5px 9px;
    }}
    QWidget#workspacePanel QPushButton#workspaceFileAction:hover,
    QWidget#workspacePanel QPushButton#workspaceFileAction:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QWidget#workspacePanel QPushButton#workspaceFileAction:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QWidget#workspacePanel QPushButton#workspaceFileAction:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QPushButton#workspaceCancel {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_muted};
        min-height: 28px;
        padding: 5px 10px;
    }}
    QPushButton#workspaceCancel:hover,
    QPushButton#workspaceCancel:focus:!disabled {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        color: {warning_foreground};
    }}
    QPushButton#workspaceCancel:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_gold};
        color: {colors.text_primary};
    }}
    QPushButton#workspaceCancel:disabled {{
        background: transparent;
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QWidget#workspacePanel QPushButton[workspaceRole="folderPicker"] {{
        border-left: 3px solid {colors.accent_pink};
    }}
    QWidget#workspacePanel QPushButton[workspaceRole="documentPicker"] {{
        border-left: 3px solid {colors.accent_alt};
    }}
    QPushButton#primaryAction {{
        background: {colors.accent};
        border-color: {colors.accent};
        color: {colors.on_accent};
        font-weight: 700;
    }}
    QPushButton#primaryAction:hover {{
        background: {colors.accent_pink};
        border-color: {colors.accent_pink};
        color: {colors.on_accent_pink};
    }}
    QPushButton#primaryAction:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QPushButton#primaryAction:focus:!disabled {{
        border-color: {colors.accent_alt};
    }}
    QPushButton#primaryAction:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QPushButton#warningAction {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        color: {warning_foreground};
        font-weight: 700;
    }}
    QPushButton#warningAction:hover {{
        background: {colors.accent_gold};
        border-color: {colors.accent_gold};
        color: {colors.on_accent_gold};
    }}
    QPushButton#warningAction:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_gold};
        color: {colors.text_primary};
    }}
    QPushButton#warningAction:focus:!disabled {{
        border-color: {colors.accent_alt};
    }}
    QPushButton#warningAction:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QLineEdit, QSpinBox, QComboBox {{
        background: {colors.surface_0};
        border: 1px solid {colors.border_strong};
        border-radius: 7px;
        color: {colors.text_primary};
        min-height: 30px;
        padding: 5px 9px;
        selection-background-color: {colors.selection};
        selection-color: {colors.text_primary};
    }}
    QLineEdit:hover, QSpinBox:hover, QComboBox:hover {{
        background: {colors.surface_1};
        border-color: {colors.border_strong};
    }}
    QLineEdit:focus, QSpinBox:focus, QComboBox:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
    }}
    QLineEdit:read-only, QAbstractSpinBox:read-only {{
        background: {colors.surface_1};
        border-color: {colors.border};
        color: {colors.text_secondary};
    }}
    QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QWidget#findBar QLineEdit#findQuery {{
        border-color: {colors.accent_alt};
        font-weight: 600;
    }}
    QWidget#findBar QLineEdit#findQuery:focus {{
        background: {colors.surface_1};
        border-color: {colors.accent_alt};
    }}
    QWidget#findBar QLineEdit#replaceQuery {{
        border-color: {colors.accent_pink};
    }}
    QWidget#findBar QLineEdit#replaceQuery:focus {{
        background: {colors.surface_1};
        border-color: {colors.accent_pink};
    }}
    QComboBox:on {{
        background: {colors.pressed};
        border-color: {colors.accent};
    }}
    QComboBox QAbstractItemView {{
        background: {colors.surface_3};
        border: 1px solid {colors.border_strong};
        selection-background-color: {colors.selection};
        selection-color: {colors.text_primary};
    }}
    QComboBox QAbstractItemView::item {{
        border-left: 3px solid transparent;
        border-radius: 5px;
        color: {colors.text_secondary};
        padding: 6px 8px;
    }}
    QComboBox QAbstractItemView::item:hover {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
    }}
    QComboBox QAbstractItemView::item:selected {{
        background: {colors.selection};
        border-left-color: {colors.accent_alt};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QComboBox QAbstractItemView::item:selected:disabled {{
        background: {colors.surface_2};
        color: {colors.text_muted};
    }}
    QComboBox::drop-down {{
        background: {colors.surface_2};
        border-left: 1px solid {colors.border};
        border-top-right-radius: 6px;
        border-bottom-right-radius: 6px;
        subcontrol-origin: border;
        subcontrol-position: top right;
        width: 26px;
    }}
    QComboBox::drop-down:hover {{
        background: {colors.surface_hover};
        border-left-color: {colors.accent_alt};
    }}
    QComboBox::drop-down:pressed {{
        background: {colors.pressed};
        border-left-color: {colors.accent};
    }}
    QComboBox::drop-down:on {{
        background: {colors.pressed};
        border-left-color: {colors.accent};
    }}
    QComboBox::drop-down:disabled {{
        background: {colors.surface_2};
        border-left-color: {colors.border};
    }}
    QComboBox::down-arrow {{
        border-left: 4px solid transparent;
        border-right: 4px solid transparent;
        border-top: 5px solid {colors.text_secondary};
        height: 0px;
        width: 0px;
    }}
    QComboBox::down-arrow:on {{
        border-top-color: {colors.text_primary};
    }}
    QComboBox::down-arrow:disabled {{
        border-top-color: {colors.text_muted};
    }}
    QAbstractSpinBox::up-button,
    QAbstractSpinBox::down-button {{
        background: {colors.surface_2};
        border-left: 1px solid {colors.border};
        width: 20px;
    }}
    QAbstractSpinBox::up-button {{
        border-top-right-radius: 6px;
        subcontrol-origin: border;
        subcontrol-position: top right;
    }}
    QAbstractSpinBox::down-button {{
        border-top: 1px solid {colors.border};
        border-bottom-right-radius: 6px;
        subcontrol-origin: border;
        subcontrol-position: bottom right;
    }}
    QAbstractSpinBox::up-button:hover,
    QAbstractSpinBox::down-button:hover {{
        background: {colors.surface_hover};
        border-left-color: {colors.accent_alt};
    }}
    QAbstractSpinBox::up-button:pressed,
    QAbstractSpinBox::down-button:pressed {{
        background: {colors.pressed};
        border-left-color: {colors.accent};
    }}
    QAbstractSpinBox::up-button:disabled,
    QAbstractSpinBox::down-button:disabled {{
        background: {colors.surface_2};
        border-left-color: {colors.border};
    }}
    QAbstractSpinBox::up-arrow {{
        border-bottom: 5px solid {colors.text_secondary};
        border-left: 4px solid transparent;
        border-right: 4px solid transparent;
        height: 0px;
        width: 0px;
    }}
    QAbstractSpinBox::down-arrow {{
        border-left: 4px solid transparent;
        border-right: 4px solid transparent;
        border-top: 5px solid {colors.text_secondary};
        height: 0px;
        width: 0px;
    }}
    QAbstractSpinBox::up-arrow:disabled {{
        border-bottom-color: {colors.text_muted};
    }}
    QAbstractSpinBox::down-arrow:disabled {{
        border-top-color: {colors.text_muted};
    }}
    QCheckBox {{
        spacing: 6px;
        padding: 2px 0;
    }}
    QCheckBox:hover {{
        color: {accent_alt_text_foreground};
    }}
    QCheckBox::indicator {{
        background: {colors.surface_0};
        border: 1px solid {colors.border_strong};
        border-radius: 4px;
        height: 15px;
        width: 15px;
    }}
    QCheckBox::indicator:checked {{
        background: {colors.pressed};
        border-color: {checkbox_checked_edge};
    }}
    QCheckBox::indicator:hover {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
    }}
    QCheckBox::indicator:checked:hover {{
        background: {colors.surface_hover};
        border-color: {checkbox_checked_hover_edge};
    }}
    QCheckBox:focus::indicator:checked {{
        border-color: {checkbox_checked_focus_edge};
    }}
    QCheckBox::indicator:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
    }}
    QCheckBox::indicator:checked:disabled {{
        background: {colors.surface_2};
        border-color: {checkbox_disabled_edge};
    }}
    QCheckBox:disabled {{
        color: {colors.text_muted};
    }}
    QGroupBox {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-radius: 8px;
        margin-top: 10px;
        padding-top: 12px;
    }}
    QGroupBox::title {{
        color: {colors.text_primary};
        font-weight: 700;
        left: 12px;
        padding: 0 6px;
        subcontrol-origin: margin;
    }}
    QDialog#settingsDialog QGroupBox#settingsAppearanceGroup {{
        background: {colors.surface_2};
        border: 1px solid {colors.border_strong};
        border-left: 3px solid {colors.accent_alt};
        border-radius: 10px;
    }}
    QDialog#settingsDialog QLabel[settingsRole="fieldLabel"] {{
        color: {colors.text_secondary};
        font-weight: 600;
        padding-right: 6px;
    }}
    QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"] {{
        background: {colors.surface_0};
        border: 1px solid {colors.border};
        border-radius: 6px;
        color: {colors.text_secondary};
        padding: 5px 8px;
        spacing: 8px;
    }}
    QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:hover,
    QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:focus {{
        background: {colors.surface_hover};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:checked {{
        background: {colors.pressed};
        border-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:checked:focus {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="interface"] {{
        border-left: 2px solid {behavior_interface_edge};
    }}
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="interface"]:hover,
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="interface"]:focus,
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="interface"]:checked {{
        border-left-color: {behavior_interface_edge};
    }}
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="editor"] {{
        border-left: 2px solid {behavior_editor_edge};
    }}
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="editor"]:hover,
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="editor"]:focus,
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="editor"]:checked {{
        border-left-color: {behavior_editor_edge};
    }}
    QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="interface"]:disabled {{
        border-left-color: {behavior_interface_edge};
    }}
    QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="editor"]:disabled {{
        border-left-color: {behavior_editor_edge};
    }}
    QDialog#settingsDialog QGroupBox#settingsAppearanceGroup::title {{
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QGroupBox#settingsEditorGroup {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_pink};
        border-radius: 10px;
    }}
    QDialog#settingsDialog QGroupBox#settingsEditorGroup::title {{
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="localeChoice"] {{
        background: {colors.surface_2};
        border-left: 3px solid {locale_choice_edge};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:hover,
    QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:focus {{
        background: {colors.surface_hover};
        border-left-color: {locale_choice_edge};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:on {{
        background: {colors.pressed};
        border-left-color: {locale_choice_edge};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"],
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"] {{
        background: {colors.surface_1};
        border-left: 2px solid {colors.border_strong};
        color: {colors.text_secondary};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"]:hover,
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]:hover,
    QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"]:focus,
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]:focus {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"]:on {{
        background: {colors.pressed};
        border-left-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="interface"] {{
        border-left-color: {colors.accent_alt};
    }}
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="interface"]:hover,
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="interface"]:focus {{
        border-left-color: {colors.accent_alt};
    }}
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="editor"] {{
        border-left-color: {colors.accent_pink};
    }}
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="editor"]:hover,
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="editor"]:focus {{
        border-left-color: {colors.accent_pink};
    }}
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="interface"]:on {{
        border-left-color: {typography_interface_edge};
    }}
    QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="editor"]:on {{
        border-left-color: {typography_editor_edge};
    }}
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button,
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::down-button {{
        background: {spinbox_stepper_surface};
        border-left: 1px solid {colors.border};
        color: {spinbox_stepper_foreground};
        width: 22px;
    }}
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button {{
        border-top-right-radius: 6px;
        subcontrol-origin: border;
        subcontrol-position: top right;
    }}
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::down-button {{
        border-bottom-right-radius: 6px;
        border-top: 1px solid {colors.border};
        subcontrol-origin: border;
        subcontrol-position: bottom right;
    }}
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button:hover,
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::down-button:hover {{
        background: {spinbox_stepper_hover};
        color: {spinbox_stepper_hover_foreground};
    }}
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button:pressed,
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::down-button:pressed {{
        background: {spinbox_stepper_pressed};
        color: {spinbox_stepper_pressed_foreground};
    }}
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button:disabled,
    QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::down-button:disabled {{
        background: {colors.surface_2};
        color: {colors.text_muted};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="identityChoice"] {{
        background: {colors.surface_3};
        border-left: 3px solid {colors.accent};
        font-weight: 700;
    }}
    QDialog#settingsDialog QComboBox[settingsRole="identityChoice"]:hover {{
        background: {colors.surface_hover};
        border-left-color: {colors.accent_alt};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="identityChoice"]:focus {{
        background: {colors.surface_hover};
        border-left-color: {colors.accent_pink};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="identityChoice"]:on {{
        background: {colors.pressed};
        border-left-color: {colors.accent_alt};
    }}
    QDialog#settingsDialog QComboBox#settingsTheme:disabled,
    QDialog#settingsDialog QComboBox#settingsAccent:disabled,
    QDialog#settingsDialog [settingsRole="typographyChoice"]:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left-color: {locale_choice_edge};
        color: {colors.text_muted};
    }}
    QLabel#settingsNote {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 2px solid {colors.accent_alt};
        border-radius: 6px;
        color: {colors.text_secondary};
        font-weight: 600;
        padding: 6px 8px;
    }}
    QLabel#settingsFontNote {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        border-left: 2px solid {colors.accent_pink};
        border-radius: 6px;
        color: {colors.text_secondary};
        padding: 6px 8px;
    }}
    QLabel#settingsDraftStatus {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.success};
        border-radius: 6px;
        color: {colors.text_secondary};
        font-weight: 600;
        padding: 6px 8px;
    }}
    QLabel#settingsDraftStatus[draftState="changed"] {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        border-left-color: {colors.accent_gold};
        color: {warning_foreground};
    }}
    QDialog#settingsDialog QScrollArea#settingsScroll {{
        background: transparent;
        border: 0;
    }}
    QDialog#settingsDialog QScrollArea#settingsScroll > QWidget#qt_scrollarea_viewport,
    QDialog#settingsDialog QWidget#settingsContent {{
        background: transparent;
        border: 0;
    }}
    QScrollBar:vertical {{
        background: {colors.surface_1};
        margin: 3px;
        width: 8px;
    }}
    QScrollBar::handle:vertical {{
        background: {colors.border_strong};
        border-radius: 4px;
        min-height: 28px;
    }}
    QScrollBar::handle:vertical:hover {{
        background: {colors.accent};
    }}
    QScrollBar:horizontal {{
        background: {colors.surface_1};
        height: 8px;
        margin: 3px;
    }}
    QScrollBar::handle:horizontal {{
        background: {colors.border_strong};
        border-radius: 4px;
        min-width: 28px;
    }}
    QScrollBar::handle:horizontal:hover {{
        background: {colors.accent};
    }}
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical,
    QScrollBar::add-line:horizontal,
    QScrollBar::sub-line:horizontal, QScrollBar::add-page:horizontal,
    QScrollBar::sub-page:horizontal {{
        background: transparent;
        border: 0;
        height: 0;
        width: 0;
    }}
    QDialog {{
        background: {colors.surface_1};
    }}
    QDialog#settingsDialog,
    QDialog#commandPalette {{
        background: {colors.surface_1};
        border: 1px solid {colors.border_strong};
        border-radius: 10px;
    }}
    QDialog#settingsDialog {{
        background: {colors.surface_0};
        border-color: {colors.border_strong};
        border-radius: 14px;
        border-top: 3px solid {colors.accent_alt};
    }}
    QDialog#settingsDialog QDialogButtonBox#dialogActions {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        border-radius: 8px;
        padding: 8px 10px 2px;
    }}
    QDialog#settingsDialog QPushButton#resetSettingsAction {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left: 2px solid {colors.accent_gold};
        color: {colors.text_secondary};
        min-height: 30px;
    }}
    QDialog#settingsDialog QPushButton#resetSettingsAction:hover,
    QDialog#settingsDialog QPushButton#resetSettingsAction:focus {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        color: {warning_foreground};
    }}
    QDialog#settingsDialog QPushButton#resetSettingsAction:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent_gold};
        color: {colors.text_primary};
    }}
    QDialog#settingsDialog QPushButton#resetSettingsAction:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        color: {colors.text_muted};
    }}
    QDialog#commandPalette {{
        border-top: 3px solid {colors.accent_pink};
    }}
    QDialog#pluginCatalogDialog,
    QDialog#pluginStatusDialog {{
        background: {colors.surface_1};
        border: 1px solid {colors.border_strong};
        border-top: 3px solid {colors.accent_alt};
    }}
    QDialog#pluginCatalogDialog QLabel#dialogSummary,
    QDialog#pluginStatusDialog QLabel#dialogSummary {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_alt};
        border-radius: 7px;
        color: {colors.text_primary};
        padding: 8px 10px;
    }}
    QDialog#pluginCatalogDialog QListWidget#catalogEntries,
    QDialog#pluginStatusDialog QListWidget#pluginStatusEntries {{
        background: {colors.surface_0};
        border: 1px solid {colors.border_strong};
        border-radius: 9px;
        padding: 5px;
    }}
    QDialog#pluginCatalogDialog QListWidget#catalogEntries:focus,
    QDialog#pluginStatusDialog QListWidget#pluginStatusEntries:focus {{
        border-color: {colors.accent_alt};
    }}
    QDialog#pluginCatalogDialog QListWidget#catalogEntries::item:selected,
    QDialog#pluginStatusDialog QListWidget#pluginStatusEntries::item:selected {{
        background: {colors.selection};
        border-left: 4px solid {colors.accent_alt};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QDialog#pluginCatalogDialog QLabel#dialogHint {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 2px solid {colors.accent_pink};
        border-radius: 6px;
        color: {colors.text_secondary};
        padding: 5px 8px;
    }}
    QDialog#pluginCatalogDialog QPushButton#primaryAction:disabled,
    QDialog#pluginCatalogDialog QPushButton#warningAction:disabled,
    QDialog#pluginStatusDialog QPushButton#primaryAction:disabled,
    QDialog#pluginStatusDialog QPushButton#warningAction:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left: 3px solid {colors.border_strong};
        color: {colors.text_muted};
        font-weight: 600;
    }}
    QDialog#workspaceSearchDialog {{
        background: {colors.surface_1};
        border: 1px solid {colors.border_strong};
        border-top: 3px solid {colors.accent_alt};
    }}
    QDialog#workspaceSearchDialog QLabel#workspaceSearchRoot {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left: 3px solid {colors.accent_alt};
        color: {colors.text_secondary};
    }}
    QDialog#workspaceSearchDialog QFrame#workspaceSearchQueryCard {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_pink};
        border-radius: 10px;
    }}
    QDialog#workspaceSearchDialog QLineEdit#workspaceSearchQuery {{
        background: {colors.surface_0};
        border: 1px solid {colors.border_strong};
        border-radius: 8px;
        font-weight: 600;
        min-height: 32px;
        padding: 5px 9px;
    }}
    QDialog#workspaceSearchDialog QLineEdit#workspaceSearchQuery:hover {{
        border-color: {colors.accent};
    }}
    QDialog#workspaceSearchDialog QLineEdit#workspaceSearchQuery:focus {{
        border-color: {colors.accent_alt};
        background: {colors.surface_1};
    }}
    QDialog#workspaceSearchDialog QCheckBox {{
        background: transparent;
        border-radius: 5px;
        color: {colors.text_secondary};
        padding: 3px 5px;
    }}
    QDialog#workspaceSearchDialog QCheckBox:hover,
    QDialog#workspaceSearchDialog QCheckBox:focus {{
        background: {colors.surface_3};
        color: {colors.text_primary};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults,
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchDiagnostics {{
        background: {colors.surface_0};
        border: 1px solid {colors.border_strong};
        border-radius: 9px;
        padding: 5px;
    }}
    QWidget#workspaceSearchResultsStage {{
        background: transparent;
    }}
    QLabel#workspaceSearchEmpty {{
        background: {colors.surface_1};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_alt};
        border-radius: 10px;
        color: {colors.text_secondary};
        font-weight: 600;
        padding: 18px;
    }}
    QLabel#workspaceSearchEmpty[state="working"] {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left-color: {colors.accent_alt};
        color: {colors.text_secondary};
    }}
    QLabel#workspaceSearchEmpty[state="warning"] {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        border-left-color: {colors.accent_gold};
        color: {warning_foreground};
    }}
    QLabel#workspaceSearchEmpty[state="error"] {{
        background: {colors.error_bg};
        border-color: {colors.danger};
        border-left-color: {colors.danger};
        color: {error_foreground};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults:focus,
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchDiagnostics:focus {{
        border-color: {colors.accent_alt};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults::item {{
        border-radius: 5px;
        color: {colors.text_secondary};
        padding: 7px 9px;
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults::item:alternate {{
        background: {colors.surface_1};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults::item:hover {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults::item:selected {{
        background: {colors.selection};
        border-left: 3px solid {colors.accent_alt};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchResults::item:selected:disabled {{
        background: {colors.surface_2};
        border-left-color: {colors.border_strong};
        color: {colors.text_muted};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchDiagnostics {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        border-left: 3px solid {colors.accent_gold};
    }}
    QDialog#workspaceSearchDialog QListWidget#workspaceSearchDiagnostics::item {{
        color: {warning_foreground};
        padding: 5px 7px;
    }}
    QDialog#workspaceSearchDialog QToolButton#workspaceSearchDiagnosticsToggle {{
        background: {colors.warning_bg};
        border: 1px solid {colors.accent_gold};
        border-left: 3px solid {colors.accent_gold};
        border-radius: 6px;
        color: {warning_foreground};
        font-weight: 600;
        padding: 5px 8px;
    }}
    QDialog#workspaceSearchDialog QToolButton#workspaceSearchDiagnosticsToggle:hover,
    QDialog#workspaceSearchDialog QToolButton#workspaceSearchDiagnosticsToggle:focus {{
        background: {colors.accent_gold};
        color: {colors.on_accent_gold};
    }}
    QDialog#workspaceSearchDialog QToolButton#workspaceSearchDiagnosticsToggle:checked {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        color: {colors.text_primary};
    }}
    QDialog#workspaceSearchDialog QToolButton#workspaceSearchDiagnosticsToggle:disabled {{
        background: {colors.surface_2};
        border-color: {colors.border};
        border-left-color: {colors.border_strong};
        color: {colors.text_muted};
        font-weight: 600;
    }}
    QMessageBox {{
        background: {colors.surface_1};
        border: 1px solid {colors.border_strong};
        border-radius: 12px;
        padding: 6px;
    }}
    QMessageBox#messageDialog,
    QMessageBox#aboutDialog {{
        border-top: 3px solid {colors.accent_alt};
    }}
    QMessageBox#errorDialog {{
        border-top: 3px solid {colors.danger};
    }}
    QMessageBox#recoveryPrompt {{
        border-top: 3px solid {colors.accent_gold};
    }}
    QMessageBox QLabel {{
        color: {colors.text_primary};
        padding: 4px 6px;
    }}
    QMessageBox QLabel#qt_msgbox_informativelabel {{
        color: {colors.text_secondary};
    }}
    QMessageBox QPushButton {{
        min-width: 96px;
        margin: 4px 2px;
    }}
    QDialog#commandPalette {{
        background: {colors.surface_1};
    }}
    QLineEdit#commandPaletteQuery {{
        background: {colors.surface_0};
        border-color: {colors.accent_alt};
        font-weight: 600;
        min-height: 34px;
        padding: 6px 11px;
    }}
    QWidget#commandPaletteResultsStage {{
        background: transparent;
    }}
    QLabel#commandPaletteEmpty {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.accent_alt};
        border-radius: 9px;
        color: {colors.text_secondary};
        font-weight: 600;
        padding: 16px;
    }}
    QListWidget#commandPaletteList {{
        background: {colors.surface_0};
        border-color: {colors.border};
        border-radius: 8px;
        padding: 4px;
    }}
    QListWidget#commandPaletteList:focus {{
        border-color: {colors.accent_alt};
    }}
    QListWidget#commandPaletteList::item {{
        border-radius: 5px;
        padding: 8px 10px;
    }}
    QListWidget#commandPaletteList::item:alternate {{
        background: {colors.surface_1};
    }}
    QListWidget#commandPaletteList::item:hover {{
        background: {colors.surface_hover};
        color: {colors.text_primary};
    }}
    QListWidget#commandPaletteList::item:selected {{
        background: {colors.selection};
        border-left: 3px solid {colors.accent_alt};
        color: {colors.text_primary};
        font-weight: 700;
    }}
    QListWidget#commandPaletteList::item:selected:hover {{
        background: {colors.selection};
        border-left: 3px solid {colors.accent};
        color: {colors.text_primary};
    }}
    QLabel#commandPaletteHint {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-left: 2px solid {colors.accent_pink};
        border-radius: 6px;
        color: {colors.text_secondary};
        padding: 5px 8px;
    }}
    QLabel#dialogSummary {{
        color: {colors.text_secondary};
        font-size: {ui_font_size}pt;
        font-weight: 600;
    }}
    QLabel#dialogHint {{
        color: {colors.text_muted};
    }}
    QToolButton {{
        background: transparent;
        border: 1px solid transparent;
        border-radius: 5px;
        color: {colors.text_secondary};
        padding: 5px 8px;
    }}
    QToolButton:hover {{
        background: {colors.surface_3};
        border-color: {colors.border_strong};
        color: {colors.text_primary};
    }}
    QToolButton:pressed {{
        background: {colors.pressed};
        border-color: {colors.accent};
        color: {colors.text_primary};
    }}
    QWidget#findBar {{
        background: {colors.surface_2};
        border: 1px solid {colors.border};
        border-top: 2px solid {colors.accent_alt};
        border-radius: 12px;
        margin: 8px 10px 6px;
    }}
    QWidget#findBar QLabel {{
        color: {colors.text_secondary};
        font-weight: 600;
    }}
    QWidget#findBar QCheckBox {{
        background: transparent;
        border-radius: 5px;
        color: {colors.text_secondary};
        padding: 3px 5px;
    }}
    QWidget#findBar QCheckBox:hover,
    QWidget#findBar QCheckBox:focus {{
        background: {colors.surface_3};
        color: {colors.text_primary};
    }}
    QLabel#findStatus {{
        background: {colors.surface_3};
        border: 1px solid {colors.border};
        border-left: 3px solid {colors.border_strong};
        border-radius: 6px;
        color: {colors.text_secondary};
        font-weight: 600;
        padding: 4px 8px;
    }}
    QLabel#findStatus[state="info"] {{
        background: {colors.surface_3};
        border-color: {colors.border};
        border-left-color: {colors.border_strong};
        color: {colors.text_secondary};
    }}
    QLabel#findStatus[state="working"] {{
        background: {colors.pressed};
        border-color: {colors.accent_alt};
        border-left-color: {colors.accent_alt};
        color: {working_foreground};
        font-weight: 600;
    }}
    QLabel#findStatus[state="success"] {{
        background: {colors.success_bg};
        border-color: {colors.success};
        border-left-color: {colors.success};
        color: {success_foreground};
        font-weight: 600;
    }}
    QLabel#findStatus[state="warning"] {{
        background: {colors.warning_bg};
        border-color: {colors.accent_gold};
        border-left-color: {colors.accent_gold};
        color: {warning_foreground};
        font-weight: 600;
    }}
    QLabel#findStatus[state="error"] {{
        background: {colors.error_bg};
        border-color: {colors.danger};
        border-left-color: {colors.danger};
        color: {error_foreground};
        font-weight: 600;
    }}
    QToolTip {{
        background: {colors.surface_3};
        border: 1px solid {colors.accent_alt};
        border-radius: 7px;
        color: {colors.text_primary};
        font-size: {max(8, ui_font_size - 1)}pt;
        padding: 6px 8px;
    }}
    """


def _accent_alt_text_foreground(colors: ThemeColors) -> str:
    """Resolve accent-alt text for the deepest shared surface."""
    return qss_foreground_tokens(colors).accent_alt_text
