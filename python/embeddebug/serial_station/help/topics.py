"""帮助主题、条目数据结构与静态帮助内容。"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum


class HelpTopic(Enum):
    GETTING_STARTED = "getting_started"
    PROTOCOLS = "protocols"
    SHORTCUTS = "shortcuts"
    TROUBLESHOOTING = "troubleshooting"
    ABOUT = "about"


@dataclass(frozen=True)
class HelpEntry:
    """单个帮助条目。"""
    topic: HelpTopic
    title: str
    content: str
    tags: list[str] = field(default_factory=list)


PROTOCOL_HELP: dict[str, HelpEntry] = {
    "raw_data": HelpEntry(
        topic=HelpTopic.PROTOCOLS,
        title="RawData 原始数据协议",
        content="RawData 协议对串口字节做透传。接收: UTF-8 解码显示。发送: 字符串或 hex。不产生 measurement 事件。",
        tags=["raw_data", "透传", "文本", "AT"],
    ),
    "fire_water": HelpEntry(
        topic=HelpTopic.PROTOCOLS,
        title="FireWater 火水协议",
        content="FireWater 按行解析逗号分隔数值。表头: name1,name2。测量行: v1,v2。单行最大 32 通道。",
        tags=["fire_water", "csv", "采样", "多通道"],
    ),
    "just_float": HelpEntry(
        topic=HelpTopic.PROTOCOLS,
        title="JustFloat 协议",
        content="JustFloat 解析小端 float32 帧。帧尾: 00 00 80 7F。payload 须为 4 字节整数倍。适用 VOFA+ 高速波形。",
        tags=["just_float", "vofa", "float32", "波形"],
    ),
}

SHORTCUT_HELP: list[dict[str, str]] = [
    {"key": "Ctrl+Enter", "action": "发送当前命令"},
    {"key": "Ctrl+L", "action": "清空接收区"},
    {"key": "Ctrl+P", "action": "打开命令面板"},
    {"key": "F1", "action": "打开帮助系统"},
    {"key": "F11", "action": "切换全屏"},
    {"key": "Esc", "action": "关闭弹层"},
]
