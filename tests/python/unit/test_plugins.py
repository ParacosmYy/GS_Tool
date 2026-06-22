"""B8 插件系统测试：发现 / 加载 / 管理。"""

from __future__ import annotations

import os
from pathlib import Path
from textwrap import dedent

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.plugins import (
    PLUGIN_FILE,
    PluginManager,
    discover_plugins,
    load_plugin,
)


def _write_protocol_plugin(root: Path, name: str = "my_proto") -> Path:
    """在 root 下写一个示例协议插件，返回插件目录。"""

    plugin_dir = root / name
    plugin_dir.mkdir(parents=True, exist_ok=True)
    (plugin_dir / PLUGIN_FILE).write_text(
        dedent(
            """
            PLUGIN_NAME = "my_proto"
            PLUGIN_VERSION = "1.2.3"
            PLUGIN_TYPE = "protocol"

            class StubProtocol:
                name = "my_proto"

            def register(registry):
                registry.register("my_proto", StubProtocol)
            """
        ).strip(),
        encoding="utf-8",
    )
    return plugin_dir


def _write_control_plugin(root: Path, name: str = "my_ctrl") -> Path:
    plugin_dir = root / name
    plugin_dir.mkdir(parents=True, exist_ok=True)
    (plugin_dir / PLUGIN_FILE).write_text(
        dedent(
            """
            PLUGIN_NAME = "my_ctrl"
            PLUGIN_VERSION = "0.1.0"
            PLUGIN_TYPE = "control"

            def register(registry):
                registry["my_ctrl"] = "stub_widget"
            """
        ).strip(),
        encoding="utf-8",
    )
    return plugin_dir


class _FakeRegistry:
    def __init__(self) -> None:
        self.entries: dict[str, object] = {}

    def register(self, name: str, factory: object) -> None:
        self.entries[name] = factory


def test_discover_finds_protocol_plugin(tmp_path):
    _write_protocol_plugin(tmp_path)
    plugins = discover_plugins(tmp_path)
    assert len(plugins) == 1
    assert plugins[0].name == "my_proto"
    assert plugins[0].version == "1.2.3"
    assert plugins[0].plugin_type == "protocol"


def test_discover_finds_multiple_plugins(tmp_path):
    _write_protocol_plugin(tmp_path)
    _write_control_plugin(tmp_path)
    plugins = discover_plugins(tmp_path)
    names = {p.name for p in plugins}
    assert names == {"my_proto", "my_ctrl"}


def test_discover_skips_invalid_plugin_type(tmp_path):
    plugin_dir = tmp_path / "bad"
    plugin_dir.mkdir()
    (plugin_dir / PLUGIN_FILE).write_text(
        dedent(
            """
            PLUGIN_NAME = "bad"
            PLUGIN_TYPE = "unknown"
            def register(r): pass
            """
        ).strip(),
        encoding="utf-8",
    )
    assert discover_plugins(tmp_path) == []


def test_discover_skips_missing_register(tmp_path):
    plugin_dir = tmp_path / "noreg"
    plugin_dir.mkdir()
    (plugin_dir / PLUGIN_FILE).write_text(
        dedent(
            """
            PLUGIN_NAME = "noreg"
            PLUGIN_TYPE = "protocol"
            """
        ).strip(),
        encoding="utf-8",
    )
    assert discover_plugins(tmp_path) == []


def test_discover_skips_syntax_error(tmp_path):
    plugin_dir = tmp_path / "broken"
    plugin_dir.mkdir()
    (plugin_dir / PLUGIN_FILE).write_text("def broken(:\n", encoding="utf-8")
    assert discover_plugins(tmp_path) == []


def test_discover_empty_dir(tmp_path):
    assert discover_plugins(tmp_path) == []


def test_discover_nonexistent_dir(tmp_path):
    assert discover_plugins(tmp_path / "nope") == []


def test_load_plugin_returns_module(tmp_path):
    _write_protocol_plugin(tmp_path)
    info = discover_plugins(tmp_path)[0]
    module = load_plugin(info)
    assert hasattr(module, "register")
    assert hasattr(module, "PLUGIN_NAME")


def test_plugin_manager_discover(tmp_path):
    _write_protocol_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    plugins = manager.discover()
    assert len(plugins) == 1
    assert "my_proto" in manager.plugin_names()


def test_plugin_manager_load_all_registers(tmp_path):
    _write_protocol_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    manager.discover()
    registry = _FakeRegistry()
    loaded = manager.load_all(protocol_registry=registry)
    assert loaded == ["my_proto"]
    assert "my_proto" in registry.entries
    assert manager.is_loaded("my_proto")


def test_plugin_manager_load_all_skips_disabled(tmp_path):
    _write_protocol_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    manager.discover()
    manager.disable("my_proto")
    registry = _FakeRegistry()
    loaded = manager.load_all(protocol_registry=registry)
    assert loaded == []
    assert not manager.is_loaded("my_proto")
    assert manager.is_disabled("my_proto")


def test_plugin_manager_enable_disable(tmp_path):
    _write_protocol_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    manager.discover()
    manager.disable("my_proto")
    assert manager.is_disabled("my_proto") is True
    manager.enable("my_proto")
    assert manager.is_disabled("my_proto") is False


def test_plugin_manager_reload(tmp_path):
    _write_protocol_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    manager.discover()
    registry = _FakeRegistry()
    manager.load_all(protocol_registry=registry)
    assert manager.is_loaded("my_proto")
    # reload 清除 loaded 标记后重新加载。
    assert manager.reload("my_proto", registry=registry) is True
    assert manager.is_loaded("my_proto")


def test_plugin_manager_load_unknown(tmp_path):
    manager = PluginManager(tmp_path)
    manager.discover()
    assert manager.load("does-not-exist") is False


def test_plugin_manager_load_missing_registry(tmp_path):
    _write_protocol_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    manager.discover()
    # 不传 registry，加载应失败（register 无目标）。
    assert manager.load("my_proto") is False


def test_plugin_manager_control_type_routes_to_control_registry(tmp_path):
    _write_control_plugin(tmp_path)
    manager = PluginManager(tmp_path)
    manager.discover()
    control_registry: dict[str, object] = {}
    protocol_registry = _FakeRegistry()
    loaded = manager.load_all(
        protocol_registry=protocol_registry, control_registry=control_registry
    )
    assert loaded == ["my_ctrl"]
    assert "my_ctrl" in control_registry
    assert protocol_registry.entries == {}
