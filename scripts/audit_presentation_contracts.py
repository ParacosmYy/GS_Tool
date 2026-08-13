"""Audit presentation dependency, notification, and worker observability contracts."""

from __future__ import annotations

import ast
import sys
from pathlib import Path

from quillforge.presentation.i18n import localize_message
from quillforge.presentation.theme_tokens import (
    contrast_ratio,
    editor_color_tokens,
    qss_foreground_tokens,
    readable_edge_foreground_for_surfaces,
    theme_colors,
)

ROOT = Path(__file__).resolve().parents[1]
PRESENTATION_ROOT = ROOT / "src" / "quillforge" / "presentation"
I18N_PATH = PRESENTATION_ROOT / "i18n.py"


def _parse(path: Path) -> ast.Module:
    return ast.parse(path.read_text(encoding="utf-8"), filename=str(path))


def _imported_modules(tree: ast.Module) -> list[str]:
    modules: list[str] = []
    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            modules.extend(alias.name for alias in node.names)
        elif isinstance(node, ast.ImportFrom):
            modules.append(node.module or "")
    return modules


def _english_catalog_keys() -> set[str]:
    """Read the canonical literal-key catalog without importing Qt code."""
    tree = _parse(I18N_PATH)
    for node in tree.body:
        if not isinstance(node, ast.AnnAssign) or not isinstance(node.target, ast.Name):
            continue
        if node.target.id != "_ENGLISH" or not isinstance(node.value, ast.Dict):
            continue
        keys = {
            key.value
            for key in node.value.keys
            if isinstance(key, ast.Constant) and isinstance(key.value, str)
        }
        return keys
    return set()


def _audit_i18n_literal_keys() -> list[str]:
    """Reject literal ``tr()`` calls whose keys are absent from the catalog."""
    catalog_keys = _english_catalog_keys()
    violations: list[str] = []
    if not catalog_keys:
        return ["i18n.py: _ENGLISH literal-key catalog is missing or not a dict"]
    for path in sorted(PRESENTATION_ROOT.glob("*.py")):
        tree = _parse(path)
        relative = path.relative_to(ROOT).as_posix()
        for node in ast.walk(tree):
            if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Name):
                continue
            if node.func.id != "tr" or not node.args:
                continue
            key = node.args[0]
            if isinstance(key, ast.Constant) and isinstance(key.value, str):
                if key.value not in catalog_keys:
                    violations.append(f"{relative}:{node.lineno}: missing i18n key {key.value!r}")
    return violations


def _audit_static_notification_localization() -> list[str]:
    """Reject static notification text that remains unchanged in zh-CN."""
    violations: list[str] = []
    source_root = ROOT / "src" / "quillforge"
    for path in sorted(source_root.rglob("*.py")):
        tree = _parse(path)
        relative = path.relative_to(ROOT).as_posix()
        for node in ast.walk(tree):
            if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Attribute):
                continue
            if node.func.attr != "notify" or not node.args:
                continue
            message = node.args[0]
            if not isinstance(message, ast.Constant) or not isinstance(message.value, str):
                continue
            text = message.value
            if any(char.isalpha() and char.isascii() for char in text):
                if localize_message(text, "zh-CN") == text:
                    violations.append(
                        f"{relative}:{node.lineno}: static notify text is not localized: {text!r}"
                    )
    return violations


def _audit_theme_contrast_contract() -> list[str]:
    """Reject resolved semantic text colors below the normal-text contrast floor."""
    violations: list[str] = []
    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            editor = editor_color_tokens(theme, accent)
            qss = qss_foreground_tokens(colors)
            checks = (
                ("on_accent", colors.on_accent, colors.accent),
                ("on_accent_pink", colors.on_accent_pink, colors.accent_pink),
                ("on_accent_gold", colors.on_accent_gold, colors.accent_gold),
                ("text_primary", colors.text_primary, colors.surface_0),
                ("text_secondary", colors.text_secondary, colors.surface_0),
                ("text_muted", colors.text_muted, colors.surface_0),
                ("editor_number", editor.number, editor.canvas),
                ("editor_keyword", editor.keyword, editor.canvas),
                ("editor_type", editor.type_name, editor.canvas),
                ("editor_operator", editor.operator, editor.canvas),
                ("editor_error", editor.error, editor.canvas),
                ("editor_decorator", editor.decorator, editor.canvas),
                ("qss_accent_alt_text", qss.accent_alt_text, colors.surface_3),
                ("qss_accent_alt_fill", qss.accent_alt_fill, colors.accent_alt),
                ("qss_selection", qss.selection, colors.selection),
                ("qss_warning", qss.warning, colors.warning_bg),
                ("qss_success", qss.success, colors.success_bg),
                ("qss_working", qss.working, colors.pressed),
                ("qss_error", qss.error, colors.error_bg),
            )
            for name, foreground, background in checks:
                ratio = contrast_ratio(foreground, background)
                if ratio < 4.5:
                    violations.append(
                        f"theme {theme}/{accent} {name} contrast is {ratio:.2f}, below 4.5"
                    )
    return violations


def _audit_tooltip_contract() -> list[str]:
    """Keep global tooltip text and edge treatments readable in every theme."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    source = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "QToolTip {{",
        "background: {colors.surface_3};",
        "border-left: 3px solid {tooltip_edge};",
        "color: {colors.text_primary};",
        "font-weight: 600;",
        "padding: 6px 9px;",
        "tooltip_edge = readable_edge_foreground_for_surfaces(",
    ):
        if fragment not in source:
            violations.append(f"theme.py: Tooltip contract is missing {fragment!r}")

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            tooltip_edge = readable_edge_foreground_for_surfaces(
                colors.accent_alt,
                (colors.surface_3,),
                colors.text_primary,
            )
            text_ratio = contrast_ratio(colors.text_primary, colors.surface_3)
            edge_ratio = contrast_ratio(tooltip_edge, colors.surface_3)
            if text_ratio < 4.5:
                violations.append(
                    f"theme {theme}/{accent} Tooltip text contrast is {text_ratio:.2f}, below 4.5"
                )
            if edge_ratio < 3.0:
                violations.append(
                    f"theme {theme}/{accent} Tooltip edge contrast is {edge_ratio:.2f}, below 3.0"
                )
    return violations


def _audit_shell_separator_contract() -> list[str]:
    """Keep shell splitters theme-aware and visibly interactive."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    source = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "separator_edge = readable_edge_foreground_for_surfaces(",
        "QMainWindow::separator,",
        "QDockWidget::separator {{",
        "background: {separator_edge};",
        "QMainWindow::separator:hover,",
        "QDockWidget::separator:hover {{",
        "background: {colors.accent_alt};",
    ):
        if fragment not in source:
            violations.append(f"theme.py: Shell separator contract is missing {fragment!r}")

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            shell_surfaces = (colors.surface_0, colors.surface_1, colors.surface_2)
            separator_edge = readable_edge_foreground_for_surfaces(
                colors.border,
                shell_surfaces,
                colors.text_primary,
            )
            for surface in shell_surfaces:
                edge_ratio = contrast_ratio(separator_edge, surface)
                hover_ratio = contrast_ratio(colors.accent_alt, surface)
                if edge_ratio < 3.0:
                    violations.append(
                        f"theme {theme}/{accent} separator edge contrast is "
                        f"{edge_ratio:.2f}, below 3.0"
                    )
                if hover_ratio < 3.0:
                    violations.append(
                        f"theme {theme}/{accent} separator hover contrast is "
                        f"{hover_ratio:.2f}, below 3.0"
                    )
    return violations


def _audit_scrollbar_contract() -> list[str]:
    """Keep scrollbars compact, theme-aware, and visibly interactive."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    source = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "scrollbar_handle = readable_edge_foreground_for_surfaces(",
        "scrollbar_hover = readable_edge_foreground_for_surfaces(",
        "scrollbar_pressed = readable_edge_foreground_for_surfaces(",
        "QScrollBar:vertical,",
        "QScrollBar:horizontal {{",
        "QScrollBar::handle:vertical,",
        "QScrollBar::handle:horizontal {{",
        "background: {scrollbar_handle};",
        "QScrollBar::handle:vertical:hover,",
        "QScrollBar::handle:horizontal:hover {{",
        "background: {scrollbar_hover};",
        "QScrollBar::handle:vertical:pressed,",
        "QScrollBar::handle:horizontal:pressed {{",
        "background: {scrollbar_pressed};",
        "QScrollBar::add-line,",
        "QScrollBar::sub-line {{",
    ):
        if fragment not in source:
            violations.append(f"theme.py: Scrollbar contract is missing {fragment!r}")

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            scrollbar_surfaces = (colors.surface_1, colors.surface_2)
            states = (
                (
                    "handle",
                    readable_edge_foreground_for_surfaces(
                        colors.border_strong,
                        scrollbar_surfaces,
                        colors.text_primary,
                    ),
                ),
                (
                    "hover",
                    readable_edge_foreground_for_surfaces(
                        colors.accent_alt,
                        scrollbar_surfaces,
                        colors.text_primary,
                    ),
                ),
                (
                    "pressed",
                    readable_edge_foreground_for_surfaces(
                        colors.accent,
                        scrollbar_surfaces,
                        colors.text_primary,
                    ),
                ),
            )
            for state, foreground in states:
                for surface in scrollbar_surfaces:
                    ratio = contrast_ratio(foreground, surface)
                    if ratio < 3.0:
                        violations.append(
                            f"theme {theme}/{accent} scrollbar {state} contrast is "
                            f"{ratio:.2f}, below 3.0"
                        )
    return violations


def _audit_combo_expanded_contract() -> list[str]:
    """Keep combo-box popup-open affordances explicit and theme-aware."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    source = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "QComboBox::drop-down {{",
        "background: {colors.surface_2};",
        "subcontrol-origin: border;",
        "subcontrol-position: top right;",
        "QComboBox::drop-down:on {{",
        "background: {colors.pressed};",
        "border-left-color: {colors.accent};",
        "QComboBox::down-arrow:on {{",
        "border-top-color: {colors.text_primary};",
    ):
        if fragment not in source:
            violations.append(f"theme.py: combo expanded contract is missing {fragment!r}")

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            ratio = contrast_ratio(colors.text_primary, colors.pressed)
            if ratio < 4.5:
                violations.append(
                    f"theme {theme}/{accent} combo expanded text contrast is {ratio:.2f}, below 4.5"
                )
    return violations


def _audit_checkbox_indicator_contract() -> list[str]:
    """Keep native checkbox marks readable on every themed indicator state."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    source = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "checkbox_checked_edge = readable_edge_foreground_for_surfaces(",
        "checkbox_checked_hover_edge = readable_edge_foreground_for_surfaces(",
        "checkbox_checked_focus_edge = readable_edge_foreground_for_surfaces(",
        "checkbox_disabled_edge = readable_edge_foreground_for_surfaces(",
        "QCheckBox::indicator:checked {{",
        "background: {colors.pressed};",
        "border-color: {checkbox_checked_edge};",
        "QCheckBox::indicator:checked:hover {{",
        "background: {colors.surface_hover};",
        "border-color: {checkbox_checked_hover_edge};",
        "QCheckBox:focus::indicator:checked {{",
        "border-color: {checkbox_checked_focus_edge};",
        "QCheckBox::indicator:checked:disabled {{",
        "background: {colors.surface_2};",
        "border-color: {checkbox_disabled_edge};",
    ):
        if fragment not in source:
            violations.append(f"theme.py: checkbox indicator contract is missing {fragment!r}")

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            states = (
                ("checked", colors.text_primary, colors.pressed),
                ("checked-hover", colors.text_primary, colors.surface_hover),
                ("checked-disabled", colors.text_muted, colors.surface_2),
            )
            for state, foreground, background in states:
                ratio = contrast_ratio(foreground, background)
                if ratio < 4.5:
                    violations.append(
                        f"theme {theme}/{accent} checkbox {state} mark contrast is "
                        f"{ratio:.2f}, below 4.5"
                    )
    return violations


def _audit_tab_close_button_contract() -> list[str]:
    """Keep native tab close glyphs readable in every interactive state."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    tab_path = PRESENTATION_ROOT / "document_tab_surface.py"
    source = theme_path.read_text(encoding="utf-8")
    tab_source = tab_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "tab_close_hover_edge = readable_edge_foreground_for_surfaces(",
        "tab_close_focus_edge = readable_edge_foreground_for_surfaces(",
        "tab_close_pressed_edge = readable_edge_foreground_for_surfaces(",
        "QTabBar#documentTabBar::close-button:hover {{",
        "background: {colors.error_bg};",
        "border-color: {tab_close_hover_edge};",
        "QTabBar#documentTabBar::close-button:focus {{",
        "background: {colors.surface_hover};",
        "border-color: {tab_close_focus_edge};",
        "QTabBar#documentTabBar::close-button:pressed {{",
        "background: {colors.pressed};",
        "border-color: {tab_close_pressed_edge};",
        "QTabBar#documentTabBar::close-button:disabled {{",
        "background: {colors.surface_2};",
        "border-color: {colors.border};",
    ):
        if fragment not in source:
            violations.append(f"theme.py: tab close contract is missing {fragment!r}")
    for fragment in (
        "self._widget.setTabsClosable(True)",
        "self._widget.tabCloseRequested.connect(close_requested)",
    ):
        if fragment not in tab_source:
            violations.append(
                f"document_tab_surface.py: tab close boundary is missing {fragment!r}"
            )

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            states = (
                ("hover", colors.text_primary, colors.error_bg),
                ("focus", colors.text_primary, colors.surface_hover),
                ("pressed", colors.text_primary, colors.pressed),
                ("disabled", colors.text_muted, colors.surface_2),
            )
            for state, foreground, background in states:
                ratio = contrast_ratio(foreground, background)
                if ratio < 4.5:
                    violations.append(
                        f"theme {theme}/{accent} tab close {state} glyph contrast is "
                        f"{ratio:.2f}, below 4.5"
                    )
    return violations


