"""Presentation-only combo surface for builtin and custom preset entries."""

from __future__ import annotations

from .connection_preset_context_surface import ConnectionPresetContextSurface
from .connection_presets import (
    BUILTIN_CONNECTION_PRESET_KEYS,
    ConnectionPreset,
    ConnectionPresetCatalog,
)
from .property_refresh import refresh_dynamic_property
from .qt import QComboBox, Qt


def is_custom_connection_preset(preset: object) -> bool:
    """Return whether a catalog item is user-owned rather than builtin."""

    return isinstance(preset, ConnectionPreset) and (
        preset.key not in BUILTIN_CONNECTION_PRESET_KEYS
    )


def update_connection_preset_context(
    combo: QComboBox,
    preset: ConnectionPreset | None,
    *,
    context_surface: ConnectionPresetContextSurface | None = None,
) -> None:
    """Keep tooltip and assistive text aligned with the visible selection."""

    if preset is None:
        if context_surface is not None:
            context_surface.set_preset(None)
        combo.setToolTip("选择内置或自定义连接快速配置；只填入表单，不会自动连接或保存密钥。")
        combo.setAccessibleDescription(
            "连接快速配置：未选择。选择内置或自定义配置只填入表单，不会自动连接。"
        )
        return
    if context_surface is not None:
        context_surface.set_preset(preset)
    source = "自定义" if is_custom_connection_preset(preset) else "内置"
    combo.setToolTip(
        f"{source}连接快速配置：{preset.label}\n{preset.description}\n只填入表单，不会自动连接。"
    )
    combo.setAccessibleDescription(
        f"当前选中{source}连接快速配置：{preset.label}。{preset.description} "
        "选择只填入表单，不会自动连接或保存密钥。"
    )


def refresh_connection_preset_combo(
    combo: QComboBox,
    catalog: ConnectionPresetCatalog,
    *,
    selected_key: str | None = None,
    context_surface: ConnectionPresetContextSurface | None = None,
) -> None:
    """Replace the bounded preset list while keeping selection explicit."""

    blocked = combo.blockSignals(True)
    try:
        combo.clear()
        combo.addItem("选择快速配置…", None)
        selected_index = 0
        for preset in catalog:
            is_custom = is_custom_connection_preset(preset)
            label = f"自定义 · {preset.label}" if is_custom else preset.label
            combo.addItem(label, preset)
            item_index = combo.count() - 1
            combo.setItemData(item_index, preset.description, Qt.ItemDataRole.ToolTipRole)
            if preset.key == selected_key:
                selected_index = item_index
        combo.setCurrentIndex(selected_index)
        selected_preset = combo.itemData(selected_index, Qt.ItemDataRole.UserRole)
        refresh_dynamic_property(
            combo,
            "customSelected",
            selected_index > 0 and is_custom_connection_preset(selected_preset),
        )
        update_connection_preset_context(
            combo,
            selected_preset if isinstance(selected_preset, ConnectionPreset) else None,
            context_surface=context_surface,
        )
    finally:
        combo.blockSignals(blocked)
    combo.updateGeometry()


__all__ = [
    "is_custom_connection_preset",
    "refresh_connection_preset_combo",
    "update_connection_preset_context",
]
