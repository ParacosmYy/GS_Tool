"""快捷键定义层：分类枚举 + 值对象 + 默认快捷键表。"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class ShortcutCategory(Enum):
    """快捷键分类。"""

    FILE = "file"
    EDIT = "edit"
    VIEW = "view"
    TRANSPORT = "transport"
    HELP = "help"


@dataclass(frozen=True)
class ShortcutDef:
    """单条快捷键的不可变定义。"""

    id: str
    category: ShortcutCategory
    default_key_sequence: str
    description: str
    callback_name: str


DEFAULT_SHORTCUTS: list[ShortcutDef] = [
    ShortcutDef("connect", ShortcutCategory.TRANSPORT, "Ctrl+Shift+C", "连接串口", "connect"),
    ShortcutDef("disconnect", ShortcutCategory.TRANSPORT, "Ctrl+Shift+D", "断开串口", "disconnect"),
    ShortcutDef("send", ShortcutCategory.TRANSPORT, "Ctrl+Return", "发送当前输入", "send"),
    ShortcutDef("clear", ShortcutCategory.EDIT, "Ctrl+L", "清空日志", "clear"),
    ShortcutDef("refresh", ShortcutCategory.EDIT, "Ctrl+R", "刷新串口列表", "refresh"),
    ShortcutDef("save_log", ShortcutCategory.FILE, "Ctrl+S", "保存日志到文件", "save_log"),
    ShortcutDef("replay", ShortcutCategory.TRANSPORT, "Ctrl+Shift+R", "回放上次脚本", "replay"),
    ShortcutDef("command_palette", ShortcutCategory.VIEW, "Ctrl+P", "打开命令面板", "open_command_palette"),
    ShortcutDef("toggle_theme", ShortcutCategory.VIEW, "Ctrl+Shift+T", "切换明暗主题", "toggle_theme"),
    ShortcutDef("toggle_sidebar", ShortcutCategory.VIEW, "Ctrl+B", "折叠/展开侧边栏", "toggle_sidebar"),
    ShortcutDef("zoom_in", ShortcutCategory.VIEW, "Ctrl++", "放大字体", "zoom_in"),
    ShortcutDef("zoom_out", ShortcutCategory.VIEW, "Ctrl+-", "缩小字体", "zoom_out"),
    ShortcutDef("fullscreen", ShortcutCategory.VIEW, "F11", "切换全屏", "toggle_fullscreen"),
    ShortcutDef("about", ShortcutCategory.HELP, "F1", "关于 EmbedDebug", "about"),
    ShortcutDef("quit", ShortcutCategory.FILE, "Ctrl+Q", "退出应用", "quit"),
]
