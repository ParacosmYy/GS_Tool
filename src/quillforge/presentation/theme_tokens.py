"""Framework-neutral visual tokens and contrast resolution for QuillForge."""

from __future__ import annotations

from dataclasses import dataclass, replace


@dataclass(frozen=True, slots=True)
class ThemeColors:
    """Resolved shell tokens shared by Qt and editor presentation adapters."""

    surface_0: str
    surface_1: str
    surface_2: str
    surface_3: str
    surface_hover: str
    border: str
    border_strong: str
    text_primary: str
    text_secondary: str
    text_muted: str
    accent: str
    accent_alt: str
    accent_pink: str
    accent_gold: str
    success: str
    danger: str
    selection: str
    pressed: str
    success_bg: str
    warning_bg: str
    error_bg: str
    editor_line: str
    on_accent: str
    on_accent_pink: str
    on_accent_gold: str


@dataclass(frozen=True, slots=True)
class EditorColorTokens:
    """Resolved canvas, interaction, and syntax colors for one editor theme."""

    canvas: str
    gutter: str
    gutter_text: str
    selection: str
    selection_text: str
    caret: str
    current_line: str
    comment: str
    number: str
    string: str
    triple_string: str
    keyword: str
    type_name: str
    function: str
    operator: str
    error: str
    decorator: str


@dataclass(frozen=True, slots=True)
class QssForegroundTokens:
    """Resolved foregrounds for semantic QSS surfaces and state feedback."""

    accent_alt_text: str
    accent_alt_fill: str
    selection: str
    warning: str
    success: str
    working: str
    error: str


INK_VIOLET = ThemeColors(
    surface_0="#090c14",
    surface_1="#0e121d",
    surface_2="#121827",
    surface_3="#171e30",
    surface_hover="#1d2639",
    border="#273149",
    border_strong="#35415e",
    text_primary="#edf2ff",
    text_secondary="#a7b1c8",
    text_muted="#9aa6bf",
    accent="#8b6cff",
    accent_alt="#38d6e8",
    accent_pink="#e77bff",
    accent_gold="#f2c66d",
    success="#56d6a2",
    danger="#ff718d",
    selection="#51418a",
    pressed="#30295a",
    success_bg="#173d35",
    warning_bg="#4b3b20",
    error_bg="#4b2531",
    editor_line="#111a2a",
    on_accent="#15101f",
    on_accent_pink="#15101f",
    on_accent_gold="#15101f",
)

PAPER_SAND = ThemeColors(
    surface_0="#fbfaf8",
    surface_1="#f4f1ed",
    surface_2="#eee9e3",
    surface_3="#e7e1d9",
    surface_hover="#ddd6cc",
    border="#d6cec3",
    border_strong="#bdb3a5",
    text_primary="#252936",
    text_secondary="#5c6575",
    text_muted="#566272",
    accent="#7359d6",
    accent_alt="#087f8c",
    accent_pink="#a954aa",
    accent_gold="#875508",
    success="#18835c",
    danger="#c54259",
    selection="#ded5ff",
    pressed="#d2c7f2",
    success_bg="#dff3ea",
    warning_bg="#f6ead0",
    error_bg="#f8e0e6",
    editor_line="#f0ebff",
    on_accent="#ffffff",
    on_accent_pink="#15101f",
    on_accent_gold="#ffffff",
)

SAKURA_POP = ThemeColors(
    surface_0="#171122",
    surface_1="#211731",
    surface_2="#2d1d42",
    surface_3="#3a2450",
    surface_hover="#543568",
    border="#6a4b78",
    border_strong="#8b5d93",
    text_primary="#fff3fb",
    text_secondary="#e7c7db",
    text_muted="#c49ab9",
    accent="#ff72b6",
    accent_alt="#78e5ef",
    accent_pink="#ffb0d6",
    accent_gold="#ffd27a",
    success="#78e0b1",
    danger="#ff779b",
    selection="#81456f",
    pressed="#633957",
    success_bg="#214a3d",
    warning_bg="#62451f",
    error_bg="#5f2d43",
    editor_line="#251733",
    on_accent="#15101f",
    on_accent_pink="#15101f",
    on_accent_gold="#15101f",
)

_BASE_THEMES = {
    "ink-violet": INK_VIOLET,
    "paper-sand": PAPER_SAND,
    "sakura-pop": SAKURA_POP,
}

_ACCENT_TONES = {
    "ink-violet": {
        "violet": ("#8b6cff", "#38d6e8", "#e77bff", "#f2c66d"),
        "cyan": ("#18afc0", "#66e4ed", "#9a80ff", "#f2c66d"),
        "rose": ("#d96b91", "#62d8df", "#ff8fb4", "#f2c66d"),
        "amber": ("#d39a38", "#62dce7", "#d97de2", "#f0c663"),
    },
    "paper-sand": {
        "violet": ("#7359d6", "#087f8c", "#a954aa", "#875508"),
        "cyan": ("#087f8c", "#126c9b", "#7359d6", "#875508"),
        "rose": ("#b84770", "#087f8c", "#a14c91", "#875508"),
        "amber": ("#9a620a", "#087f8c", "#9a5c9a", "#875508"),
    },
    "sakura-pop": {
        "violet": ("#b06cff", "#78e5ef", "#ff9ed0", "#ffd27a"),
        "cyan": ("#2bbcc8", "#8cecf1", "#ff9ed0", "#ffd27a"),
        "rose": ("#ff72b6", "#78e5ef", "#ffb0d6", "#ffd27a"),
        "amber": ("#e6a83e", "#78e5ef", "#ff9ed0", "#ffd27a"),
    },
}