def _audit_typography_stepper_contract() -> list[str]:
    """Keep typography size steppers themed without changing value behavior."""
    theme_path = PRESENTATION_ROOT / "theme.py"
    source = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "spinbox_stepper_surface = colors.surface_2",
        "spinbox_stepper_hover = colors.surface_hover",
        "spinbox_stepper_pressed = colors.pressed",
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button,',
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::down-button {{',
        "background: {spinbox_stepper_surface};",
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button:hover,',
        "background: {spinbox_stepper_hover};",
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button:pressed,',
        "background: {spinbox_stepper_pressed};",
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]::up-button:disabled,',
        "color: {colors.text_muted};",
        "subcontrol-position: top right;",
        "subcontrol-position: bottom right;",
    ):
        if fragment not in source:
            violations.append(f"theme.py: Typography stepper contract is missing {fragment!r}")

    for theme in ("ink-violet", "paper-sand", "sakura-pop"):
        for accent in ("violet", "cyan", "rose", "amber"):
            colors = theme_colors(theme, accent)
            states = (
                ("normal", colors.surface_2, colors.text_primary),
                ("hover", colors.surface_hover, colors.text_primary),
                ("pressed", colors.pressed, colors.text_primary),
            )
            for state, background, foreground in states:
                ratio = contrast_ratio(foreground, background)
                if ratio < 4.5:
                    violations.append(
                        f"theme {theme}/{accent} typography stepper {state} text "
                        f"contrast is {ratio:.2f}, below 4.5"
                    )
    return violations


def _audit_workspace_file_activation_contract() -> list[str]:
    """Keep file activation connected from the tree to async document opening."""
    contracts = {
        "src/quillforge/presentation/workspace_panel.py": (
            "file_requested = pyqtSignal(object)",
            "self._tree.itemClicked.connect(self._on_item_clicked)",
            'self._emit_item_intent(item, expected_kind="file")',
            "self.file_requested.emit(path)",
            "self.directory_requested.emit(path)",
        ),
        "src/quillforge/presentation/workspace_surface.py": (
            "self._panel.file_requested.connect(callbacks.file_requested)",
            "self._panel.directory_requested.connect(callbacks.directory_requested)",
        ),
        "src/quillforge/presentation/workspace_file_activation_coordinator.py": (
            "self._ports.contains(path)",
            "self._ports.find_tab(path)",
            "self._ports.start_open(path)",
        ),
        "src/quillforge/presentation/main_window.py": (
            "file_requested=self._workspace_file_activation_coordinator.activate",
            "start_open=self._start_open,",
            "return self._document_open_admission_coordinator.admit(",
        ),
        "src/quillforge/presentation/document_open_admission_coordinator.py": (
            "self._ports.submit_open(",
            "lambda: self._ports.open_document(path)",
        ),
        "src/quillforge/presentation/file_dialog_surface.py": (
            "QFileDialog.getOpenFileName(",
            "return Path(selected) if selected else None",
        ),
    }
    violations: list[str] = []
    for relative, required_fragments in contracts.items():
        path = ROOT / relative
        source = path.read_text(encoding="utf-8") if path.is_file() else ""
        for fragment in required_fragments:
            if fragment not in source:
                violations.append(f"{relative}: file-open contract is missing {fragment!r}")
    return violations


def _audit_workspace_open_affordance_contract() -> list[str]:
    """Keep folder/file actions distinct, localized, and assistive-technology readable."""
    panel_path = PRESENTATION_ROOT / "workspace_panel.py"
    i18n_path = PRESENTATION_ROOT / "i18n.py"
    theme_path = PRESENTATION_ROOT / "theme.py"
    panel = panel_path.read_text(encoding="utf-8")
    i18n = i18n_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        'self._open_button.setProperty("workspaceRole", "folderPicker")',
        'self._file_button.setProperty("workspaceRole", "documentPicker")',
        'self._back_button.setProperty("workspaceRole", "parentNavigation")',
        'self._cancel_button.setProperty("workspaceRole", "cancelOperation")',
        '"workspace.open_folder_hint"',
        '"workspace.open_file_hint"',
        '"workspace.up_hint"',
        '"workspace.cancel_hint"',
        "button.setAccessibleName(label)",
        "button.setAccessibleDescription(hint)",
        "button.setToolTip(hint)",
    ):
        if fragment not in panel and fragment not in i18n:
            violations.append(f"workspace affordance contract is missing {fragment!r}")
    for fragment in (
        'QWidget#workspacePanel QPushButton[workspaceRole="folderPicker"] {{',
        'QWidget#workspacePanel QPushButton[workspaceRole="documentPicker"] {{',
    ):
        if fragment not in theme:
            violations.append(f"theme.py: workspace affordance selector is missing {fragment!r}")
    locale_projection = panel.find("def set_locale(")
    accessibility_projection = panel.find("self._set_action_accessibility(", locale_projection)
    if locale_projection < 0 or accessibility_projection < 0:
        violations.append("workspace_panel.py: action accessibility must refresh in set_locale()")
    return violations


def _audit_settings_typography_contract() -> list[str]:
    """Keep locale, appearance, typography, persistence, and projection connected."""
    contracts = {
        "src/quillforge/application/settings.py": (
            "CURRENT_SETTINGS_VERSION = 3",
            "SUPPORTED_EDITOR_FONTS",
            "SUPPORTED_UI_FONTS",
            "SUPPORTED_FONT_STYLES",
            "SUPPORTED_LOCALES",
            "SUPPORTED_THEMES",
            "SUPPORTED_ACCENTS",
            "8 <= editor.font_size <= 32",
            "9 <= appearance.ui_font_size <= 16",
            "motion_enabled",
        ),
        "src/quillforge/infrastructure/settings_store.py": (
            '"font_family"',
            '"font_size"',
            '"font_style"',
            '"ui_font_family"',
            '"ui_font_size"',
            '"ui_font_style"',
            '"motion_enabled"',
        ),
        "src/quillforge/presentation/settings_dialog.py": (
            'setObjectName("settingsLanguage")',
            'setObjectName("settingsTheme")',
            'setObjectName("settingsAccent")',
            'setObjectName("settingsUiFont")',
            'setObjectName("settingsUiFontSize")',
            'setObjectName("settingsUiFontStyle")',
            'setObjectName("settingsEditorFont")',
            'setObjectName("settingsEditorFontSize")',
            'setObjectName("settingsEditorFontStyle")',
            'setObjectName("settingsMotion")',
            "QFontDatabase.families()",
            "def _read_installed_font_families() -> frozenset[str] | None:",
            "def _refresh_font_status(self) -> None:",
            "def settings_snapshot(self) -> SettingsSnapshot:",
            "def set_locale(self, locale: Locale) -> None:",
        ),
        "src/quillforge/presentation/settings_save_projection_coordinator.py": (
            "self._ports.apply_snapshot(result)",
            "self._ports.retranslate()",
            "self._ports.apply_editor_settings()",
            "self._ports.animate_transition()",
        ),
        "src/quillforge/presentation/main_window.py": (
            "apply_theme(application, result.appearance)",
            "def _apply_editor_settings_to_tabs(self) -> None:",
            "def _animate_theme_transition(self) -> None:",
            "enabled=self._settings.appearance.motion_enabled",
        ),
        "src/quillforge/presentation/theme.py": (
            "selected.ui_font_family",
            "selected.ui_font_size",
            "selected.ui_font_style",
            'font-family: "{ui_font_family}";',
            "font-size: {ui_font_size}pt;",
            "qss_font_style(ui_font_style)",
            "qss_font_weight(ui_font_style)",
        ),
        "src/quillforge/presentation/editor_document_surface.py": (
            "editor.set_font_family(settings.font_family)",
            "editor.set_font_size(settings.font_size)",
            "editor.set_font_style(settings.font_style)",
            "editor.set_wrap_lines(settings.wrap_lines)",
            "editor.set_line_numbers(settings.show_line_numbers)",
        ),
        "src/quillforge/presentation/editor_widget.py": (
            "def set_font_family(self, family: str) -> None:",
            "def set_font_size(self, size: int) -> None:",
            "def set_font_style(self, style: FontStyle) -> None:",
            "font_with_style(self.font(), self._font_style)",
        ),
        "src/quillforge/presentation/i18n.py": (
            '"settings.font_size_suffix": " pt"',
            '"settings.font_size_suffix": " 磅"',
            '"settings.font_style_regular"',
            '"settings.font_style_semibold"',
            '"settings.font_style_bold"',
            '"settings.font_style_italic"',
            '"settings.font_status"',
            '"settings.font_status_installed"',
            '"settings.font_status_fallback"',
            '"settings.font_status_unknown"',
        ),
    }
    violations: list[str] = []
    for relative, required_fragments in contracts.items():
        path = ROOT / relative
        source = path.read_text(encoding="utf-8") if path.is_file() else ""
        for fragment in required_fragments:
            if fragment not in source:
                violations.append(
                    f"{relative}: settings/typography contract is missing {fragment!r}"
                )
    return violations


