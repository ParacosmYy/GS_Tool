"""插件系统（对齐 VOFA+ 开放可扩展插件体系）。

支持从指定目录发现并加载用户协议/控件插件（Python 模块），
不改动主线代码即可扩展能力。

插件契约（每个插件目录下一个 ``plugin.py``）：
- ``PLUGIN_NAME``: str — 插件显示名。
- ``PLUGIN_VERSION``: str — 版本。
- ``PLUGIN_TYPE``: str — "protocol" 或 "control"。
- ``register(registry)``: 把插件能力注册到对应 registry。

约束：本模块只依赖标准库，不 import PyQt（纯加载逻辑，便于单测）。
"""

from embeddebug.serial_station.plugins.discovery import (
    PLUGIN_FILE,
    PluginInfo,
    discover_plugins,
    load_plugin,
)
from embeddebug.serial_station.plugins.manager import PluginManager

__all__ = [
    "PLUGIN_FILE",
    "PluginInfo",
    "PluginManager",
    "discover_plugins",
    "load_plugin",
]
