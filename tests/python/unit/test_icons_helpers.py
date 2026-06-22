"""icons + button_icons 纯 helper 边界单元测试。

补强 test_icons.py 未直接断言的边角：
- IconManager._tint：currentColor 替换 + 多 stroke 全替换 + 无 stroke 不变 + hex 颜色注入。
- _BUTTON_ICON_MAP：全部 objectName 键 + 每条 2 元组 + 颜色非空。
- _FOCUSABLE_WIDGET_TYPES：5 种控件类型。
- _resolve_widget_class：已知类型返回类 / 未知返回 None。
- _is_readonly：isReadOnly=True/False / 无 isReadOnly 属性=False。
- button_icon：便捷函数委托 IconManager。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.button_icons import (
    _BUTTON_ICON_MAP,
    _FOCUSABLE_WIDGET_TYPES,
    _is_readonly,
    _resolve_widget_class,
)
from embeddebug.serial_station.ui.icons import IconManager, button_icon


# ── IconManager._tint 静态方法 ───────────────────────────────────────────


def test_tint_replaces_current_color():
    """currentColor → 目标颜色。"""

    svg = '<path stroke="currentColor"/>'
    tinted = IconManager._tint(svg, "#22d3ee")
    assert "#22d3ee" in tinted
    assert "currentColor" not in tinted


def test_tint_replaces_all_strokes_multi_path():
    """多 path SVG 的全部 stroke 属性都被替换（count 不限 1）。"""

    svg = '<g><path stroke="#abc"/><path stroke="#def"/></g>'
    tinted = IconManager._tint(svg, "#ff0000")
    assert tinted.count('stroke="#ff0000"') == 2
    assert "#abc" not in tinted
    assert "#def" not in tinted


def test_tint_preserves_non_stroke_content():
    """非 stroke 属性（fill/width/height）保持不变。"""

    svg = '<svg width="24" fill="none"><path stroke="#000"/></svg>'
    tinted = IconManager._tint(svg, "#22d3ee")
    assert 'width="24"' in tinted
    assert 'fill="none"' in tinted


def test_tint_handles_rgba_color():
    """rgba() 颜色字符串也能注入 stroke。"""

    svg = '<path stroke="currentColor"/>'
    tinted = IconManager._tint(svg, "rgba(34,211,238,0.12)")
    assert "rgba(34,211,238,0.12)" in tinted


def test_tint_empty_svg():
    """空 SVG 不崩溃。"""

    assert IconManager._tint("", "#fff") == ""


def test_tint_no_stroke_attribute():
    """无 stroke 属性的 SVG 保持不变（仅替换 currentColor）。"""

    svg = '<path fill="red"/>'
    tinted = IconManager._tint(svg, "#22d3ee")
    assert 'fill="red"' in tinted
    # 无 stroke 属性 → re.sub 不匹配 → 原样
    assert 'stroke="#22d3ee"' not in tinted


# ── _BUTTON_ICON_MAP 常量契约 ───────────────────────────────────────────


def test_button_icon_map_non_empty():
    """_BUTTON_ICON_MAP 非空（至少有连接类按钮）。"""

    assert len(_BUTTON_ICON_MAP) > 10


def test_button_icon_map_all_values_are_two_tuples():
    """每条映射值是 (icon_name, color) 2 元组。"""

    for object_name, value in _BUTTON_ICON_MAP.items():
        assert isinstance(value, tuple), f"{object_name} value not tuple"
        assert len(value) == 2, f"{object_name} value not 2-tuple"
        icon_name, color = value
        assert isinstance(icon_name, str) and icon_name, f"{object_name} empty icon"
        assert isinstance(color, str) and color, f"{object_name} empty color"


def test_button_icon_map_keys_are_serialstation_prefixed():
    """所有 objectName 以 serialStation 开头（QSS 契约）。"""

    for object_name in _BUTTON_ICON_MAP:
        assert object_name.startswith("serialStation"), f"{object_name} missing prefix"


def test_button_icon_map_contains_connect_and_disconnect():
    """含连接类和断开按钮（核心交互）。"""

    assert "serialStationConnectButton" in _BUTTON_ICON_MAP
    assert "serialStationDisconnectButton" in _BUTTON_ICON_MAP


# ── _FOCUSABLE_WIDGET_TYPES 常量 ─────────────────────────────────────────


def test_focusable_widget_types_has_five():
    """_FOCUSABLE_WIDGET_TYPES 含 5 种控件类型。"""

    assert len(_FOCUSABLE_WIDGET_TYPES) == 5
    assert "QLineEdit" in _FOCUSABLE_WIDGET_TYPES
    assert "QComboBox" in _FOCUSABLE_WIDGET_TYPES
    assert "QPlainTextEdit" in _FOCUSABLE_WIDGET_TYPES
    assert "QSpinBox" in _FOCUSABLE_WIDGET_TYPES
    assert "QDoubleSpinBox" in _FOCUSABLE_WIDGET_TYPES


# ── _resolve_widget_class ────────────────────────────────────────────────


def test_resolve_widget_class_known():
    """已知类型（QLineEdit）→ 返回类对象。"""

    cls = _resolve_widget_class("QLineEdit")
    assert cls is not None
    assert cls.__name__ == "QLineEdit"


def test_resolve_widget_class_unknown():
    """未知类型名 → None。"""

    assert _resolve_widget_class("QNonexistent") is None


def test_resolve_widget_class_all_focusable_types():
    """5 种 focusable 类型都能解析。"""

    for type_name in _FOCUSABLE_WIDGET_TYPES:
        assert _resolve_widget_class(type_name) is not None, f"{type_name} unresolved"


# ── _is_readonly ────────────────────────────────────────────────────────


class _FakeWidget:
    """模拟 QWidget 的 isReadOnly 行为。"""

    def __init__(self, readonly: bool = False) -> None:
        self._readonly = readonly

    def isReadOnly(self) -> bool:
        return self._readonly


class _NoReadonlyAttr:
    """无 isReadOnly 属性的控件。"""

    pass


def test_is_readonly_true():
    """isReadOnly()=True → True。"""

    assert _is_readonly(_FakeWidget(readonly=True)) is True


def test_is_readonly_false():
    """isReadOnly()=False → False。"""

    assert _is_readonly(_FakeWidget(readonly=False)) is False


def test_is_readonly_no_attribute():
    """无 isReadOnly 属性 → False（getattr 默认）。"""

    assert _is_readonly(_NoReadonlyAttr()) is False


# ── button_icon 便捷函数 ────────────────────────────────────────────────


def test_button_icon_delegates_to_icon_manager():
    """button_icon 返回 QIcon（委托 IconManager.icon）。"""

    from PyQt6.QtGui import QIcon

    # 用不存在的图标名测试委托关系（避免 SVG 资源加载，返回空 QIcon）
    icon = button_icon("__nonexistent_test_icon__")
    assert isinstance(icon, QIcon)
