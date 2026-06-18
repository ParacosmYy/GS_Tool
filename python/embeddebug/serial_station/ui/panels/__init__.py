"""模式面板注册入口。

集中注册所有功能模式到 ``mode_panel`` registry，AppShell 按注册顺序装配导航。
新增模式 = 在此 import + register_panel 一行。

注册顺序即导航栏从上到下顺序：串口 / OTA / RTT / CAN / BLE / 自动化 / 设置。
"""

from __future__ import annotations

from embeddebug.app.mode_panel import register_panel, reset_registry
from embeddebug.serial_station.ui.panels.placeholder_panel import PlaceholderPanel
from embeddebug.serial_station.ui.panels.serial_panel import SerialPanel


def _register_real_or_placeholder(
    mode_id: str, icon: str, label: str, desc: str, module: str, cls: str
) -> None:
    """注册真实面板；import 失败回退占位（保证导航完整不空屏）。

    占位面板会渲染对应 lucide icon（Batch 5 改进：替代朴素双 QLabel）。
    """

    try:
        factory = _import_factory(module, cls)
        register_panel(mode_id, icon, label, factory)
    except ImportError:
        register_panel(
            mode_id, icon, label,
            lambda app, _desc=desc, _icon=icon, _label=label:
                PlaceholderPanel(mode_id, _label, _desc, icon_name=_icon),
        )


def _import_factory(module: str, cls: str):
    """惰性 import 面板类，返回 lambda 工厂。"""

    import importlib
    mod = importlib.import_module(module)
    panel_cls = getattr(mod, cls)

    def factory(app):
        return panel_cls()

    return factory


def register_default_panels() -> None:
    """注册全部模式面板。

    幂等：重复调用不会重复注册（register_panel 内部按 mode_id 去重）。
    每个面板 try/except 容错：引擎或 UI 缺失时回退占位，保证导航不空屏。
    """

    reset_registry()
    # 串口（现有，第一个=默认模式）。
    register_panel("serial", "cable", "串口", lambda app: SerialPanel())
    # OTA（X/YMODEM 固件升级）。
    _register_real_or_placeholder(
        "ota", "upload", "OTA 升级", "XMODEM / YMODEM 固件升级（即将就绪）。",
        "embeddebug.serial_station.ui.panels.ota_panel", "OtaPanel",
    )
    # RTT（SEGGER 实时通道 + loopback 演示）。
    _register_real_or_placeholder(
        "rtt", "activity", "RTT 调试", "SEGGER RTT 实时通道（即将就绪）。",
        "embeddebug.serial_station.ui.panels.rtt_panel", "RttPanel",
    )
    # CAN（总线帧监视 + DBC 解码 + 发送）。
    _register_real_or_placeholder(
        "can", "network", "CAN 调试", "CAN / CAN-FD 帧监视与发送（即将就绪）。",
        "embeddebug.serial_station.ui.panels.can_panel", "CanPanel",
    )
    # BLE（蓝牙低功耗 + GATT 树 + 读写 + notify）。
    _register_real_or_placeholder(
        "ble", "bluetooth", "BLE", "蓝牙低功耗调试（即将就绪）。",
        "embeddebug.serial_station.ui.panels.ble_panel", "BlePanel",
    )
    # 自动化（规则触发器 + 手动触发 + 日志）。
    _register_real_or_placeholder(
        "automation", "zap", "自动化", "规则触发器与动作（即将就绪）。",
        "embeddebug.serial_station.ui.panels.automation_panel", "AutomationPanel",
    )
    # 仪表盘（拖拽式控件仪表盘，Batch 17 激活 dashboard 子系统）。
    _register_real_or_placeholder(
        "dashboard", "layout-dashboard", "仪表盘", "拖拽式控件仪表盘（即将就绪）。",
        "embeddebug.serial_station.ui.panels.dashboard_panel", "DashboardPanel",
    )
    # 设置（主题切换 + 快捷键 + 关于）。
    _register_real_or_placeholder(
        "settings", "settings", "设置", "应用设置与主题（即将就绪）。",
        "embeddebug.serial_station.ui.panels.settings_panel", "SettingsPanel",
    )


__all__ = ["register_default_panels", "reset_registry"]