def _audit_settings_identity_choice_contract() -> list[str]:
    """Keep settings identity choices behind one semantic presentation role."""
    dialog_path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    theme_path = ROOT / "src" / "quillforge" / "presentation" / "theme.py"
    dialog = dialog_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for object_name, add_fragment in (
        ('setObjectName("settingsTheme")', "self._theme.addItem("),
        ('setObjectName("settingsAccent")', "self._accent.addItem("),
    ):
        object_position = dialog.find(object_name)
        property_position = dialog.find(
            'setProperty("settingsRole", "identityChoice")',
            object_position,
        )
        add_position = dialog.find(add_fragment, property_position)
        if min(object_position, property_position, add_position) < 0 or not (
            object_position < property_position < add_position
        ):
            violations.append(
                f"settings_dialog.py: {object_name} must declare identityChoice "
                "between object identity and item population"
            )
    state_contracts = (
        (
            "base",
            'QDialog#settingsDialog QComboBox[settingsRole="identityChoice"] {{',
            (
                "background: {colors.surface_3};",
                "border-left: 3px solid {colors.accent};",
                "font-weight: 700;",
            ),
        ),
        (
            "hover",
            'QDialog#settingsDialog QComboBox[settingsRole="identityChoice"]:hover {{',
            (
                "background: {colors.surface_hover};",
                "border-left-color: {colors.accent_alt};",
            ),
        ),
        (
            "focus",
            'QDialog#settingsDialog QComboBox[settingsRole="identityChoice"]:focus {{',
            (
                "background: {colors.surface_hover};",
                "border-left-color: {colors.accent_pink};",
            ),
        ),
        (
            "on",
            'QDialog#settingsDialog QComboBox[settingsRole="identityChoice"]:on {{',
            (
                "background: {colors.pressed};",
                "border-left-color: {colors.accent_alt};",
            ),
        ),
    )
    for state, selector, required_fragments in state_contracts:
        start = theme.find(selector)
        next_selector = theme.find(
            "QDialog#settingsDialog QComboBox",
            start + len(selector),
        )
        scope = theme[start:next_selector] if start >= 0 else ""
        if start < 0:
            violations.append(f"theme.py: identityChoice {state} selector is missing")
            continue
        for fragment in required_fragments:
            if fragment not in scope:
                violations.append(f"theme.py: identityChoice {state} state is missing {fragment!r}")
    for fragment in (
        "QDialog#settingsDialog QComboBox#settingsTheme,",
        "QDialog#settingsDialog QComboBox#settingsAccent,",
        "QDialog#settingsDialog QComboBox#settingsTheme:hover,",
        "QDialog#settingsDialog QComboBox#settingsAccent:hover,",
        "QDialog#settingsDialog QComboBox#settingsTheme:focus,",
        "QDialog#settingsDialog QComboBox#settingsAccent:focus,",
        "QDialog#settingsDialog QComboBox#settingsTheme:on,",
        "QDialog#settingsDialog QComboBox#settingsAccent:on,",
    ):
        if fragment in theme:
            violations.append(f"theme.py: duplicated identity selector remains {fragment!r}")
    return violations


def _audit_settings_locale_choice_contract() -> list[str]:
    """Keep the language selector behind one semantic presentation role."""
    dialog_path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    theme_path = ROOT / "src" / "quillforge" / "presentation" / "theme.py"
    dialog = dialog_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    object_position = dialog.find('setObjectName("settingsLanguage")')
    role_position = dialog.find(
        'self._language.setProperty("settingsRole", "localeChoice")',
        object_position,
    )
    item_position = dialog.find("self._language.addItem(", role_position)
    current_position = dialog.find("self._language.setCurrentIndex(", item_position)
    violations: list[str] = []
    if min(object_position, role_position, item_position, current_position) < 0 or not (
        object_position < role_position < item_position < current_position
    ):
        violations.append(
            "settings_dialog.py: settingsLanguage must declare localeChoice before "
            "language item population and current-value setup"
        )
    for fragment in (
        'QDialog#settingsDialog QComboBox[settingsRole="localeChoice"] {{',
        'QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:hover,',
        'QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:focus {{',
        'QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:on {{',
        'QDialog#settingsDialog QComboBox[settingsRole="localeChoice"]:disabled {{',
        "border-left: 3px solid {locale_choice_edge};",
        "border-left-color: {locale_choice_edge};",
    ):
        if fragment not in theme:
            violations.append(f"theme.py: localeChoice QSS is missing {fragment!r}")
    for fragment in (
        "QDialog#settingsDialog QComboBox#settingsLanguage {{",
        "QDialog#settingsDialog QComboBox#settingsLanguage:hover {{",
        "QDialog#settingsDialog QComboBox#settingsLanguage:focus {{",
        "QDialog#settingsDialog QComboBox#settingsLanguage:disabled,",
    ):
        if fragment in theme:
            violations.append(f"theme.py: legacy locale selector remains {fragment!r}")
    return violations


def _audit_settings_accessibility_contract() -> list[str]:
    """Keep Settings control names localized at the set_locale boundary."""
    path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    tree = _parse(path)
    violations: list[str] = []
    settings_dialog = next(
        (
            node
            for node in tree.body
            if isinstance(node, ast.ClassDef) and node.name == "SettingsDialog"
        ),
        None,
    )
    methods = (
        {
            node.name: node
            for node in settings_dialog.body
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
        }
        if settings_dialog is not None
        else {}
    )
    helper = methods.get("_refresh_accessible_names")
    locale_method = methods.get("set_locale")
    if helper is None or locale_method is None:
        violations.append("settings_dialog.py: localized accessible-name methods are missing")
        return violations

    def self_attribute(node: ast.AST) -> str | None:
        if (
            isinstance(node, ast.Attribute)
            and isinstance(node.value, ast.Name)
            and node.value.id == "self"
        ):
            return f"self.{node.attr}"
        return None

    def text_call(node: ast.AST, receiver: str) -> bool:
        return (
            isinstance(node, ast.Call)
            and not node.args
            and not node.keywords
            and isinstance(node.func, ast.Attribute)
            and node.func.attr == "text"
            and isinstance(node.func.value, ast.Name)
            and node.func.value.id == receiver
        )

    def accessible_name_call(node: ast.AST, receiver: str) -> bool:
        return (
            isinstance(node, ast.Call)
            and len(node.args) == 1
            and not node.keywords
            and isinstance(node.func, ast.Attribute)
            and node.func.attr == "setAccessibleName"
            and isinstance(node.func.value, ast.Name)
            and node.func.value.id == "widget"
            and text_call(node.args[0], receiver)
        )

    def loop_has_accessible_name(loop: ast.For, receiver: str) -> bool:
        return (
            len(loop.body) == 1
            and isinstance(loop.body[0], ast.Expr)
            and accessible_name_call(loop.body[0].value, receiver)
        )

    loops = [node for node in helper.body if isinstance(node, ast.For)]
    if len(loops) != 2:
        violations.append(
            "settings_dialog.py: accessible-name helper must contain exactly two loops"
        )
    else:
        mapping_loop, behavior_loop = loops
        expected_mapping = (
            ("self._language", "self._language_label"),
            ("self._theme", "self._theme_label"),
            ("self._accent", "self._accent_label"),
            ("self._ui_font", "self._ui_font_label"),
            ("self._ui_font_size", "self._ui_font_size_label"),
            ("self._ui_font_style", "self._ui_font_style_label"),
            ("self._editor_font", "self._editor_font_label"),
            ("self._editor_font_size", "self._editor_font_size_label"),
            ("self._editor_font_style", "self._editor_font_style_label"),
        )
        mapping_pairs = []
        if isinstance(mapping_loop.iter, ast.Tuple):
            for pair in mapping_loop.iter.elts:
                if isinstance(pair, ast.Tuple) and len(pair.elts) == 2:
                    mapping_pairs.append(
                        (self_attribute(pair.elts[0]), self_attribute(pair.elts[1]))
                    )
        if tuple(mapping_pairs) != expected_mapping or len(mapping_pairs) != len(
            set(mapping_pairs)
        ):
            violations.append(
                "settings_dialog.py: accessible-name label mapping must contain the "
                "nine expected Settings controls exactly once"
            )
        mapping_target = (
            isinstance(mapping_loop.target, ast.Tuple)
            and len(mapping_loop.target.elts) == 2
            and all(isinstance(element, ast.Name) for element in mapping_loop.target.elts)
            and [element.id for element in mapping_loop.target.elts] == ["widget", "label"]
        )
        if not mapping_target or not loop_has_accessible_name(mapping_loop, "label"):
            violations.append(
                "settings_dialog.py: label mapping must use widget.setAccessibleName(label.text())"
            )
        expected_behavior = ("self._wrap_lines", "self._line_numbers", "self._motion")
        behavior_values = (
            tuple(self_attribute(element) for element in behavior_loop.iter.elts)
            if isinstance(behavior_loop.iter, ast.Tuple)
            else ()
        )
        if behavior_values != expected_behavior or len(behavior_values) != len(
            set(behavior_values)
        ):
            violations.append(
                "settings_dialog.py: behavior accessible-name mapping must contain "
                "wrap, line-number, and motion controls exactly once"
            )
        behavior_target = (
            isinstance(behavior_loop.target, ast.Name) and behavior_loop.target.id == "widget"
        )
        if not behavior_target or not loop_has_accessible_name(behavior_loop, "widget"):
            violations.append(
                "settings_dialog.py: behavior mapping must use "
                "widget.setAccessibleName(widget.text())"
            )
        setter_calls = [
            node
            for node in ast.walk(helper)
            if isinstance(node, ast.Call)
            and isinstance(node.func, ast.Attribute)
            and node.func.attr == "setAccessibleName"
        ]
        if len(setter_calls) != 2:
            violations.append(
                "settings_dialog.py: accessible-name helper must have exactly two setter calls"
            )

    def self_method_call(node: ast.AST, method_name: str) -> bool:
        return (
            isinstance(node, ast.Call)
            and not node.args
            and not node.keywords
            and isinstance(node.func, ast.Attribute)
            and node.func.attr == method_name
            and isinstance(node.func.value, ast.Name)
            and node.func.value.id == "self"
        )

    refresh_calls = [
        node
        for node in ast.walk(locale_method)
        if self_method_call(node, "_refresh_accessible_names")
    ]
    preview_calls = [
        node for node in ast.walk(locale_method) if self_method_call(node, "_refresh_preview")
    ]
    if len(refresh_calls) != 1 or len(preview_calls) != 1:
        violations.append(
            "settings_dialog.py: set_locale must call accessible-name refresh and "
            "preview exactly once"
        )
    elif refresh_calls[0].lineno >= preview_calls[0].lineno:
        violations.append(
            "settings_dialog.py: accessible-name refresh must precede preview projection"
        )
    localized_targets = (
        "self._language_label",
        "self._theme_label",
        "self._accent_label",
        "self._ui_font_label",
        "self._ui_font_size_label",
        "self._ui_font_style_label",
        "self._editor_font_label",
        "self._editor_font_size_label",
        "self._editor_font_style_label",
        "self._wrap_lines",
        "self._line_numbers",
        "self._motion",
    )
    set_text_lines = {target: [] for target in localized_targets}
    for node in ast.walk(locale_method):
        if not (
            isinstance(node, ast.Call)
            and len(node.args) == 1
            and not node.keywords
            and isinstance(node.func, ast.Attribute)
            and node.func.attr == "setText"
        ):
            continue
        target = self_attribute(node.func.value)
        if target in set_text_lines:
            set_text_lines[target].append(node.lineno)
    if refresh_calls:
        refresh_line = refresh_calls[0].lineno
        for target, lines in set_text_lines.items():
            if not lines or max(lines) >= refresh_line:
                violations.append(
                    f"settings_dialog.py: {target} must receive localized text before "
                    "accessible-name refresh"
                )
    return violations


