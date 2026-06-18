"""会话状态数据结构。

定义会话恢复所需的不可变状态快照。仅依赖标准库与 dataclass，
便于在不同进程/启动实例之间序列化与还原 EmbedDebug 工作区状态。
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


def _default_geometry() -> dict[str, Any]:
    """默认窗口几何信息。"""

    return {"x": 0, "y": 0, "width": 1280, "height": 800}


def _default_connection_config() -> dict[str, Any]:
    """默认连接参数（不包含敏感凭证）。"""

    return {"port": "", "baudrate": 115200}


@dataclass
class SessionState:
    """单次工作区会话的可恢复状态快照。

    Attributes:
        window_geometry: 窗口位置/尺寸，纯数值字典。
        transport_mode: 传输通道模式（如 ``uart``/``tcp_client``）。
        protocol: 当前协议名称（如 ``raw_data``/``modbus_rtu``）。
        connection_config: 与传输相关的连接参数，禁止包含密钥。
        command_history: 最近用户输入命令列表。
        log_filter: 当前日志过滤关键字。
        active_tab: 当前激活面板的标识。
        timestamp_ns: 状态捕获时刻的纳秒时间戳。
    """

    window_geometry: dict[str, Any] = field(default_factory=_default_geometry)
    transport_mode: str = "uart"
    protocol: str = "raw_data"
    connection_config: dict[str, Any] = field(default_factory=_default_connection_config)
    command_history: list[str] = field(default_factory=list)
    log_filter: str = ""
    active_tab: str = ""
    timestamp_ns: int = 0

    def to_dict(self) -> dict[str, Any]:
        """转换为可被 JSON 序列化的字典。"""

        return {
            "window_geometry": dict(self.window_geometry),
            "transport_mode": self.transport_mode,
            "protocol": self.protocol,
            "connection_config": dict(self.connection_config),
            "command_history": list(self.command_history),
            "log_filter": self.log_filter,
            "active_tab": self.active_tab,
            "timestamp_ns": int(self.timestamp_ns),
        }

    @classmethod
    def from_dict(cls, data: dict[str, Any] | None) -> "SessionState":
        """从字典构造状态，缺失字段使用默认值以保证向后兼容。"""

        if not isinstance(data, dict):
            return cls()

        def _get(key: str, default: Any) -> Any:
            value = data.get(key, default)
            return default if value is None else value

        geometry = _get("window_geometry", _default_geometry())
        connection = _get("connection_config", _default_connection_config())
        history = _get("command_history", [])

        return cls(
            window_geometry=geometry if isinstance(geometry, dict) else _default_geometry(),
            transport_mode=str(_get("transport_mode", "uart")),
            protocol=str(_get("protocol", "raw_data")),
            connection_config=connection if isinstance(connection, dict) else _default_connection_config(),
            command_history=[str(item) for item in history] if isinstance(history, list) else [],
            log_filter=str(_get("log_filter", "")),
            active_tab=str(_get("active_tab", "")),
            timestamp_ns=int(_get("timestamp_ns", 0)),
        )
