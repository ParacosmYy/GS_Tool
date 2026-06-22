"""plugins/discovery 单元测试 — 插件发现 + 加载。

用 tmp_path 创建临时插件目录，测试 discover_plugins 扫描 + PluginInfo 提取 +
无效插件跳过 + load_plugin 重载。
"""

from __future__ import annotations

from pathlib import Path

from embeddebug.serial_station.plugins.discovery import (
    PluginInfo,
    discover_plugins,
    load_plugin,
)

_VALID_PLUGIN = '''
PLUGIN_NAME = "test_proto"
PLUGIN_VERSION = "1.2.3"
PLUGIN_TYPE = "protocol"

def register():
    return {"name": "test_proto"}
'''

_INVALID_TYPE_PLUGIN = '''
PLUGIN_NAME = "bad"
PLUGIN_TYPE = "unknown"
def register():
    pass
'''

_NO_REGISTER_PLUGIN = '''
PLUGIN_NAME = "noregister"
PLUGIN_TYPE = "protocol"
'''


def _write_plugin(root: Path, name: str, content: str) -> Path:
    """在 root 下创建 name/plugin.py。"""
    plugin_dir = root / name
    plugin_dir.mkdir(parents=True)
    (plugin_dir / "plugin.py").write_text(content, encoding="utf-8")
    return plugin_dir


def test_discover_plugins_empty_dir(tmp_path):
    """空目录返回空列表。"""
    assert discover_plugins(tmp_path) == []


def test_discover_plugins_nonexistent_dir(tmp_path):
    """不存在的目录返回空列表。"""
    assert discover_plugins(tmp_path / "nope") == []


def test_discover_valid_plugin(tmp_path):
    """有效插件被发现并提取元数据。"""
    _write_plugin(tmp_path, "my_proto", _VALID_PLUGIN)
    plugins = discover_plugins(tmp_path)
    assert len(plugins) == 1
    info = plugins[0]
    assert info.name == "test_proto"
    assert info.version == "1.2.3"
    assert info.plugin_type == "protocol"


def test_discover_skips_invalid_type(tmp_path):
    """PLUGIN_TYPE 不在 SUPPORTED 里跳过。"""
    _write_plugin(tmp_path, "bad_type", _INVALID_TYPE_PLUGIN)
    plugins = discover_plugins(tmp_path)
    assert plugins == []


def test_discover_skips_missing_register(tmp_path):
    """无 register 函数跳过。"""
    _write_plugin(tmp_path, "no_register", _NO_REGISTER_PLUGIN)
    plugins = discover_plugins(tmp_path)
    assert plugins == []


def test_discover_skips_non_plugin_dirs(tmp_path):
    """无 plugin.py 的子目录跳过。"""
    (tmp_path / "empty_dir").mkdir()
    (tmp_path / "readme.txt").write_text("not a plugin")
    assert discover_plugins(tmp_path) == []


def test_discover_multiple_plugins(tmp_path):
    """多个有效插件都发现。"""
    _write_plugin(tmp_path, "proto_a", _VALID_PLUGIN.replace("test_proto", "a"))
    _write_plugin(tmp_path, "proto_b", _VALID_PLUGIN.replace("test_proto", "b"))
    plugins = discover_plugins(tmp_path)
    assert len(plugins) == 2
    names = {p.name for p in plugins}
    assert names == {"a", "b"}


def test_load_plugin_returns_module(tmp_path):
    """load_plugin 返回模块对象。"""
    _write_plugin(tmp_path, "loadable", _VALID_PLUGIN)
    plugins = discover_plugins(tmp_path)
    assert len(plugins) == 1
    module = load_plugin(plugins[0])
    assert hasattr(module, "PLUGIN_NAME")
    assert module.PLUGIN_NAME == "test_proto"


def test_plugin_info_frozen():
    """PluginInfo 不可变。"""
    import pytest
    info = PluginInfo(name="x", version="1", plugin_type="protocol", path=Path("/x"), module_name="m")
    with pytest.raises((AttributeError, TypeError)):
        info.name = "y"