def _audit_settings_restore_defaults_contract() -> list[str]:
    """Keep reset-to-defaults local to the unsaved Settings draft."""
    dialog_path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    theme_path = ROOT / "src" / "quillforge" / "presentation" / "theme.py"
    dialog = dialog_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    tree = _parse(dialog_path)
    settings_dialog = next(
        (
            node
            for node in tree.body
            if isinstance(node, ast.ClassDef) and node.name == "SettingsDialog"
        ),
        None,
    )
    methods = (
        {
            node.name: node
            for node in settings_dialog.body
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
        }
        if settings_dialog is not None
        else {}
    )
    restore_method = methods.get("_restore_defaults")
    locale_method = methods.get("set_locale")
    violations: list[str] = []
    for fragment in (
        "DEFAULT_SETTINGS,",
        "QSignalBlocker",
        "QDialogButtonBox.StandardButton.RestoreDefaults",
        'reset_button.setObjectName("resetSettingsAction")',
        "reset_button.clicked.connect(self._restore_defaults)",
        'self._reset_button.setText(tr("settings.restore_defaults", self._locale))',
        "self._reset_button.setToolTip(",
        "self._reset_button.setAccessibleName(",
    ):
        if fragment not in dialog:
            violations.append(
                f"settings_dialog.py: reset-defaults contract is missing {fragment!r}"
            )
    if restore_method is None or locale_method is None:
        violations.append("settings_dialog.py: reset-defaults or locale method is missing")
    else:
        restore_source = ast.get_source_segment(dialog, restore_method) or ""
        for fragment in (
            "defaults = DEFAULT_SETTINGS",
            "controls = (",
            "blockers = [QSignalBlocker(widget) for widget in controls]",
            "del blockers",
            "self._refresh_palette_icons()",
            "self.set_locale(normalize_locale(defaults.appearance.locale))",
        ):
            if fragment not in restore_source:
                violations.append(f"settings_dialog.py: _restore_defaults is missing {fragment!r}")
        for forbidden in ("self.accept(", "self.reject(", ".save("):
            if forbidden in restore_source:
                violations.append(
                    "settings_dialog.py: _restore_defaults must not persist or close "
                    f"via {forbidden!r}"
                )
        reset_line = next(
            (
                node.lineno
                for node in ast.walk(locale_method)
                if isinstance(node, ast.Call)
                and isinstance(node.func, ast.Attribute)
                and node.func.attr == "setText"
                and isinstance(node.func.value, ast.Attribute)
                and node.func.value.attr == "_reset_button"
            ),
            None,
        )
        accessibility_line = next(
            (
                node.lineno
                for node in ast.walk(locale_method)
                if isinstance(node, ast.Call)
                and isinstance(node.func, ast.Attribute)
                and node.func.attr == "_refresh_accessible_names"
            ),
            None,
        )
        if reset_line is None or accessibility_line is None or reset_line >= accessibility_line:
            violations.append(
                "settings_dialog.py: reset button localization must precede "
                "accessibility projection"
            )
    for fragment in (
        "QDialog#settingsDialog QPushButton#resetSettingsAction {{",
        "QDialog#settingsDialog QPushButton#resetSettingsAction:hover,",
        "QDialog#settingsDialog QPushButton#resetSettingsAction:focus {{",
        "QDialog#settingsDialog QPushButton#resetSettingsAction:pressed {{",
        "QDialog#settingsDialog QPushButton#resetSettingsAction:disabled {{",
        "border-left: 2px solid {colors.accent_gold};",
        "color: {warning_foreground};",
    ):
        if fragment not in theme:
            violations.append(f"theme.py: reset action QSS is missing {fragment!r}")
    return violations


def _audit_settings_draft_status_contract() -> list[str]:
    """Keep the unsaved Settings indicator local, explicit, and localized."""
    dialog_path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    theme_path = ROOT / "src" / "quillforge" / "presentation" / "theme.py"
    dialog = dialog_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    tree = _parse(dialog_path)
    settings_dialog = next(
        (
            node
            for node in tree.body
            if isinstance(node, ast.ClassDef) and node.name == "SettingsDialog"
        ),
        None,
    )
    methods = (
        {
            node.name: node
            for node in settings_dialog.body
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
        }
        if settings_dialog is not None
        else {}
    )
    status_method = methods.get("_refresh_draft_status")
    locale_method = methods.get("set_locale")
    preview_method = methods.get("_on_preview_changed")
    draft_method = methods.get("_on_draft_changed")
    violations: list[str] = []
    for fragment in (
        "self._draft_baseline = settings",
        'self._draft_status.setObjectName("settingsDraftStatus")',
        "content_layout.addWidget(self._draft_status)",
        "self._wrap_lines.toggled.connect(self._on_draft_changed)",
        "self._line_numbers.toggled.connect(self._on_draft_changed)",
        "self._motion.toggled.connect(self._on_draft_changed)",
    ):
        if fragment not in dialog:
            violations.append(f"settings_dialog.py: draft-status contract is missing {fragment!r}")
    if any(
        method is None for method in (status_method, locale_method, preview_method, draft_method)
    ):
        violations.append("settings_dialog.py: draft-status methods are incomplete")
    else:
        status_source = ast.get_source_segment(dialog, status_method) or ""
        for fragment in (
            "self.settings_snapshot() != self._draft_baseline",
            '"settings.draft.changed" if changed else "settings.draft.clean"',
            'self._draft_status.setProperty("draftState", state)',
            "self._draft_status.setAccessibleName(message)",
            "style.unpolish(self._draft_status)",
            "style.polish(self._draft_status)",
        ):
            if fragment not in status_source:
                violations.append(
                    f"settings_dialog.py: _refresh_draft_status is missing {fragment!r}"
                )
        for method, name in (
            (preview_method, "_on_preview_changed"),
            (draft_method, "_on_draft_changed"),
        ):
            if "self._refresh_draft_status()" not in (ast.get_source_segment(dialog, method) or ""):
                violations.append(f"settings_dialog.py: {name} must refresh draft status")
        locale_calls = [
            node
            for node in ast.walk(locale_method)
            if isinstance(node, ast.Call)
            and isinstance(node.func, ast.Attribute)
            and node.func.attr in {"_refresh_preview", "_refresh_draft_status"}
        ]
        by_name = {node.func.attr: node.lineno for node in locale_calls}
        if by_name.get("_refresh_preview", 0) >= by_name.get("_refresh_draft_status", 0):
            violations.append(
                "settings_dialog.py: draft status must refresh after locale preview projection"
            )
    for fragment in (
        "QLabel#settingsDraftStatus {{",
        'QLabel#settingsDraftStatus[draftState="changed"] {{',
        "border-left: 3px solid {colors.success};",
        "border-left-color: {colors.accent_gold};",
        "color: {warning_foreground};",
    ):
        if fragment not in theme:
            violations.append(f"theme.py: draft-status QSS is missing {fragment!r}")
    return violations


def _audit_settings_typography_role_contract() -> list[str]:
    """Keep font controls behind one semantic role with two visual tones."""
    dialog_path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    theme_path = ROOT / "src" / "quillforge" / "presentation" / "theme.py"
    dialog = dialog_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    controls = (
        ("settingsUiFont", "self._ui_font", "interface", "self._ui_font.addItems("),
        ("settingsUiFontSize", "self._ui_font_size", "interface", "self._ui_font_size.setRange("),
        (
            "settingsUiFontStyle",
            "self._ui_font_style",
            "interface",
            "self._add_font_style_options(self._ui_font_style)",
        ),
        ("settingsEditorFont", "self._editor_font", "editor", "self._editor_font.addItems("),
        (
            "settingsEditorFontSize",
            "self._editor_font_size",
            "editor",
            "self._editor_font_size.setRange(",
        ),
        (
            "settingsEditorFontStyle",
            "self._editor_font_style",
            "editor",
            "self._add_font_style_options(self._editor_font_style)",
        ),
    )
    violations: list[str] = []
    for object_name, instance, tone, setup_fragment in controls:
        object_position = dialog.find(f'setObjectName("{object_name}")')
        role_fragment = f'{instance}.setProperty("settingsRole", "typographyChoice")'
        tone_fragment = f'{instance}.setProperty("settingsTone", "{tone}")'
        role_position = dialog.find(role_fragment, object_position)
        tone_position = dialog.find(tone_fragment, role_position)
        setup_position = dialog.find(setup_fragment, tone_position)
        if min(object_position, role_position, tone_position, setup_position) < 0 or not (
            object_position < role_position < tone_position < setup_position
        ):
            violations.append(
                f"settings_dialog.py: {object_name} must declare typography role/tone "
                "before its value population/configuration"
            )
    for fragment in (
        'QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"],',
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"] {{',
        'QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"]:hover,',
        'QDialog#settingsDialog QSpinBox[settingsRole="typographyChoice"]:focus {{',
        'QDialog#settingsDialog QComboBox[settingsRole="typographyChoice"]:on {{',
        'QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="interface"] {{',
        'QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="editor"] {{',
        'QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="interface"]:on {{',
        "border-left-color: {typography_interface_edge};",
        'QDialog#settingsDialog [settingsRole="typographyChoice"][settingsTone="editor"]:on {{',
        "border-left-color: {typography_editor_edge};",
        'QDialog#settingsDialog [settingsRole="typographyChoice"]:disabled {{',
    ):
        if fragment not in theme:
            violations.append(f"theme.py: typography role QSS is missing {fragment!r}")
    for fragment in (
        "QDialog#settingsDialog QComboBox#settingsUiFont,",
        "QDialog#settingsDialog QComboBox#settingsUiFontStyle,",
        "QDialog#settingsDialog QSpinBox#settingsUiFontSize,",
        "QDialog#settingsDialog QComboBox#settingsEditorFont,",
        "QDialog#settingsDialog QComboBox#settingsEditorFontStyle,",
        "QDialog#settingsDialog QSpinBox#settingsEditorFontSize,",
    ):
        if fragment in theme:
            violations.append(f"theme.py: legacy typography selector remains {fragment!r}")
    combined_selector = (
        'QDialog#settingsDialog [settingsRole="typographyChoice"]'
        '[settingsTone="interface"]:on,\n'
        '    QDialog#settingsDialog [settingsRole="typographyChoice"]'
        '[settingsTone="editor"]:on'
    )
    if combined_selector in theme:
        violations.append("theme.py: typography tone-specific :on selectors are combined")
    return violations


