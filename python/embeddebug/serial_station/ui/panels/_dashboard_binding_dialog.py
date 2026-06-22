"""仪表盘控件绑定配置对话框 helper — 右键「配置数据源...」时弹出。

根据 widget 类型显示不同的配置控件：
- ValueDisplay / Gauge: 测量通道下拉（通道 0-7）
- StatusLed: 日志级别下拉（info / warning / error）
- CommandSlider: 命令前缀 + min/max（三段输入）
- ConfigurableButton: 命令文本输入

输出 binding spec 字符串（参考 ``dashboard_binding_service.parse_binding_spec``
的格式）。点取消 / 关闭返回 None，由调用方判定是否应用。

约束：本模块依赖 PyQt6 + binding_service 的纯数据层 API，不访问 controller/transport。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtWidgets import (
    QDialog,
    QDialogButtonBox,
    QFormLayout,
    QLineEdit,
    QSpinBox,
    QWidget,
)

from embeddebug.serial_station.services.dashboard_binding_service import (
    LED_LEVELS,
    default_spec_for_widget_type,
    is_valid_spec,
    parse_binding_spec,
)

# widget_type → binding kind 前缀（spec 第一段）。
_KIND_BY_WIDGET_TYPE: dict[str, str] = {
    "value_display": "value",
    "gauge": "gauge",
    "led": "led",
    "slider": "slider",
    "button": "button",
}

# 通道下拉的可选项（VOFA+ 一般 8 通道以内）。
_CHANNEL_INDICES: tuple[int, ...] = tuple(range(8))


def open_binding_dialog(
    widget: QWidget,
    widget_type: str,
    current_binding: str | None,
    parent: QWidget,
    on_apply: Callable[[str], None],
) -> None:
    """打开绑定配置对话框；用户确认时调用 ``on_apply(new_spec)``。

    任一步取消 / 关闭 / 输入无效时静默返回（不调用 on_apply）。
    所有用户可见文字走 ``parent.tr(...)``（铁律 19）。
    """

    spec = _build_dialog(
        parent=parent,
        widget_type=widget_type,
        current_spec=current_binding,
    )
    if spec and is_valid_spec(spec):
        on_apply(spec)


def _build_dialog(
    parent: QWidget,
    widget_type: str,
    current_spec: str | None,
) -> str | None:
    """根据 widget_type 选择对应的对话框内容，返回确认后的 spec 字符串。"""

    kind = _KIND_BY_WIDGET_TYPE.get(widget_type)
    if kind is None:
        return None
    if kind in ("value", "gauge"):
        return _channel_dialog(parent, kind, current_spec)
    if kind == "led":
        return _led_dialog(parent, current_spec)
    if kind == "slider":
        return _slider_dialog(parent, current_spec)
    if kind == "button":
        return _button_dialog(parent, current_spec)
    return None


def _channel_dialog(
    parent: QWidget,
    kind: str,
    current_spec: str | None,
) -> str | None:
    """value / gauge: 通道下拉。"""

    parsed = parse_binding_spec(current_spec) if current_spec else None
    current_index = parsed.channel if parsed and parsed.channel is not None else 0
    from PyQt6.QtWidgets import QInputDialog

    labels = [parent.tr("通道 {n}").format(n=i) for i in _CHANNEL_INDICES]
    current_label = labels[min(current_index, len(labels) - 1)]
    choice, ok = QInputDialog.getItem(
        parent,
        parent.tr("绑定测量通道"),
        parent.tr("选择数据源通道："),
        labels,
        labels.index(current_label) if current_label in labels else 0,
        editable=False,
    )
    if not ok:
        return None
    try:
        idx = labels.index(choice)
    except ValueError:
        return None
    return f"{kind}:ch{idx}"


def _led_dialog(parent: QWidget, current_spec: str | None) -> str | None:
    """led: 日志级别下拉。"""

    parsed = parse_binding_spec(current_spec) if current_spec else None
    current_level = parsed.level if parsed and parsed.level else "info"
    from PyQt6.QtWidgets import QInputDialog

    level_labels = {
        "info": parent.tr("信息（任意日志到达时脉冲）"),
        "warning": parent.tr("警告"),
        "error": parent.tr("错误（错误日志到达时点亮）"),
    }
    labels = [level_labels[lvl] for lvl in LED_LEVELS]
    keys = list(LED_LEVELS)
    try:
        current_idx = keys.index(current_level)
    except ValueError:
        current_idx = 0
    choice, ok = QInputDialog.getItem(
        parent,
        parent.tr("绑定日志级别"),
        parent.tr("选择 LED 响应级别："),
        labels,
        current_idx,
        editable=False,
    )
    if not ok:
        return None
    try:
        idx = labels.index(choice)
    except ValueError:
        return None
    return f"led:{keys[idx]}"


def _slider_dialog(parent: QWidget, current_spec: str | None) -> str | None:
    """slider: 自定义对话框（前缀 + min/max）。"""

    parsed = parse_binding_spec(current_spec) if current_spec else None
    dialog = _SpecDialog(parent)
    dialog.setWindowTitle(parent.tr("配置命令滑块"))
    dialog.setObjectName("serialStationDashboardBindingDialogSlider")
    form = QFormLayout(dialog)
    prefix_edit = QLineEdit(parsed.prefix if parsed and parsed.prefix else "SET", dialog)
    prefix_edit.setObjectName("serialStationDashboardBindingDialogSliderPrefix")
    min_spin = QSpinBox(dialog)
    min_spin.setRange(-100000, 100000)
    min_spin.setValue(parsed.minimum if parsed and parsed.minimum is not None else 0)
    max_spin = QSpinBox(dialog)
    max_spin.setRange(-100000, 100000)
    max_spin.setValue(parsed.maximum if parsed and parsed.maximum is not None else 100)
    form.addRow(parent.tr("命令前缀:"), prefix_edit)
    form.addRow(parent.tr("最小值:"), min_spin)
    form.addRow(parent.tr("最大值:"), max_spin)
    buttons = QDialogButtonBox(
        QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel,
        parent=dialog,
    )
    buttons.accepted.connect(dialog.accept)
    buttons.rejected.connect(dialog.reject)
    form.addRow(buttons)
    if dialog.exec() != QDialog.DialogCode.Accepted:
        return None
    prefix = prefix_edit.text().strip()
    if not prefix or "," in prefix:
        return None
    if min_spin.value() >= max_spin.value():
        return None
    return f"slider:{prefix},{min_spin.value()},{max_spin.value()}"


def _button_dialog(parent: QWidget, current_spec: str | None) -> str | None:
    """button: 命令文本输入。"""

    parsed = parse_binding_spec(current_spec) if current_spec else None
    current_cmd = parsed.command if parsed and parsed.command else ""
    from PyQt6.QtWidgets import QInputDialog

    text, ok = QInputDialog.getText(
        parent,
        parent.tr("配置按钮命令"),
        parent.tr("点击按钮时发送的命令文本："),
        text=current_cmd,
    )
    if not ok:
        return None
    text = text.strip()
    if not text:
        return None
    return f"button:{text}"


def suggest_default_spec(widget_type: str) -> str:
    """为给定 widget_type 返回默认 spec（用于首次添加时的初始绑定）。"""

    return default_spec_for_widget_type(widget_type) or ""


class _SpecDialog(QDialog):
    """辅助自定义对话框（slider 配置用），便于测试 mock exec()。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationDashboardBindingDialog")
