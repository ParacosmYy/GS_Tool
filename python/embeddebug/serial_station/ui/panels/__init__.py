"""模式面板注册入口。

集中注册所有功能模式到 ``mode_panel`` registry，AppShell 按注册顺序装配导航。
新增模式 = 在此 import + register_panel 一行。

注册顺序即导航栏从上到下顺序。
"""

from __future__ import annotations

from embeddebug.app.mode_panel import register_panel, reset_registry
from embeddebug.serial_station.ui.panels.placeholder_panel import PlaceholderPanel
from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel


def register_default_panels() -> None:
    """注册首批模式面板（串口 / OTA / RTT / 设置）。

    幂等：重复调用不会重复注册（register_panel 内部去重）。
    """

    reset_registry()
    # 串口（现有，第一个=默认模式）。
    register_panel("serial", "cable", "串口", lambda app: SerialPanel())
    # OTA（X/YMODEM）— 引擎就绪后用真实面板替换；先占位保证导航完整。
    try:
        from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

        register_panel("ota", "upload", "OTA 升级", lambda app: OtaPanel())
    except ImportError:
        register_panel(
            "ota", "upload", "OTA 升级",
            lambda app: PlaceholderPanel(
                "ota", "OTA 升级", "XMODEM / YMODEM 固件升级（即将就绪）。"
            ),
        )
    # RTT（占位，后续独立 spec）。
    register_panel(
        "rtt", "activity", "RTT 调试",
        lambda app: PlaceholderPanel("rtt", "RTT 调试", "SEGGER RTT 实时通道（即将就绪）。"),
    )
    # 设置（占位）。
    register_panel(
        "settings", "settings", "设置",
        lambda app: PlaceholderPanel("settings", "设置", "应用设置与主题（即将就绪）。"),
    )


__all__ = ["register_default_panels", "reset_registry"]