def _audit_settings_behavior_toggle_role_contract() -> list[str]:
    """Keep Settings behavior toggles behind one semantic role and tone."""
    dialog_path = ROOT / "src" / "quillforge" / "presentation" / "settings_dialog.py"
    theme_path = ROOT / "src" / "quillforge" / "presentation" / "theme.py"
    dialog = dialog_path.read_text(encoding="utf-8")
    theme = theme_path.read_text(encoding="utf-8")
    controls = (
        ("settingsWrapLines", "self._wrap_lines", "editor"),
        ("settingsLineNumbers", "self._line_numbers", "editor"),
        ("settingsMotion", "self._motion", "interface"),
    )
    violations: list[str] = []
    for object_name, instance, tone in controls:
        object_position = dialog.find(f'setObjectName("{object_name}")')
        role_fragment = f'{instance}.setProperty("settingsRole", "behaviorToggle")'
        tone_fragment = f'{instance}.setProperty("settingsTone", "{tone}")'
        checked_fragment = f"{instance}.setChecked("
        role_position = dialog.find(role_fragment, object_position)
        tone_position = dialog.find(tone_fragment, role_position)
        checked_position = dialog.find(checked_fragment, tone_position)
        if min(object_position, role_position, tone_position, checked_position) < 0 or not (
            object_position < role_position < tone_position < checked_position
        ):
            violations.append(
                f"settings_dialog.py: {object_name} must declare behaviorToggle/{tone} "
                "before its checked value"
            )
    behavior_toggle_role_prefix = 'QDialog#settingsDialog [settingsRole="behaviorToggle"]'
    behavior_toggle_interface_disabled = (
        f'{behavior_toggle_role_prefix}[settingsTone="interface"]:disabled {{'
    )
    behavior_toggle_editor_disabled = (
        f'{behavior_toggle_role_prefix}[settingsTone="editor"]:disabled {{'
    )
    for fragment in (
        'QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"] {{',
        'QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:hover,',
        'QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:focus {{',
        'QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:checked {{',
        'QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:checked:focus {{',
        'QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="interface"] {{',
        'QDialog#settingsDialog [settingsRole="behaviorToggle"][settingsTone="editor"] {{',
        'QDialog#settingsDialog QCheckBox[settingsRole="behaviorToggle"]:disabled {{',
        behavior_toggle_interface_disabled,
        behavior_toggle_editor_disabled,
        "behavior_interface_edge",
        "behavior_editor_edge",
    ):
        if fragment not in theme:
            violations.append(f"theme.py: behaviorToggle QSS is missing {fragment!r}")
    for fragment in (
        "QDialog#settingsDialog QCheckBox#settingsWrapLines,",
        "QDialog#settingsDialog QCheckBox#settingsLineNumbers,",
        "QDialog#settingsDialog QCheckBox#settingsMotion,",
        "QDialog#settingsDialog QCheckBox#settingsWrapLines:hover,",
        "QDialog#settingsDialog QCheckBox#settingsLineNumbers:hover,",
        "QDialog#settingsDialog QCheckBox#settingsMotion:hover,",
        "QDialog#settingsDialog QCheckBox#settingsWrapLines:checked,",
        "QDialog#settingsDialog QCheckBox#settingsLineNumbers:checked,",
        "QDialog#settingsDialog QCheckBox#settingsMotion:checked,",
    ):
        if fragment in theme:
            violations.append(f"theme.py: legacy behavior-toggle selector remains {fragment!r}")
    return violations


def _audit_startup_surface_contract() -> list[str]:
    """Keep the command surface ready before MainWindow asks it to build menus."""
    command_surface_path = PRESENTATION_ROOT / "command_surface.py"
    main_window_path = PRESENTATION_ROOT / "main_window.py"
    command_surface = command_surface_path.read_text(encoding="utf-8")
    main_window = main_window_path.read_text(encoding="utf-8")
    violations: list[str] = []
    for fragment in (
        "self._locale_provider = locale_provider",
        "def _locale(self) -> Locale:",
        "return self._locale_provider()",
        "self.refresh()",
    ):
        if fragment not in command_surface:
            violations.append(
                f"{command_surface_path.relative_to(ROOT).as_posix()}: startup locale contract "
                f"is missing {fragment!r}"
            )

    ordered_fragments = (
        "self._command_surface = CommandSurface(self, self._commands, lambda: self._locale)",
        "self._create_menus()",
        "self._command_surface.create_toolbar(",
        "self._locale_coordinator = PresentationLocaleCoordinator(",
    )
    positions = [main_window.find(fragment) for fragment in ordered_fragments]
    if any(position < 0 for position in positions):
        missing = [
            fragment
            for fragment, position in zip(ordered_fragments, positions, strict=True)
            if position < 0
        ]
        violations.append(
            f"{main_window_path.relative_to(ROOT).as_posix()}: startup ordering contract is "
            f"missing {missing!r}"
        )
    elif positions != sorted(positions):
        violations.append(
            f"{main_window_path.relative_to(ROOT).as_posix()}: command-surface startup "
            f"ordering changed: {positions!r}"
        )
    return violations


def _audit_main_window_startup_state_contract() -> list[str]:
    """Keep callback-visible MainWindow state initialized before coordinator wiring."""
    path = PRESENTATION_ROOT / "main_window.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    tree = _parse(path)
    main_window = next(
        (
            node
            for node in tree.body
            if isinstance(node, ast.ClassDef) and node.name == "MainWindow"
        ),
        None,
    )
    init = (
        next(
            (
                node
                for node in main_window.body
                if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
                and node.name == "__init__"
            ),
            None,
        )
        if main_window is not None
        else None
    )
    default_assignments = []
    if init is not None:
        for node in ast.walk(init):
            if not isinstance(node, ast.Assign):
                continue
            if not isinstance(node.value, ast.Constant) or node.value.value is not False:
                continue
            if any(
                isinstance(target, ast.Attribute)
                and isinstance(target.value, ast.Name)
                and target.value.id == "self"
                and target.attr == "_busy"
                for target in node.targets
            ):
                default_assignments.append(node.lineno)
    state_assignment = "self._busy = False"
    busy_callback = "is_busy=lambda: self._busy"
    if len(default_assignments) != 1:
        violations.append(
            f"{relative}: __init__ must have exactly one _busy=False default assignment"
        )
    assignment_position = source.find(state_assignment)
    callback_position = source.find(busy_callback)
    if assignment_position < 0 or callback_position < 0:
        violations.append(
            f"{relative}: startup busy-state initialization/callback contract is missing"
        )
    elif assignment_position > callback_position:
        violations.append(
            f"{relative}: _busy must be initialized before coordinator callbacks capture it"
        )
    startup_anchor = "self._startup_restore_inflight = False"
    if (
        startup_anchor not in source
        or assignment_position < 0
        or source.find(startup_anchor) > assignment_position
    ):
        violations.append(
            f"{relative}: _busy default must be established at the startup state boundary"
        )
    return violations