def theme_colors(theme_id: str, accent_id: str = "rose") -> ThemeColors:
    """Resolve one immutable shell token set from bounded theme identifiers."""
    theme_key = theme_id if theme_id in _BASE_THEMES else "sakura-pop"
    base = _BASE_THEMES[theme_key]
    tones = _ACCENT_TONES[theme_key]
    accent, accent_alt, accent_pink, accent_gold = tones.get(accent_id, tones["violet"])
    return replace(
        base,
        accent=accent,
        accent_alt=accent_alt,
        accent_pink=accent_pink,
        accent_gold=accent_gold,
        on_accent=best_on_accent((accent, accent_pink, accent_gold)),
        on_accent_pink=best_on_accent((accent_pink,)),
        on_accent_gold=best_on_accent((accent_gold,)),
    )


def editor_color_tokens(theme_id: str, accent_id: str = "rose") -> EditorColorTokens:
    """Resolve the editor canvas, interaction, and syntax token set."""
    colors = theme_colors(theme_id, accent_id)
    canvas = colors.surface_0
    operator_fallback = "#85396d" if theme_id == "paper-sand" else colors.text_primary
    return EditorColorTokens(
        canvas=canvas,
        gutter=colors.surface_1,
        gutter_text=colors.text_muted,
        selection=colors.selection,
        selection_text=best_on_accent((colors.selection,)),
        caret=colors.accent_alt,
        current_line=colors.editor_line,
        comment=colors.text_muted,
        number=readable_foreground(colors.accent_gold, canvas, colors.text_primary),
        string="#287a57" if theme_id == "paper-sand" else "#b8ed9c",
        triple_string="#236b4d" if theme_id == "paper-sand" else "#8fca86",
        keyword=readable_foreground(colors.accent, canvas, colors.text_primary),
        type_name=readable_foreground(colors.accent_alt, canvas, colors.text_primary),
        function=readable_foreground(colors.accent_alt, canvas, colors.text_primary),
        operator=readable_foreground(colors.accent_pink, canvas, operator_fallback),
        error=readable_foreground(colors.danger, canvas, colors.text_primary),
        decorator=readable_foreground(colors.accent_gold, canvas, colors.text_primary),
    )


def qss_foreground_tokens(colors: ThemeColors) -> QssForegroundTokens:
    """Resolve every non-trivial text-on-state-surface QSS foreground once."""
    return QssForegroundTokens(
        accent_alt_text=readable_foreground(
            colors.accent_alt,
            colors.surface_3,
            colors.text_primary,
        ),
        accent_alt_fill=best_on_accent((colors.accent_alt,)),
        selection=best_on_accent((colors.selection,)),
        warning=best_on_accent((colors.warning_bg,)),
        success=readable_foreground(
            colors.success,
            colors.success_bg,
            colors.text_primary,
        ),
        working=readable_foreground(
            colors.accent_alt,
            colors.pressed,
            colors.text_primary,
        ),
        error=readable_foreground(
            colors.danger,
            colors.error_bg,
            colors.text_primary,
        ),
    )


def best_on_accent(backgrounds: tuple[str, ...]) -> str:
    """Choose a readable foreground for every bright/dark accent endpoint."""
    candidates = ("#ffffff", "#15101f")
    return max(
        candidates,
        key=lambda foreground: min(
            contrast_ratio(foreground, background) for background in backgrounds
        ),
    )


def readable_foreground(preferred: str, background: str, fallback: str) -> str:
    """Keep semantic color when it meets normal-text contrast, else fall back."""
    return preferred if contrast_ratio(preferred, background) >= 4.5 else fallback


def readable_edge_foreground(preferred: str, background: str, fallback: str) -> str:
    """Keep a semantic edge color when it meets the non-text contrast floor."""
    return preferred if contrast_ratio(preferred, background) >= 3.0 else fallback


def readable_edge_foreground_for_surfaces(
    preferred: str,
    backgrounds: tuple[str, ...],
    fallback: str,
) -> str:
    """Keep an edge color only when it clears the floor on every state surface."""
    return (
        preferred
        if all(contrast_ratio(preferred, background) >= 3.0 for background in backgrounds)
        else fallback
    )


def contrast_ratio(foreground: str, background: str) -> float:
    """Return the WCAG-style contrast ratio for two six-digit hex colors."""
    foreground_luminance = relative_luminance(foreground)
    background_luminance = relative_luminance(background)
    lighter, darker = sorted((foreground_luminance, background_luminance), reverse=True)
    return (lighter + 0.05) / (darker + 0.05)


def relative_luminance(color: str) -> float:
    """Return relative luminance for a six-digit hex color."""
    channels = []
    for offset in (1, 3, 5):
        channel = int(color[offset : offset + 2], 16) / 255
        channels.append(
            channel / 12.92 if channel <= 0.03928 else ((channel + 0.055) / 1.055) ** 2.4
        )
    return 0.2126 * channels[0] + 0.7152 * channels[1] + 0.0722 * channels[2]
