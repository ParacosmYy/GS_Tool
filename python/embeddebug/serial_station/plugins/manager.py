"""插件管理器。

管理插件发现、加载、启用/禁用/重载，并把插件能力注册到对应 registry。

用法：
    manager = PluginManager(plugin_root="plugins_user")
    manager.discover()
    manager.load_all(protocol_registry, control_registry)
    manager.reload("my_protocol")

约束：本模块只依赖标准库 + discovery，不 import PyQt。
"""

from __future__ import annotations

from collections.abc import Callable
from pathlib import Path

from embeddebug.serial_station.plugins.discovery import (
    PluginInfo,
    discover_plugins,
    load_plugin,
)


class PluginManager:
    """插件生命周期管理。"""

    def __init__(self, plugin_root: str | Path) -> None:
        self._root = Path(plugin_root)
        self._plugins: dict[str, PluginInfo] = {}
        self._loaded: set[str] = set()
        self._disabled: set[str] = set()

    @property
    def root(self) -> Path:
        return self._root

    def discover(self) -> list[PluginInfo]:
        """扫描插件目录，返回发现的插件列表。"""

        infos = discover_plugins(self._root)
        self._plugins = {info.name: info for info in infos}
        return infos

    def plugins(self) -> list[PluginInfo]:
        return list(self._plugins.values())

    def plugin_names(self) -> list[str]:
        return sorted(self._plugins)

    def is_loaded(self, name: str) -> bool:
        return name in self._loaded

    def is_disabled(self, name: str) -> bool:
        return name in self._disabled

    def enable(self, name: str) -> None:
        self._disabled.discard(name)

    def disable(self, name: str) -> None:
        self._disabled.add(name)
        self._loaded.discard(name)

    def load_all(
        self,
        protocol_registry: object | None = None,
        control_registry: object | None = None,
    ) -> list[str]:
        """加载所有非禁用插件，返回成功加载的插件名列表。"""

        loaded: list[str] = []
        for info in self._plugins.values():
            if info.name in self._disabled:
                continue
            registry = self._select_registry(info, protocol_registry, control_registry)
            if self._load_one(info, registry):
                loaded.append(info.name)
        return loaded

    def load(self, name: str, registry: object | None = None) -> bool:
        """加载单个插件。"""

        info = self._plugins.get(name)
        if info is None or name in self._disabled:
            return False
        return self._load_one(info, registry)

    def reload(self, name: str, registry: object | None = None) -> bool:
        """重载单个插件（先移除已加载标记再重新加载）。"""

        self._loaded.discard(name)
        return self.load(name, registry)

    def _load_one(self, info: PluginInfo, registry: object | None) -> bool:
        try:
            module = load_plugin(info)
            register: Callable[[object], None] | None = getattr(module, "register", None)
            if register is None or registry is None:
                return False
            register(registry)
            self._loaded.add(info.name)
            return True
        except Exception:
            return False

    @staticmethod
    def _select_registry(
        info: PluginInfo,
        protocol_registry: object | None,
        control_registry: object | None,
    ) -> object | None:
        if info.plugin_type == "protocol":
            return protocol_registry
        if info.plugin_type == "control":
            return control_registry
        return None