def _audit_startup_failure_log_lifecycle_contract() -> list[str]:
    """Keep stale app-owned startup diagnostics from surviving a new attempt."""
    path = ROOT / "src" / "quillforge" / "__main__.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    main_position = source.find("def main(")
    clear_call = source.find("    _clear_previous_startup_failure()", main_position)
    dispatch = source.find("    if __package__:", main_position)
    if main_position < 0 or clear_call < 0 or dispatch < 0 or clear_call > dispatch:
        violations.append(f"{relative}: main() must clear the prior startup log before dispatch")
    for fragment in (
        "def _clear_previous_startup_failure() -> None:",
        "_startup_error_path().unlink(missing_ok=True)",
        "except Exception:",
        "def _record_startup_failure(error: Exception) -> Path | None:",
    ):
        if fragment not in source:
            violations.append(f"{relative}: startup-log lifecycle contract is missing {fragment!r}")
    return violations


def _audit_startup_runtime_composition_contract() -> list[str]:
    """Keep startup diagnostics deep enough to catch constructor-time failures."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    composition_probe = 'probe("runtime_composition", _startup_runtime_composition)'
    import_probe = '"composition_import"'
    if composition_probe not in source:
        violations.append(f"{relative}: startup diagnostic must probe runtime composition")
    if source.find(import_probe) >= source.find(composition_probe) >= 0:
        violations.append(
            f"{relative}: runtime composition probe must follow the composition import probe"
        )
    for fragment in (
        "def _startup_runtime_composition() -> dict[str, object]:",
        "application = QApplication.instance()",
        "runtime = build_desktop_runtime(application)",
        "runtime.prepare_startup()",
        "runtime.refresh_startup_commands()",
        "runtime.preflight_editor_shell()",
        "runtime.stop()",
        "application.quit()",
        '"startup_prepared": True',
        '"editor_shell_prepared": True',
        '"window_shown": False',
        '"event_loop_entered": False',
    ):
        if fragment not in source:
            violations.append(f"{relative}: runtime composition contract is missing {fragment!r}")
    runtime_start = source.find("def _startup_runtime_composition(")
    runtime_end = source.find("\ndef _startup_settings_preflight(", runtime_start)
    runtime_source = source[runtime_start:runtime_end] if runtime_start >= 0 else ""
    if "application.exec(" in runtime_source or "runtime.window.show(" in runtime_source:
        violations.append(
            f"{relative}: runtime composition probe must not enter Qt or show a window"
        )
    composition_path = ROOT / "src" / "quillforge" / "composition.py"
    composition_source = composition_path.read_text(encoding="utf-8")
    if "def prepare_startup(self) -> None:" not in composition_source:
        violations.append("composition.py: shared startup preparation method is missing")
    if "self.prepare_startup()" not in composition_source:
        violations.append("composition.py: normal start() must reuse startup preparation")
    if "def refresh_startup_commands(self) -> None:" not in composition_source:
        violations.append("composition.py: shared startup command refresh method is missing")
    if "self.refresh_startup_commands()" not in composition_source:
        violations.append("composition.py: normal start() must reuse startup command refresh")
    if "def preflight_editor_shell(self) -> None:" not in composition_source:
        violations.append("composition.py: editor-shell preflight method is missing")
    main_window_path = ROOT / "src" / "quillforge" / "presentation" / "main_window.py"
    main_window_source = main_window_path.read_text(encoding="utf-8")
    for fragment in (
        "def preflight_editor_shell(self) -> None:",
        "self.ensure_initial_document()",
        "self._stop_close_timers()",
    ):
        if fragment not in main_window_source:
            violations.append(
                f"main_window.py: editor-shell preflight cleanup contract is missing {fragment!r}"
            )
    start_position = composition_source.find("    def start(self) -> None:")
    stop_position = composition_source.find("    def stop(self) -> None:", start_position)
    start_source = (
        composition_source[start_position:stop_position]
        if start_position >= 0 and stop_position > start_position
        else ""
    )
    startup_order = (
        "self.prepare_startup()",
        "self.window.restore_startup_state()",
        "self.refresh_startup_commands()",
        "self.window.show()",
        "self.window.open_startup_paths(self.startup_paths)",
    )
    positions = [start_source.find(fragment) for fragment in startup_order]
    if any(position < 0 for position in positions) or positions != sorted(positions):
        violations.append(
            "composition.py: normal start() startup preparation order must be preserved"
        )
    return violations


def _audit_entrypoint_runtime_cleanup_contract() -> list[str]:
    """Keep normal startup inside the same cleanup boundary as the event loop."""
    path = ROOT / "src" / "quillforge" / "app.py"
    relative = path.relative_to(ROOT).as_posix()
    tree = _parse(path)
    main = next(
        (node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == "main"),
        None,
    )
    if main is None:
        return [f"{relative}: main() entrypoint is missing"]

    def contains_call(nodes: list[ast.stmt], expression: str) -> bool:
        return any(
            isinstance(child, ast.Call) and ast.unparse(child) == expression
            for statement in nodes
            for child in ast.walk(statement)
        )

    runtime_starts = [
        node
        for node in ast.walk(main)
        if isinstance(node, ast.Call) and ast.unparse(node) == "runtime.start()"
    ]
    cleanup_boundaries = [
        node
        for node in ast.walk(main)
        if isinstance(node, ast.Try)
        and contains_call(node.body, "runtime.start()")
        and contains_call(node.body, "application.exec()")
        and contains_call(node.finalbody, "runtime.stop()")
    ]
    violations: list[str] = []
    if len(runtime_starts) != 1:
        violations.append(
            f"{relative}: main() must have exactly one runtime.start() call inside cleanup"
        )
    if not cleanup_boundaries:
        violations.append(
            f"{relative}: runtime.start(), application.exec(), and runtime.stop() "
            "must share one try/finally cleanup boundary"
        )
    return violations


def _audit_runtime_shutdown_contract() -> list[str]:
    """Keep composition shutdown ordered and delegated through a presentation port."""
    composition_path = ROOT / "src" / "quillforge" / "composition.py"
    main_window_path = ROOT / "src" / "quillforge" / "presentation" / "main_window.py"
    violations: list[str] = []
    composition_tree = _parse(composition_path)
    runtime_class = next(
        (
            node
            for node in composition_tree.body
            if isinstance(node, ast.ClassDef) and node.name == "DesktopRuntime"
        ),
        None,
    )
    stop_method = (
        next(
            (
                node
                for node in runtime_class.body
                if isinstance(node, ast.FunctionDef) and node.name == "stop"
            ),
            None,
        )
        if runtime_class is not None
        else None
    )
    stop_calls = (
        [ast.unparse(node) for node in ast.walk(stop_method) if isinstance(node, ast.Call)]
        if stop_method is not None
        else []
    )
    try:
        activity_position = stop_calls.index("self.window.stop_background_activity()")
        plugin_position = stop_calls.index("self.plugins.deactivate_all()")
    except ValueError:
        violations.append(
            "composition.py: DesktopRuntime.stop() must stop UI activity before plugin deactivation"
        )
    else:
        if activity_position >= plugin_position:
            violations.append(
                "composition.py: DesktopRuntime.stop() shutdown order must stop UI activity first"
            )

    main_window_source = main_window_path.read_text(encoding="utf-8")
    for fragment in (
        "def stop_background_activity(self) -> None:",
        "self._stop_close_timers()",
    ):
        if fragment not in main_window_source:
            violations.append(f"main_window.py: shutdown port is missing {fragment!r}")
    return violations


def _audit_startup_diagnostic_qt_lifecycle_contract() -> list[str]:
    """Keep all startup probes inside one diagnostic-owned QApplication lifetime."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    start = source.find("def _run_startup_diagnostic(")
    end = source.find("\ndef _startup_runtime_composition(", start)
    scope = source[start:end] if start >= 0 else ""
    violations: list[str] = []
    for fragment in (
        "diagnostic_application = None",
        "owns_diagnostic_application = False",
        "diagnostic_application = QApplication.instance()",
        "diagnostic_application = QApplication([])",
        "if owns_diagnostic_application and diagnostic_application is not None:",
        "diagnostic_application.quit()",
    ):
        if fragment not in scope:
            violations.append(f"{relative}: startup diagnostic Qt scope is missing {fragment!r}")
    probe_start = scope.find('probe("runtime_composition", _startup_runtime_composition)')
    probe_end = scope.find('probe("startup_restore_preflight", _startup_restore_preflight)')
    application_start = scope.find("diagnostic_application = QApplication.instance()")
    quit_start = scope.find("diagnostic_application.quit()")
    if (
        min(probe_start, probe_end, application_start, quit_start) < 0
        or application_start > probe_start
        or quit_start < probe_end
    ):
        violations.append(
            f"{relative}: one diagnostic QApplication must cover runtime/restore "
            "probes and close after them"
        )
    return violations


def _audit_frozen_qt_plugin_contract() -> list[str]:
    """Keep frozen Qt plugin discovery bound to the extracted application bundle."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    tree = _parse(path)
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []

    functions = {
        node.name: node
        for node in tree.body
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
    }
    main_node = functions.get("main")
    configure_node = functions.get("_configure_frozen_qt_plugins")
    root_node = functions.get("_frozen_qt_plugin_root")
    startup_path_node = functions.get("_startup_qt_plugin_path")

    def function_scope(node: ast.AST | None) -> str:
        return ast.get_source_segment(source, node) if node is not None else ""

    root_scope = function_scope(root_node)
    configure_scope = function_scope(configure_node)
    startup_path_scope = function_scope(startup_path_node)
    for fragment, scope in (
        ("def _configure_frozen_qt_plugins() -> None:", configure_scope),
        ("def _frozen_qt_plugin_root(bundle_root: Path) -> Path:", root_scope),
        ('bundle_root / "PyQt6" / "Qt6" / "plugins"', root_scope),
        ('bundle_root / "PyQt6" / "Qt" / "plugins"', root_scope),
        ('(candidate / "platforms" / "qwindows.dll").is_file()', root_scope),
        ('os.environ["QT_PLUGIN_PATH"] = str(plugin_root)', configure_scope),
        ('os.environ["QT_QPA_PLATFORM_PLUGIN_PATH"] = str(platform_root)', configure_scope),
    ):
        if fragment not in scope:
            violations.append(f"{relative}: frozen Qt plugin contract is missing {fragment!r}")

    qt6_root_position = root_scope.find('bundle_root / "PyQt6" / "Qt6" / "plugins"')
    legacy_root_position = root_scope.find('bundle_root / "PyQt6" / "Qt" / "plugins"')
    file_probe_position = root_scope.find('(candidate / "platforms" / "qwindows.dll").is_file()')
    directory_fallback_position = root_scope.find("candidate.is_dir()")
    if not (
        0 <= qt6_root_position < legacy_root_position
        and 0 <= file_probe_position < directory_fallback_position
    ):
        violations.append(
            f"{relative}: frozen Qt plugin selector must keep Qt6→Qt order and "
            "prefer qwindows.is_file() before directory fallback"
        )

    for scope_name, scope in (
        ("configuration", configure_scope),
        ("diagnostic", startup_path_scope),
    ):
        if "_frozen_qt_plugin_root(" not in scope:
            violations.append(
                f"{relative}: frozen Qt {scope_name} path must reuse _frozen_qt_plugin_root()"
            )

    main_calls = (
        [
            node
            for node in ast.walk(main_node)
            if isinstance(node, ast.Call)
            and isinstance(node.func, ast.Name)
            and node.func.id == "_configure_frozen_qt_plugins"
        ]
        if main_node is not None
        else []
    )
    qt_imports = (
        [
            node
            for node in ast.walk(main_node)
            if isinstance(node, ast.ImportFrom)
            and node.module == "PyQt6.QtWidgets"
            and any(alias.name == "QApplication" for alias in node.names)
        ]
        if main_node is not None
        else []
    )
    if not main_calls or not qt_imports or main_calls[0].lineno >= qt_imports[0].lineno:
        violations.append(
            f"{relative}: frozen Qt plugin configuration must precede QApplication import"
        )
    return violations


def _audit_frozen_qt_runtime_preflight_contract() -> list[str]:
    """Keep normal frozen GUI startup fail-fast and leave diagnostics untouched."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    main_start = source.find("def main(")
    helper_start = source.find("\ndef _configure_frozen_qt_plugins(", main_start)
    scope = source[main_start:helper_start] if main_start >= 0 and helper_start >= 0 else ""
    violations: list[str] = []
    for fragment in (
        "_validate_frozen_qt_runtime()",
        "def _validate_frozen_qt_runtime() -> None:",
        "plugin_check = _startup_qt_plugin_path(bundle_root)",
        "dependency_check = _startup_qt_runtime_dependencies(bundle_root)",
        "Frozen Qt runtime is incomplete; missing bundle files:",
    ):
        if fragment not in source:
            violations.append(f"{relative}: frozen Qt runtime preflight is missing {fragment!r}")
    validation_position = scope.find("_validate_frozen_qt_runtime()")
    qt_import_position = scope.find("from PyQt6.QtWidgets import QApplication")
    if (
        validation_position < 0
        or qt_import_position < 0
        or validation_position > qt_import_position
    ):
        violations.append(
            f"{relative}: frozen Qt runtime preflight must precede normal QApplication import"
        )
    branch_returns = (
        "return run_plugin_host(arguments)",
        "return run_capture_diagnostic(arguments[diagnostic_start:])",
        "return _run_workspace_search_diagnostic(arguments[diagnostic_start:])",
        "return _run_file_open_diagnostic(arguments[diagnostic_start:])",
        "return _run_startup_diagnostic(arguments[diagnostic_start:])",
    )
    branch_positions = [scope.find(fragment) for fragment in branch_returns]
    if any(position < 0 for position in branch_positions) or validation_position <= max(
        branch_positions
    ):
        violations.append(
            f"{relative}: frozen Qt runtime preflight must run only after "
            "plugin/diagnostic branches"
        )
    return violations


def _audit_frozen_qt_runtime_layout_contract() -> list[str]:
    """Keep frozen Qt DLL discovery aligned with both PyQt6 package layouts."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    for fragment in (
        "dependency_roots = (",
        'Path("PyQt6") / "Qt6" / "bin"',
        'Path("PyQt6") / "Qt" / "bin"',
        "for dependency_root in dependency_roots:",
        "if not missing:",
        "candidates.append((required, missing))",
    ):
        if fragment not in source:
            violations.append(f"{relative}: Qt layout fallback is missing {fragment!r}")
    return violations


def _audit_safe_mode_startup_contract() -> list[str]:
    """Keep the user-invoked safe startup path isolated from normal restore."""
    app_path = ROOT / "src" / "quillforge" / "app.py"
    composition_path = ROOT / "src" / "quillforge" / "composition.py"
    app_source = app_path.read_text(encoding="utf-8")
    composition_source = composition_path.read_text(encoding="utf-8")
    violations: list[str] = []
    app_relative = app_path.relative_to(ROOT).as_posix()
    composition_relative = composition_path.relative_to(ROOT).as_posix()
    for fragment in (
        'safe_mode = "--safe-mode" in arguments',
        'arguments = [argument for argument in arguments if argument != "--safe-mode"]',
        "safe_mode=safe_mode",
    ):
        if fragment not in app_source:
            violations.append(f"{app_relative}: safe-mode entry contract is missing {fragment!r}")
    for fragment in (
        "safe_mode: bool = False",
        "initial_settings = DEFAULT_SETTINGS if safe_mode else settings.load()",
        "if self.safe_mode:",
        "self.window.ensure_initial_document()",
        "self.prepare_startup()",
        "self.window.restore_startup_state()",
    ):
        if fragment not in composition_source:
            violations.append(
                f"{composition_relative}: safe-mode runtime contract is missing {fragment!r}"
            )
    start_position = composition_source.find("def start(self) -> None:")
    safe_branch = composition_source.find("if self.safe_mode:", start_position)
    normal_restore = composition_source.find("self.window.restore_startup_state()", start_position)
    if start_position < 0 or safe_branch < 0 or normal_restore < 0 or safe_branch > normal_restore:
        violations.append(
            f"{composition_relative}: safe mode must branch before normal session restore"
        )
    return violations


def _audit_startup_state_preflight_contract() -> list[str]:
    """Keep startup-state diagnosis read-only and aligned with production stores."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    probes = (
        'probe("settings_preflight", _startup_settings_preflight)',
        'probe("session_preflight", _startup_session_preflight)',
        'probe("recovery_preflight", _startup_recovery_preflight)',
    )
    positions = [source.find(fragment) for fragment in probes]
    if any(position < 0 for position in positions):
        missing = [
            fragment for fragment, position in zip(probes, positions, strict=True) if position < 0
        ]
        violations.append(f"{relative}: startup-state probes are missing {missing!r}")
    elif positions != sorted(positions):
        violations.append(f"{relative}: startup-state probe ordering changed: {positions!r}")

    session_start = source.find("def _startup_session_preflight(")
    recovery_start = source.find("def _startup_recovery_preflight(")
    restore_start = source.find("def _startup_restore_preflight(")
    plugin_start = source.find("def _startup_qt_plugin_path(")
    session_source = source[session_start:recovery_start] if session_start >= 0 else ""
    recovery_end = restore_start if restore_start >= 0 else plugin_start
    recovery_source = source[recovery_start:recovery_end] if recovery_start >= 0 else ""
    for fragment in (
        "SessionService(JsonSessionStore(path)).load()",
        '"load_state": result.state',
        '"document_count": len(documents)',
        '"missing_document_count": sum(',
        '"workspace_root_present":',
    ):
        if fragment not in session_source:
            violations.append(f"{relative}: session preflight contract is missing {fragment!r}")
    for fragment in (
        "JsonRecoverySnapshotStore(root).list_snapshots()",
        '"snapshot_file_count": snapshot_file_count',
        '"valid_snapshot_count": valid_snapshot_count',
        '"invalid_snapshot_count": max(0, snapshot_file_count - valid_snapshot_count)',
    ):
        if fragment not in recovery_source:
            violations.append(f"{relative}: recovery preflight contract is missing {fragment!r}")
    for name, helper_source in (
        ("session", session_source),
        ("recovery", recovery_source),
    ):
        for forbidden in ("write_text(", ".save(", "QApplication", "application.exec("):
            if forbidden in helper_source:
                violations.append(
                    f"{relative}: {name} preflight must remain non-destructive; found {forbidden!r}"
                )
    if '"document_paths"' in session_source or '"snapshot_text"' in recovery_source:
        violations.append(
            f"{relative}: startup-state preflight must not expose user content or path lists"
        )
    return violations


def _audit_startup_restore_preflight_contract() -> list[str]:
    """Keep asynchronous restore diagnosis bounded, no-window, and non-modal."""
    path = ROOT / "src" / "quillforge" / "app.py"
    source = path.read_text(encoding="utf-8")
    relative = path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    for fragment in (
        'probe("startup_restore_preflight", _startup_restore_preflight)',
        "def _startup_restore_preflight() -> dict[str, object]:",
        "JsonRecoverySnapshotStore(recovery_root).list_snapshots()",
        '"restore_stage": "skipped_recovery_candidates"',
        "runtime.preflight_startup_restore()",
        '"restore_stage": "executed"',
    ):
        if fragment not in source:
            violations.append(
                f"{relative}: startup restore preflight contract is missing {fragment!r}"
            )
    helper_start = source.find("def _startup_restore_preflight(")
    helper_end = source.find("\ndef _startup_qt_plugin_path(", helper_start)
    helper_source = source[helper_start:helper_end] if helper_start >= 0 else ""
    if "application.exec(" in helper_source or "window.show(" in helper_source:
        violations.append(
            f"{relative}: startup restore preflight must not enter Qt or show a window"
        )
    composition_path = ROOT / "src" / "quillforge" / "composition.py"
    composition_source = composition_path.read_text(encoding="utf-8")
    if "def preflight_startup_restore(self) -> dict[str, object]:" not in composition_source:
        violations.append("composition.py: startup restore preflight wrapper is missing")
    main_window_path = ROOT / "src" / "quillforge" / "presentation" / "main_window.py"
    main_window_source = main_window_path.read_text(encoding="utf-8")
    for fragment in (
        (
            "def preflight_startup_restore(self, *, timeout_seconds: float = 5.0) "
            "-> dict[str, object]:"
        ),
        "self.restore_startup_state()",
        "application.processEvents(",
        "QEventLoop.ProcessEventsFlag.WaitForMoreEvents",
        "wake_timer.setInterval(2)",
        "self._runner.has_pending_work()",
        "wake_timer.stop()",
        "self._stop_close_timers()",
        '"window_shown": False',
        '"event_loop_entered": False',
    ):
        if fragment not in main_window_source:
            violations.append(
                f"main_window.py: startup restore preflight contract is missing {fragment!r}"
            )
    if "application.exec(" in main_window_source or "self.show()" in main_window_source:
        violations.append("main_window.py: startup restore preflight must not enter exec or show")

    tree = _parse(main_window_path)
    main_window = next(
        (
            node
            for node in tree.body
            if isinstance(node, ast.ClassDef) and node.name == "MainWindow"
        ),
        None,
    )
    wait_method = next(
        (
            node
            for node in (main_window.body if main_window is not None else ())
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
            and node.name == "_wait_for_startup_preflight"
        ),
        None,
    )
    if wait_method is None:
        violations.append("main_window.py: bounded startup wait method is missing")
        return violations

    call_source = {
        ast.unparse(node) for node in ast.walk(wait_method) if isinstance(node, ast.Call)
    }
    for fragment in (
        "QTimer()",
        "wake_timer.setInterval(2)",
        "wake_timer.start()",
        "wake_timer.stop()",
        "time.monotonic()",
        "TimeoutError('Startup preflight timed out')",
    ):
        if fragment not in call_source:
            violations.append(f"main_window.py: bounded startup wait AST is missing {fragment!r}")
    if not any(
        "application.processEvents(QEventLoop.ProcessEventsFlag.AllEvents | "
        "QEventLoop.ProcessEventsFlag.WaitForMoreEvents)" in call
        for call in call_source
    ):
        violations.append(
            "main_window.py: bounded startup wait AST must process AllEvents and WaitForMoreEvents"
        )
    try_nodes = [node for node in ast.walk(wait_method) if isinstance(node, ast.Try)]
    if not any(
        any(
            isinstance(node, ast.Expr)
            and isinstance(node.value, ast.Call)
            and ast.unparse(node.value) == "wake_timer.stop()"
            for node in try_node.finalbody
        )
        for try_node in try_nodes
    ):
        violations.append(
            "main_window.py: bounded startup wait AST must stop the wake timer in finally"
        )
    report_keys = {
        key.value
        for return_node in ast.walk(wait_method)
        if isinstance(return_node, ast.Return) and isinstance(return_node.value, ast.Dict)
        for key in return_node.value.keys
        if isinstance(key, ast.Constant) and isinstance(key.value, str)
    }
    for key in (
        "startup_restore_completed",
        "pending_work",
        "pending_startup_paths",
        "tab_count",
        "processed_event_batches",
        "timeout_mode",
        "window_shown",
        "event_loop_entered",
    ):
        if key not in report_keys:
            violations.append(f"main_window.py: bounded startup wait report is missing {key!r}")
    return violations


def _audit_file_open_diagnostic_contract() -> list[str]:
    """Keep explicit file-open diagnosis on the existing startup-path boundary."""
    app_path = ROOT / "src" / "quillforge" / "app.py"
    app_source = app_path.read_text(encoding="utf-8")
    relative = app_path.relative_to(ROOT).as_posix()
    violations: list[str] = []
    for fragment in (
        'if "--diagnose-file-open" in arguments:',
        "return _run_file_open_diagnostic(arguments[diagnostic_start:])",
        "def _run_file_open_diagnostic(arguments: list[str]) -> int:",
        'parser.add_argument("path")',
        'parser.add_argument("--report", required=True)',
        "JsonRecoverySnapshotStore(recovery_root).list_snapshots()",
        "runtime.preflight_startup_paths((requested_path,))",
        '"diagnostic": "file-open"',
    ):
        if fragment not in app_source:
            violations.append(f"{relative}: file-open diagnostic contract is missing {fragment!r}")
    helper_start = app_source.find("def _run_file_open_diagnostic(")
    helper_end = app_source.find("\ndef _run_startup_diagnostic(", helper_start)
    helper_source = app_source[helper_start:helper_end] if helper_start >= 0 else ""
    if "application.exec(" in helper_source or "window.show(" in helper_source:
        violations.append(f"{relative}: file-open diagnostic must not enter Qt or show a window")

    composition_path = ROOT / "src" / "quillforge" / "composition.py"
    composition_source = composition_path.read_text(encoding="utf-8")
    for fragment in (
        "def preflight_startup_paths(self, paths: Sequence[Path]) -> dict[str, object]:",
        "self.window.restore_startup_state()",
        "self.refresh_startup_commands()",
        "self.window.preflight_open_startup_paths(paths)",
    ):
        if fragment not in composition_source:
            violations.append(
                f"composition.py: file-open diagnostic contract is missing {fragment!r}"
            )

    main_window_path = ROOT / "src" / "quillforge" / "presentation" / "main_window.py"
    main_window_source = main_window_path.read_text(encoding="utf-8")
    for fragment in (
        "def preflight_open_startup_paths(",
        "self.open_startup_paths(requested_paths)",
        "self._tab_surface.find_by_path(path) is not None",
        "self._wait_for_startup_preflight(timeout_seconds)",
        "self._stop_close_timers()",
    ):
        if fragment not in main_window_source:
            violations.append(
                f"main_window.py: file-open diagnostic contract is missing {fragment!r}"
            )
    method_start = composition_source.find("    def preflight_startup_paths(")
    method_end = composition_source.find("    def start(", method_start)
    method_source = composition_source[method_start:method_end] if method_start >= 0 else ""
    if "self.window.show(" in method_source or "application.exec(" in method_source:
        violations.append("composition.py: file-open preflight must not show or enter exec")
    return violations


def _class_attribute_names(class_node: ast.ClassDef) -> set[str]:
    """Collect class-level assignments and annotated fields visible through ``self``."""
    attributes: set[str] = set()
    for node in class_node.body:
        if isinstance(node, ast.Assign):
            targets = node.targets
        elif isinstance(node, (ast.AnnAssign, ast.AugAssign)):
            targets = [node.target]
        else:
            continue
        attributes.update(target.id for target in targets if isinstance(target, ast.Name))
    return attributes


def _audit_private_self_calls() -> list[str]:
    """Catch direct private self-call typos before presentation startup."""
    violations: list[str] = []
    for path in sorted(PRESENTATION_ROOT.glob("*.py")):
        tree = _parse(path)
        relative = path.relative_to(ROOT).as_posix()
        for class_node in (node for node in tree.body if isinstance(node, ast.ClassDef)):
            methods = [
                node
                for node in class_node.body
                if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))
            ]
            method_names = {node.name for node in methods}
            known_attributes = _class_attribute_names(class_node)
            private_calls: list[ast.Call] = []
            for method in methods:
                for node in ast.walk(method):
                    if isinstance(node, (ast.Assign, ast.AnnAssign, ast.AugAssign)):
                        targets = node.targets if isinstance(node, ast.Assign) else [node.target]
                        for target in targets:
                            if (
                                isinstance(target, ast.Attribute)
                                and isinstance(target.value, ast.Name)
                                and target.value.id == "self"
                            ):
                                known_attributes.add(target.attr)
                    if (
                        isinstance(node, ast.Call)
                        and isinstance(node.func, ast.Attribute)
                        and isinstance(node.func.value, ast.Name)
                        and node.func.value.id == "self"
                        and node.func.attr.startswith("_")
                    ):
                        private_calls.append(node)
            for call in private_calls:
                attribute = call.func.attr
                if attribute in method_names or attribute in known_attributes:
                    continue
                violations.append(
                    f"{relative}:{call.lineno}: {class_node.name}.{attribute}() "
                    "is neither a class method nor an assigned callable attribute"
                )
    return violations


