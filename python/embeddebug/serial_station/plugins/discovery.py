"""插件发现与加载。

扫描指定目录下的子目录，每个含 ``plugin.py`` 的子目录视为一个插件。
加载 ``plugin.py`` 模块并提取元数据（PLUGIN_NAME/VERSION/TYPE/register）。

约束：本模块只依赖标准库，不 import PyQt。
"""

from __future__ import annotations

import importlib.util
import sys
from dataclasses import dataclass
from pathlib import Path

PLUGIN_FILE = "plugin.py"
SUPPORTED_PLUGIN_TYPES = ("protocol", "control")


@dataclass(frozen=True)
class PluginInfo:
    """插件元数据。"""

    name: str
    version: str
    plugin_type: str
    path: Path
    module_name: str


def discover_plugins(root: str | Path) -> list[PluginInfo]:
    """扫描 root 下的插件目录，返回 PluginInfo 列表。

    每个子目录含 ``plugin.py`` 视为一个插件。无效插件跳过（不抛异常）。
    """

    root_path = Path(root)
    if not root_path.is_dir():
        return []
    plugins: list[PluginInfo] = []
    for child in sorted(root_path.iterdir()):
        if not child.is_dir():
            continue
        plugin_file = child / PLUGIN_FILE
        if not plugin_file.is_file():
            continue
        info = _inspect_plugin(child, plugin_file)
        if info is not None:
            plugins.append(info)
    return plugins


def _inspect_plugin(plugin_dir: Path, plugin_file: Path) -> PluginInfo | None:
    """加载 plugin.py 提取元数据，无效返回 None。"""

    module_name = f"_embeddebug_plugin_{plugin_dir.name}"
    try:
        spec = importlib.util.spec_from_file_location(module_name, plugin_file)
        if spec is None or spec.loader is None:
            return None
        module = importlib.util.module_from_spec(spec)
        sys.modules[module_name] = module
        spec.loader.exec_module(module)
    except Exception:
        return None
    name = getattr(module, "PLUGIN_NAME", plugin_dir.name)
    version = getattr(module, "PLUGIN_VERSION", "0.0.0")
    plugin_type = getattr(module, "PLUGIN_TYPE", "")
    if plugin_type not in SUPPORTED_PLUGIN_TYPES:
        return None
    if not hasattr(module, "register"):
        return None
    return PluginInfo(
        name=str(name),
        version=str(version),
        plugin_type=plugin_type,
        path=plugin_file,
        module_name=module_name,
    )


def load_plugin(info: PluginInfo) -> object:
    """重新加载插件模块（用于热重载），返回模块对象。"""

    module = sys.modules.get(info.module_name)
    if module is not None:
        return module
    spec = importlib.util.spec_from_file_location(info.module_name, info.path)
    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load plugin: {info.name}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[info.module_name] = module
    spec.loader.exec_module(module)
    return module
