"""仪表盘控件数据绑定服务 — 把 measurement channel / log / command 路由到 widget。

职责：
- 持有当前活跃绑定表（widget_id → binding_spec 字符串）
- 提供 ``route_measurement`` / ``route_log`` 入口，由 DashboardPanel 在接到
  AppController 的事件回调时调用
- 解析 binding spec，把事件分发给注册的 handler（由 Panel 提供，handler 内部
  调用具体 QWidget 的 setter；本服务不持有任何 QWidget 引用）
- 不持久化（持久化由 canvas config["binding"] + _dashboard_layout_store 完成）

Binding spec 格式（紧凑字符串，便于 JSON 持久化）::

    value:ch0                 # ValueDisplay 绑定通道 0
    gauge:ch1                 # Gauge 绑定通道 1
    led:error                 # StatusLed 在 error 日志到达时点亮
    led:warning               # StatusLed 在 warning 日志到达时点亮
    led:info                  # StatusLed 在任何日志到达时脉冲
    slider:SET_VOLTAGE,0,100  # CommandSlider 命令前缀 + min/max
    button:RESET              # ConfigurableButton 命令文本

落点：services/ 层（L2），不依赖 QWidget / ui/，可被 tests/python 直接覆盖。
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

# 绑定类型常量（与 widget_type 对齐，但用 binding 自己的命名空间以解耦）。
KIND_VALUE = "value"        # ValueDisplay
KIND_GAUGE = "gauge"        # GaugeWidget
KIND_LED = "led"            # StatusLed
KIND_SLIDER = "slider"      # CommandSlider
KIND_BUTTON = "button"      # ConfigurableButton

# LED 可绑定的日志级别（与 route_log 入参对齐）。
LED_LEVELS: tuple[str, ...] = ("info", "warning", "error")

# 可路由到 widget 的入站绑定类型（值/仪表/LED）。
_ROUTABLE_KINDS: frozenset[str] = frozenset({KIND_VALUE, KIND_GAUGE, KIND_LED})

# 出站命令绑定类型（不参与 route_*，仅做持久化/校验）。
_COMMAND_KINDS: frozenset[str] = frozenset({KIND_SLIDER, KIND_BUTTON})


@dataclass(frozen=True)
class ParsedBinding:
    """解析后的绑定描述。

    kind 为 value/gauge 时 ``channel`` 有效；kind 为 led 时 ``level`` 有效；
    kind 为 slider 时 ``prefix`` / ``minimum`` / ``maximum`` 有效；kind 为
    button 时 ``command`` 有效。其他字段为 None / 0。
    """

    kind: str
    channel: int | None = None
    level: str | None = None
    prefix: str | None = None
    minimum: int | None = None
    maximum: int | None = None
    command: str | None = None


def parse_binding_spec(spec: str) -> ParsedBinding | None:
    """解析 binding spec 字符串，无法识别返回 None（不抛异常，调用方做 None 安全）。

    Examples::

        >>> parse_binding_spec("value:ch0").channel
        0
        >>> parse_binding_spec("led:error").level
        'error'
        >>> parse_binding_spec("slider:SET_VOLTAGE,0,100").prefix
        'SET_VOLTAGE'
    """

    if not spec or not isinstance(spec, str) or ":" not in spec:
        return None
    kind, _, detail = spec.partition(":")
    kind = kind.strip().lower()
    detail = detail.strip()
    if not detail:
        return None
    if kind in (KIND_VALUE, KIND_GAUGE):
        return _parse_channel_kind(kind, detail)
    if kind == KIND_LED:
        return _parse_led_kind(detail)
    if kind == KIND_SLIDER:
        return _parse_slider_kind(detail)
    if kind == KIND_BUTTON:
        return ParsedBinding(kind=KIND_BUTTON, command=detail)
    return None


def _parse_channel_kind(kind: str, detail: str) -> ParsedBinding | None:
    """解析 ``value:chN`` / ``gauge:chN``。"""

    if not detail.lower().startswith("ch"):
        return None
    try:
        index = int(detail[2:])
    except ValueError:
        return None
    if index < 0:
        return None
    return ParsedBinding(kind=kind, channel=index)


def _parse_led_kind(detail: str) -> ParsedBinding | None:
    """解析 ``led:LEVEL``。"""

    level = detail.lower()
    if level not in LED_LEVELS:
        return None
    return ParsedBinding(kind=KIND_LED, level=level)


def _parse_slider_kind(detail: str) -> ParsedBinding | None:
    """解析 ``slider:PREFIX,MIN,MAX``。

    PREFIX 不允许包含逗号；MIN/MAX 必须是整数（QSlider 基线）。
    """

    parts = detail.split(",")
    if len(parts) != 3:
        return None
    prefix = parts[0].strip()
    if not prefix:
        return None
    try:
        minimum = int(parts[1])
        maximum = int(parts[2])
    except ValueError:
        return None
    if minimum >= maximum:
        return None
    return ParsedBinding(
        kind=KIND_SLIDER, prefix=prefix, minimum=minimum, maximum=maximum
    )


def is_valid_spec(spec: str) -> bool:
    """spec 是否能被识别（用于对话框输入校验）。"""

    return parse_binding_spec(spec) is not None


def default_spec_for_widget_type(widget_type: str) -> str | None:
    """按 widget_type 返回默认 binding spec（用于首次配置时的建议值）。

    widget_type 取 ``WIDGET_CATALOG`` 的 ``type`` 字段（led/slider/button/gauge/value_display）。
    """

    mapping = {
        "value_display": "value:ch0",
        "gauge": "gauge:ch0",
        "led": "led:info",
        "slider": "slider:SET,0,100",
        "button": "button:RESET",
    }
    return mapping.get(widget_type)


UpdateHandler = Callable[[object], None]


class DashboardBindingService:
    """仪表盘绑定路由表。

    ``bind(widget_id, spec, handler)`` 注册一个绑定：当 route_measurement /
    route_log 命中该绑定时，调用 ``handler(payload)`` 把数据交给调用方
    （DashboardPanel），由调用方在主线程里更新对应 QWidget。

    本类不持有 QWidget、不依赖 PyQt6，可被单元测试直接覆盖。
    """

    def __init__(self) -> None:
        self._bindings: dict[str, str] = {}           # widget_id → spec
        self._parsed: dict[str, ParsedBinding] = {}    # widget_id → 解析结果（缓存）
        self._handlers: dict[str, UpdateHandler] = {}  # widget_id → handler

    # ── 注册 / 查询 ─────────────────────────────────────────────────
    def bind(
        self,
        widget_id: str,
        spec: str,
        handler: UpdateHandler | None = None,
    ) -> bool:
        """登记一个绑定，spec 无效时拒绝（返回 False，不覆盖旧绑定）。"""

        parsed = parse_binding_spec(spec)
        if parsed is None:
            return False
        self._bindings[widget_id] = spec
        self._parsed[widget_id] = parsed
        if handler is not None:
            self._handlers[widget_id] = handler
        return True

    def unbind(self, widget_id: str) -> None:
        """移除一个绑定（同时清掉 handler，避免悬空引用）。"""

        self._bindings.pop(widget_id, None)
        self._parsed.pop(widget_id, None)
        self._handlers.pop(widget_id, None)

    def get_binding(self, widget_id: str) -> str | None:
        """读取 widget_id 当前 spec（未绑定返回 None）。"""

        return self._bindings.get(widget_id)

    def all_bindings(self) -> dict[str, str]:
        """返回 {widget_id: spec} 拷贝（用于诊断 / 持久化校验）。"""

        return dict(self._bindings)

    def clear_all(self) -> None:
        """清空全部绑定（标签页全部关闭 / 重置场景）。"""

        self._bindings.clear()
        self._parsed.clear()
        self._handlers.clear()

    # ── 路由 ─────────────────────────────────────────────────────────
    def route_measurement(self, channel: int, value: float) -> int:
        """把单通道测量值路由到所有绑定该通道的 value/gauge 控件，返回命中数。"""

        hits = 0
        for widget_id, parsed in self._parsed.items():
            if parsed.kind in (KIND_VALUE, KIND_GAUGE) and parsed.channel == channel:
                handler = self._handlers.get(widget_id)
                if handler is not None:
                    try:
                        handler(value)
                    except Exception:
                        # handler 抛异常不能影响其他控件的路由。
                        pass
                hits += 1
        return hits

    def route_log(self, level: str, message: str) -> int:
        """把一条日志路由到所有绑定该 level 的 LED 控件，返回命中数。

        level 取 ``info`` / ``warning`` / ``error``。命中时 handler 收到
        ``(level, message)`` 元组，由 Panel 决定 LED 状态切换还是脉冲。
        """

        canonical = (level or "").lower()
        if canonical not in LED_LEVELS:
            return 0
        payload = (canonical, message)
        hits = 0
        for widget_id, parsed in self._parsed.items():
            if parsed.kind == KIND_LED and parsed.level == canonical:
                handler = self._handlers.get(widget_id)
                if handler is not None:
                    try:
                        handler(payload)
                    except Exception:
                        pass
                hits += 1
        return hits