def _audit_coordinator_dependencies() -> list[str]:
    violations: list[str] = []
    forbidden_tokens = ("pyqt6", "task_runner", "main_window", "status_surface")
    for path in sorted(PRESENTATION_ROOT.glob("*_coordinator.py")):
        for module in _imported_modules(_parse(path)):
            if any(token in module.lower() for token in forbidden_tokens):
                relative = path.relative_to(ROOT).as_posix()
                violations.append(f"{relative}: forbidden coordinator import {module!r}")
    return violations


def _is_true_keyword(node: ast.Call, name: str) -> bool:
    return any(
        keyword.arg == name
        and isinstance(keyword.value, ast.Constant)
        and keyword.value.value is True
        for keyword in node.keywords
    )


def _audit_ports_contracts() -> list[str]:
    violations: list[str] = []
    for path in sorted(PRESENTATION_ROOT.glob("*_coordinator.py")):
        tree = _parse(path)
        relative = path.relative_to(ROOT).as_posix()
        for node in tree.body:
            if not isinstance(node, ast.ClassDef) or not node.name.endswith("Ports"):
                continue
            dataclass_decorators = [
                decorator
                for decorator in node.decorator_list
                if isinstance(decorator, ast.Call)
                and isinstance(decorator.func, ast.Name)
                and decorator.func.id == "dataclass"
            ]
            if not dataclass_decorators:
                violations.append(f"{relative}:{node.lineno}: {node.name} needs @dataclass")
                continue
            decorator = dataclass_decorators[0]
            if not _is_true_keyword(decorator, "frozen"):
                violations.append(f"{relative}:{node.lineno}: {node.name} needs frozen=True")
            if not _is_true_keyword(decorator, "slots"):
                violations.append(f"{relative}:{node.lineno}: {node.name} needs slots=True")
    return violations


