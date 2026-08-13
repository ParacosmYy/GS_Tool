"""Qt settings projection for language, appearance, and editor preferences."""

from __future__ import annotations

from typing import cast

from PyQt6.QtCore import QSignalBlocker, QSize, Qt
from PyQt6.QtGui import QFont, QFontDatabase
from PyQt6.QtWidgets import (
    QCheckBox,
    QComboBox,
    QDialog,
    QDialogButtonBox,
    QFormLayout,
    QFrame,
    QGroupBox,
    QLabel,
    QPushButton,
    QScrollArea,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)

from ..application.settings import (
    CURRENT_SETTINGS_VERSION,
    DEFAULT_SETTINGS,
    SUPPORTED_EDITOR_FONTS,
    SUPPORTED_FONT_STYLES,
    SUPPORTED_UI_FONTS,
)
from ..domain.models import (
    AccentId,
    AppearanceSettings,
    EditorSettings,
    FontStyle,
    Locale,
    SettingsSnapshot,
    ThemeId,
)
from .i18n import normalize_locale, tr
from .icons import color_swatch_icon
from .settings_preview import SettingsPreviewSurface
from .theme import theme_colors


class SettingsDialog(QDialog):
    """Edit validated preferences without owning persistence policy."""

    def __init__(self, settings: SettingsSnapshot, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("settingsDialog")
        self.setMinimumWidth(540)
        self._draft_baseline = settings
        self._locale: Locale = normalize_locale(settings.appearance.locale)
        self._installed_font_families = self._read_installed_font_families()

        self._language = QComboBox(self)
        self._language.setObjectName("settingsLanguage")
        self._language.setProperty("settingsRole", "localeChoice")
        self._language.addItem("English", "en-US")
        self._language.addItem("简体中文", "zh-CN")
        self._language.setCurrentIndex(self._language.findData(settings.appearance.locale))

        self._theme = QComboBox(self)
        self._theme.setObjectName("settingsTheme")
        self._theme.setProperty("settingsRole", "identityChoice")
        self._theme.addItem("Ink · Violet", "ink-violet")
        self._theme.addItem("Paper · Sand", "paper-sand")
        self._theme.addItem("Sakura · Pop", "sakura-pop")
        self._theme.setCurrentIndex(self._theme.findData(settings.appearance.theme))
        self._theme.setIconSize(QSize(32, 20))

        self._accent = QComboBox(self)
        self._accent.setObjectName("settingsAccent")
        self._accent.setProperty("settingsRole", "identityChoice")
        self._accent.addItem("Violet", "violet")
        self._accent.addItem("Cyan", "cyan")
        self._accent.addItem("Rose", "rose")
        self._accent.addItem("Amber", "amber")
        self._accent.setCurrentIndex(self._accent.findData(settings.appearance.accent))
        self._accent.setIconSize(QSize(32, 20))
        self._refresh_palette_icons()

        self._ui_font = QComboBox(self)
        self._ui_font.setObjectName("settingsUiFont")
        self._ui_font.setProperty("settingsRole", "typographyChoice")
        self._ui_font.setProperty("settingsTone", "interface")
        self._ui_font.addItems(SUPPORTED_UI_FONTS)
        self._apply_font_previews(self._ui_font, SUPPORTED_UI_FONTS)
        self._ui_font.setCurrentText(settings.appearance.ui_font_family)

        self._ui_font_size = QSpinBox(self)
        self._ui_font_size.setObjectName("settingsUiFontSize")
        self._ui_font_size.setProperty("settingsRole", "typographyChoice")
        self._ui_font_size.setProperty("settingsTone", "interface")
        self._ui_font_size.setRange(9, 16)
        self._ui_font_size.setValue(settings.appearance.ui_font_size)

        self._ui_font_style = QComboBox(self)
        self._ui_font_style.setObjectName("settingsUiFontStyle")
        self._ui_font_style.setProperty("settingsRole", "typographyChoice")
        self._ui_font_style.setProperty("settingsTone", "interface")
        self._add_font_style_options(self._ui_font_style)
        self._ui_font_style.setCurrentIndex(
            self._ui_font_style.findData(settings.appearance.ui_font_style)
        )

        self._editor_font = QComboBox(self)
        self._editor_font.setObjectName("settingsEditorFont")
        self._editor_font.setProperty("settingsRole", "typographyChoice")
        self._editor_font.setProperty("settingsTone", "editor")
        self._editor_font.addItems(SUPPORTED_EDITOR_FONTS)
        self._apply_font_previews(self._editor_font, SUPPORTED_EDITOR_FONTS)
        self._editor_font.setCurrentText(settings.editor.font_family)

        self._editor_font_size = QSpinBox(self)
        self._editor_font_size.setObjectName("settingsEditorFontSize")
        self._editor_font_size.setProperty("settingsRole", "typographyChoice")
        self._editor_font_size.setProperty("settingsTone", "editor")
        self._editor_font_size.setRange(8, 32)
        self._editor_font_size.setValue(settings.editor.font_size)

        self._editor_font_style = QComboBox(self)
        self._editor_font_style.setObjectName("settingsEditorFontStyle")
        self._editor_font_style.setProperty("settingsRole", "typographyChoice")
        self._editor_font_style.setProperty("settingsTone", "editor")
        self._add_font_style_options(self._editor_font_style)
        self._editor_font_style.setCurrentIndex(
            self._editor_font_style.findData(settings.editor.font_style)
        )

        self._wrap_lines = QCheckBox(self)
        self._wrap_lines.setObjectName("settingsWrapLines")
        self._wrap_lines.setProperty("settingsRole", "behaviorToggle")
        self._wrap_lines.setProperty("settingsTone", "editor")
        self._wrap_lines.setChecked(settings.editor.wrap_lines)
        self._line_numbers = QCheckBox(self)
        self._line_numbers.setObjectName("settingsLineNumbers")
        self._line_numbers.setProperty("settingsRole", "behaviorToggle")
        self._line_numbers.setProperty("settingsTone", "editor")
        self._line_numbers.setChecked(settings.editor.show_line_numbers)
        self._motion = QCheckBox(self)
        self._motion.setObjectName("settingsMotion")
        self._motion.setProperty("settingsRole", "behaviorToggle")
        self._motion.setProperty("settingsTone", "interface")
        self._motion.setChecked(settings.appearance.motion_enabled)

        self._appearance_group = QGroupBox(self)
        self._appearance_group.setObjectName("settingsAppearanceGroup")
        appearance_form = QFormLayout(self._appearance_group)
        appearance_form.setContentsMargins(14, 18, 14, 14)
        appearance_form.setHorizontalSpacing(18)
        appearance_form.setVerticalSpacing(10)
        self._language_label = QLabel(self)
        self._theme_label = QLabel(self)
        self._accent_label = QLabel(self)
        self._ui_font_label = QLabel(self)
        self._ui_font_size_label = QLabel(self)
        self._ui_font_style_label = QLabel(self)
        for label in (
            self._language_label,
            self._theme_label,
            self._accent_label,
            self._ui_font_label,
            self._ui_font_size_label,
            self._ui_font_style_label,
        ):
            label.setProperty("settingsRole", "fieldLabel")
        appearance_form.addRow(self._language_label, self._language)
        appearance_form.addRow(self._theme_label, self._theme)
        appearance_form.addRow(self._accent_label, self._accent)
        appearance_form.addRow(self._ui_font_label, self._ui_font)
        appearance_form.addRow(self._ui_font_size_label, self._ui_font_size)
        appearance_form.addRow(self._ui_font_style_label, self._ui_font_style)

        self._editor_group = QGroupBox(self)
        self._editor_group.setObjectName("settingsEditorGroup")
        editor_form = QFormLayout(self._editor_group)
        editor_form.setContentsMargins(14, 18, 14, 14)
        editor_form.setHorizontalSpacing(18)
        editor_form.setVerticalSpacing(10)
        self._editor_font_label = QLabel(self)
        self._editor_font_size_label = QLabel(self)
        self._editor_font_style_label = QLabel(self)
        for label in (
            self._editor_font_label,
            self._editor_font_size_label,
            self._editor_font_style_label,
        ):
            label.setProperty("settingsRole", "fieldLabel")
        editor_form.addRow(self._editor_font_label, self._editor_font)
        editor_form.addRow(self._editor_font_size_label, self._editor_font_size)
        editor_form.addRow(self._editor_font_style_label, self._editor_font_style)
        editor_form.addRow(self._wrap_lines)
        editor_form.addRow(self._line_numbers)
        editor_form.addRow(self._motion)

        self._note = QLabel(self)
        self._note.setObjectName("settingsNote")
        self._note.setWordWrap(True)
        self._font_note = QLabel(self)
        self._font_note.setObjectName("settingsFontNote")
        self._font_note.setWordWrap(True)
        self._draft_status = QLabel(self)
        self._draft_status.setObjectName("settingsDraftStatus")
        self._draft_status.setWordWrap(True)

        self._preview_surface = SettingsPreviewSurface(self)

        self._content = QWidget(self)
        self._content.setObjectName("settingsContent")
        content_layout = QVBoxLayout(self._content)
        content_layout.setContentsMargins(0, 0, 0, 0)
        content_layout.setSpacing(14)
        content_layout.addWidget(self._appearance_group)
        content_layout.addWidget(self._preview_surface.widget)
        content_layout.addWidget(self._editor_group)
        content_layout.addWidget(self._note)
        content_layout.addWidget(self._font_note)
        content_layout.addWidget(self._draft_status)

        self._scroll = QScrollArea(self)
        self._scroll.setObjectName("settingsScroll")
        self._scroll.setFrameShape(QFrame.Shape.NoFrame)
        self._scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self._scroll.setWidgetResizable(True)
        self._scroll.setWidget(self._content)

        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Save
            | QDialogButtonBox.StandardButton.Cancel
            | QDialogButtonBox.StandardButton.RestoreDefaults,
            parent=self,
        )
        save_button = buttons.button(QDialogButtonBox.StandardButton.Save)
        if save_button is not None:
            save_button.setObjectName("primaryAction")
        cancel_button = buttons.button(QDialogButtonBox.StandardButton.Cancel)
        if cancel_button is not None:
            cancel_button.setObjectName("quietAction")
        reset_button = buttons.button(QDialogButtonBox.StandardButton.RestoreDefaults)
        if reset_button is not None:
            reset_button.setObjectName("resetSettingsAction")
            reset_button.clicked.connect(self._restore_defaults)
        buttons.setObjectName("dialogActions")
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        self._buttons = buttons
        self._reset_button: QPushButton | None = reset_button

        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(14)
        layout.addWidget(self._scroll, 1)
        layout.addWidget(buttons, 0)
        self._language.currentIndexChanged.connect(self._on_language_changed)
        self._theme.currentIndexChanged.connect(self._on_preview_changed)
        self._accent.currentIndexChanged.connect(self._on_preview_changed)
        self._ui_font.currentTextChanged.connect(self._on_preview_changed)
        self._ui_font_size.valueChanged.connect(self._on_preview_changed)
        self._ui_font_style.currentIndexChanged.connect(self._on_preview_changed)
        self._editor_font.currentTextChanged.connect(self._on_preview_changed)
        self._editor_font_size.valueChanged.connect(self._on_preview_changed)
        self._editor_font_style.currentIndexChanged.connect(self._on_preview_changed)
        self._wrap_lines.toggled.connect(self._on_draft_changed)
        self._line_numbers.toggled.connect(self._on_draft_changed)
        self._motion.toggled.connect(self._on_draft_changed)
        self.set_locale(self._locale)

    def settings_snapshot(self) -> SettingsSnapshot:
        """Return the complete dialog values as a domain value object."""
        locale = cast(Locale, self._language.currentData())
        theme = cast(ThemeId, self._theme.currentData())
        return SettingsSnapshot(
            schema_version=CURRENT_SETTINGS_VERSION,
            editor=EditorSettings(
                font_family=self._editor_font.currentText(),
                font_size=self._editor_font_size.value(),
                font_style=cast(FontStyle, self._editor_font_style.currentData()),
                wrap_lines=self._wrap_lines.isChecked(),
                show_line_numbers=self._line_numbers.isChecked(),
            ),
            appearance=AppearanceSettings(
                locale=locale,
                theme=theme,
                accent=cast(AccentId, self._accent.currentData()),
                ui_font_family=self._ui_font.currentText(),
                ui_font_size=self._ui_font_size.value(),
                ui_font_style=cast(FontStyle, self._ui_font_style.currentData()),
                motion_enabled=self._motion.isChecked(),
            ),
        )

    def set_locale(self, locale: Locale) -> None:
        """Refresh labels while preserving all current selections."""
        self._locale = normalize_locale(locale)
        self.setWindowTitle(tr("settings.title", self._locale))
        self._appearance_group.setTitle(tr("settings.appearance_group", self._locale))
        self._editor_group.setTitle(tr("settings.editor_group", self._locale))
        self._language_label.setText(tr("settings.language", self._locale))
        self._theme_label.setText(tr("settings.theme", self._locale))
        self._accent_label.setText(tr("settings.accent", self._locale))
        self._ui_font_label.setText(tr("settings.ui_font", self._locale))
        self._ui_font_size_label.setText(tr("settings.ui_font_size", self._locale))
        font_size_suffix = tr("settings.font_size_suffix", self._locale)
        self._ui_font_size.setSuffix(font_size_suffix)
        self._editor_font_size.setSuffix(font_size_suffix)
        self._ui_font_style_label.setText(tr("settings.ui_font_style", self._locale))
        self._editor_font_label.setText(tr("settings.editor_font", self._locale))
        self._editor_font_size_label.setText(tr("settings.editor_font_size", self._locale))
        self._editor_font_style_label.setText(tr("settings.editor_font_style", self._locale))
        self._wrap_lines.setText(tr("settings.wrap", self._locale))
        self._line_numbers.setText(tr("settings.line_numbers", self._locale))
        self._motion.setText(tr("settings.motion", self._locale))
        self._note.setText(tr("settings.apply_note", self._locale))
        self._refresh_font_status()
        self._language.setItemText(0, tr("settings.language_english", self._locale))
        self._language.setItemText(1, tr("settings.language_chinese", self._locale))
        self._theme.setItemText(0, tr("settings.theme_ink", self._locale))
        self._theme.setItemText(1, tr("settings.theme_paper", self._locale))
        self._theme.setItemText(2, tr("settings.theme_sakura", self._locale))
        self._accent.setItemText(0, tr("settings.accent_violet", self._locale))
        self._accent.setItemText(1, tr("settings.accent_cyan", self._locale))
        self._accent.setItemText(2, tr("settings.accent_rose", self._locale))
        self._accent.setItemText(3, tr("settings.accent_amber", self._locale))
        for combo in (self._ui_font_style, self._editor_font_style):
            for index in range(combo.count()):
                combo.setItemText(
                    index,
                    tr(self._font_style_key(combo.itemData(index)), self._locale),
                )
        self._buttons.button(QDialogButtonBox.StandardButton.Save).setText(
            tr("settings.ok", self._locale)
        )
        self._buttons.button(QDialogButtonBox.StandardButton.Cancel).setText(
            tr("settings.cancel", self._locale)
        )
        if self._reset_button is not None:
            self._reset_button.setText(tr("settings.restore_defaults", self._locale))
            self._reset_button.setToolTip(tr("settings.restore_defaults_hint", self._locale))
            self._reset_button.setAccessibleName(tr("settings.restore_defaults", self._locale))
        self._refresh_accessible_names()
        self._refresh_preview()
        self._refresh_draft_status()

    def _restore_defaults(self) -> None:
        """Reset only this dialog draft; persistence still requires Save."""
        defaults = DEFAULT_SETTINGS
        controls = (
            self._language,
            self._theme,
            self._accent,
            self._ui_font,
            self._ui_font_size,
            self._ui_font_style,
            self._editor_font,
            self._editor_font_size,
            self._editor_font_style,
            self._wrap_lines,
            self._line_numbers,
            self._motion,
        )
        blockers = [QSignalBlocker(widget) for widget in controls]
        self._language.setCurrentIndex(self._language.findData(defaults.appearance.locale))
        self._theme.setCurrentIndex(self._theme.findData(defaults.appearance.theme))
        self._accent.setCurrentIndex(self._accent.findData(defaults.appearance.accent))
        self._ui_font.setCurrentText(defaults.appearance.ui_font_family)
        self._ui_font_size.setValue(defaults.appearance.ui_font_size)
        self._ui_font_style.setCurrentIndex(
            self._ui_font_style.findData(defaults.appearance.ui_font_style)
        )
        self._editor_font.setCurrentText(defaults.editor.font_family)
        self._editor_font_size.setValue(defaults.editor.font_size)
        self._editor_font_style.setCurrentIndex(
            self._editor_font_style.findData(defaults.editor.font_style)
        )
        self._wrap_lines.setChecked(defaults.editor.wrap_lines)
        self._line_numbers.setChecked(defaults.editor.show_line_numbers)
        self._motion.setChecked(defaults.appearance.motion_enabled)
        del blockers
        self._refresh_palette_icons()
        self.set_locale(normalize_locale(defaults.appearance.locale))

    def _refresh_accessible_names(self) -> None:
        """Keep Settings control names aligned with the active locale."""
        for widget, label in (
            (self._language, self._language_label),
            (self._theme, self._theme_label),
            (self._accent, self._accent_label),
            (self._ui_font, self._ui_font_label),
            (self._ui_font_size, self._ui_font_size_label),
            (self._ui_font_style, self._ui_font_style_label),
            (self._editor_font, self._editor_font_label),
            (self._editor_font_size, self._editor_font_size_label),
            (self._editor_font_style, self._editor_font_style_label),
        ):
            widget.setAccessibleName(label.text())
        for widget in (self._wrap_lines, self._line_numbers, self._motion):
            widget.setAccessibleName(widget.text())

    def _refresh_draft_status(self) -> None:
        """Project whether the current draft differs from the opened snapshot."""
        changed = self.settings_snapshot() != self._draft_baseline
        state = "changed" if changed else "clean"
        message = tr(
            "settings.draft.changed" if changed else "settings.draft.clean",
            self._locale,
        )
        self._draft_status.setProperty("draftState", state)
        self._draft_status.setText(message)
        self._draft_status.setAccessibleName(message)
        style = self._draft_status.style()
        if style is not None:
            style.unpolish(self._draft_status)
            style.polish(self._draft_status)
        self._draft_status.update()

    def _on_language_changed(self, _index: int) -> None:
        selected = self._language.currentData()
        self.set_locale(normalize_locale(selected))

    @staticmethod
    def _read_installed_font_families() -> frozenset[str] | None:
        """Read the platform font catalog without turning an unavailable query into a crash."""
        try:
            return frozenset(QFontDatabase.families())
        except RuntimeError:
            return None

    def _refresh_font_status(self) -> None:
        """Explain whether the two selected families will render natively or fall back."""
        families = self._installed_font_families

        def status_key(family: str) -> str:
            if families is None:
                return "settings.font_status_unknown"
            return (
                "settings.font_status_installed"
                if family in families
                else "settings.font_status_fallback"
            )

        status = tr("settings.font_status", self._locale).format(
            ui_font=self._ui_font.currentText(),
            ui_status=tr(status_key(self._ui_font.currentText()), self._locale),
            editor_font=self._editor_font.currentText(),
            editor_status=tr(status_key(self._editor_font.currentText()), self._locale),
        )
        note = tr("settings.unsupported_font_note", self._locale)
        self._font_note.setText(f"{status}\n{note}")

    @staticmethod
    def _apply_font_previews(combo: QComboBox, families: tuple[str, ...]) -> None:
        """Render each supported font choice in its own family inside the menu."""
        for index, family in enumerate(families):
            combo.setItemData(
                index,
                QFont(family),
                Qt.ItemDataRole.FontRole,
            )

    @staticmethod
    def _add_font_style_options(combo: QComboBox) -> None:
        """Add one stable, locale-neutral style value per supported option."""
        for style in SUPPORTED_FONT_STYLES:
            combo.addItem("", style)

    @staticmethod
    def _font_style_key(style: object) -> str:
        return {
            "regular": "settings.font_style_regular",
            "semibold": "settings.font_style_semibold",
            "bold": "settings.font_style_bold",
            "italic": "settings.font_style_italic",
        }.get(style, "settings.font_style_regular")

    def _on_preview_changed(self, *_args: object) -> None:
        self._refresh_palette_icons()
        self._refresh_font_status()
        self._refresh_preview()
        self._refresh_draft_status()

    def _on_draft_changed(self, *_args: object) -> None:
        """Refresh draft status for behavior controls without preview work."""
        self._refresh_draft_status()

    def _refresh_palette_icons(self) -> None:
        """Keep theme and accent choices visually legible as pending values change."""
        selected_theme = self._theme.currentData()
        selected_accent = self._accent.currentData()
        if not isinstance(selected_theme, str) or not isinstance(selected_accent, str):
            return

        for index in range(self._theme.count()):
            theme = self._theme.itemData(index)
            if not isinstance(theme, str):
                continue
            colors = theme_colors(theme, selected_accent)
            self._theme.setItemIcon(
                index,
                color_swatch_icon(
                    colors.surface_0,
                    colors.accent,
                    tertiary=colors.accent_alt,
                    outline=colors.border_strong,
                ),
            )

        for index in range(self._accent.count()):
            accent = self._accent.itemData(index)
            if not isinstance(accent, str):
                continue
            colors = theme_colors(selected_theme, accent)
            self._accent.setItemIcon(
                index,
                color_swatch_icon(
                    colors.accent,
                    colors.accent_alt,
                    tertiary=colors.accent_pink,
                    outline=colors.border_strong,
                ),
            )

    def _refresh_preview(self) -> None:
        """Project pending appearance choices without applying or persisting them."""
        self._preview_surface.project(
            locale=self._locale,
            theme=cast(ThemeId, self._theme.currentData()),
            accent=cast(AccentId, self._accent.currentData()),
            font_family=self._ui_font.currentText(),
            font_size=self._ui_font_size.value(),
            font_style=cast(FontStyle, self._ui_font_style.currentData()),
            editor_font_family=self._editor_font.currentText(),
            editor_font_size=self._editor_font_size.value(),
            editor_font_style=cast(FontStyle, self._editor_font_style.currentData()),
            theme_label=self._theme.currentText(),
            accent_label=self._accent.currentText(),
            font_style_label=self._ui_font_style.currentText(),
            editor_font_style_label=self._editor_font_style.currentText(),
        )