def _audit_coordinator_notification_levels() -> list[str]:
    violations: list[str] = []
    for path in sorted(PRESENTATION_ROOT.glob("*_coordinator.py")):
        tree = _parse(path)
        relative = path.relative_to(ROOT).as_posix()
        for node in ast.walk(tree):
            if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Attribute):
                continue
            receiver = node.func.value
            if not (
                node.func.attr == "notify"
                and isinstance(receiver, ast.Attribute)
                and receiver.attr == "_ports"
                and isinstance(receiver.value, ast.Name)
                and receiver.value.id == "self"
            ):
                continue
            if not any(keyword.arg == "level" for keyword in node.keywords):
                violations.append(f"{relative}:{node.lineno}: self._ports.notify() needs level=")
    return violations


def _audit_notification_levels() -> list[str]:
    path = PRESENTATION_ROOT / "main_window.py"
    violations: list[str] = []
    for node in ast.walk(_parse(path)):
        if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Attribute):
            continue
        if node.func.attr != "notify":
            continue
        if not isinstance(node.func.value, ast.Name) or node.func.value.id != "self":
            continue
        if not any(keyword.arg == "level" for keyword in node.keywords):
            violations.append(
                f"{path.relative_to(ROOT).as_posix()}:{node.lineno}: notify() needs level="
            )
    return violations


def _class_method_names(tree: ast.Module, class_name: str) -> set[str]:
    for node in tree.body:
        if isinstance(node, ast.ClassDef) and node.name == class_name:
            return {
                item.name
                for item in node.body
                if isinstance(item, (ast.FunctionDef, ast.AsyncFunctionDef))
            }
    return set()


def _audit_task_runner_observability() -> list[str]:
    runner_path = PRESENTATION_ROOT / "task_runner.py"
    main_path = PRESENTATION_ROOT / "main_window.py"
    runner_source = runner_path.read_text(encoding="utf-8")
    main_source = main_path.read_text(encoding="utf-8")
    runner_methods = _class_method_names(_parse(runner_path), "TaskRunner")
    violations: list[str] = []
    for method in ("pending_count", "has_pending_work", "submit"):
        if method not in runner_methods:
            violations.append(f"task_runner.py: TaskRunner.{method}() contract is missing")
    if "pending_changed = pyqtSignal(int)" not in runner_source:
        violations.append("task_runner.py: pending_changed signal contract is missing")
    if runner_source.count("pending_changed.emit(len(self._tasks))") < 2:
        violations.append("task_runner.py: pending_changed must cover add and completion release")
    for fragment in (
        "class _TaskTerminationError(RuntimeError):",
        "except BaseException as error:",
        "self.error = _TaskTerminationError(error)",
    ):
        if fragment not in runner_source:
            violations.append(
                f"task_runner.py: abnormal worker termination contract is missing {fragment!r}"
            )
    if "def _release_task(self, task: _Task) -> None:" not in runner_source:
        violations.append("task_runner.py: idempotent task-release boundary is missing")
    if (
        "except Exception:\n            self._release_task(task)\n            raise"
        not in runner_source
    ):
        violations.append("task_runner.py: submit failure must roll back task retention")
    if runner_source.count("self._release_task(task)") < 2:
        violations.append("task_runner.py: submit and completion must share task release")
    for contract in (
        "self._runner.pending_changed.connect(self._on_runner_pending_changed)",
        "def _on_runner_pending_changed(",
        "self._runner.has_pending_work()",
    ):
        if contract not in main_source:
            violations.append(
                f"main_window.py: worker observability contract is missing {contract!r}"
            )
    return violations


def main() -> int:
    violations = [
        *_audit_i18n_literal_keys(),
        *_audit_static_notification_localization(),
        *_audit_theme_contrast_contract(),
        *_audit_tooltip_contract(),
        *_audit_shell_separator_contract(),
        *_audit_scrollbar_contract(),
        *_audit_combo_expanded_contract(),
        *_audit_checkbox_indicator_contract(),
        *_audit_tab_close_button_contract(),
        *_audit_typography_stepper_contract(),
        *_audit_settings_identity_choice_contract(),
        *_audit_settings_locale_choice_contract(),
        *_audit_settings_accessibility_contract(),
        *_audit_settings_restore_defaults_contract(),
        *_audit_settings_draft_status_contract(),
        *_audit_settings_typography_role_contract(),
        *_audit_settings_behavior_toggle_role_contract(),
        *_audit_workspace_file_activation_contract(),
        *_audit_workspace_open_affordance_contract(),
        *_audit_settings_typography_contract(),
        *_audit_startup_surface_contract(),
        *_audit_main_window_startup_state_contract(),
        *_audit_startup_failure_log_lifecycle_contract(),
        *_audit_startup_runtime_composition_contract(),
        *_audit_startup_diagnostic_qt_lifecycle_contract(),
        *_audit_frozen_qt_plugin_contract(),
        *_audit_frozen_qt_runtime_preflight_contract(),
        *_audit_frozen_qt_runtime_layout_contract(),
        *_audit_safe_mode_startup_contract(),
        *_audit_entrypoint_runtime_cleanup_contract(),
        *_audit_runtime_shutdown_contract(),
        *_audit_startup_state_preflight_contract(),
        *_audit_startup_restore_preflight_contract(),
        *_audit_file_open_diagnostic_contract(),
        *_audit_private_self_calls(),
        *_audit_coordinator_dependencies(),
        *_audit_ports_contracts(),
        *_audit_coordinator_notification_levels(),
        *_audit_notification_levels(),
        *_audit_task_runner_observability(),
    ]
    if violations:
        print("Presentation contract audit failed:")
        for violation in violations:
            print(f"- {violation}")
        return 1
    print("Presentation contract audit passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
