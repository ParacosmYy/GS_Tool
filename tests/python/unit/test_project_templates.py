"""工程模板单元测试。"""
from __future__ import annotations
import pytest
from embeddebug.serial_station.project_templates import BuiltInTemplates, ProjectTemplate, TemplateManager

def test_template_validate():
    ProjectTemplate(name="x", protocol="raw_data").validate()

def test_template_empty_name_raises():
    with pytest.raises(ValueError):
        ProjectTemplate(name="").validate()

def test_template_roundtrip():
    t = ProjectTemplate(name="a", protocol="just_float", commands=["AT"])
    assert ProjectTemplate.from_dict(t.to_dict()) == t

def test_builtins_count():
    assert BuiltInTemplates.count() == 5

def test_builtins_names():
    assert "UART_AT_Commands" in BuiltInTemplates.names()

def test_builtins_get():
    assert BuiltInTemplates.get("JustFloat_Waveform").protocol == "just_float"

def test_builtins_get_miss():
    assert BuiltInTemplates.get("nope") is None

def test_manager_add_list_delete():
    m = TemplateManager()
    m.add(ProjectTemplate(name="b"))
    m.add(ProjectTemplate(name="a"))
    assert [t.name for t in m.list_templates()] == ["a", "b"]
    assert m.delete("a") is True
    assert m.get("a") is None

def test_manager_save_load(tmp_path):
    p = tmp_path / "t.json"
    m = TemplateManager(p)
    m.add(ProjectTemplate(name="x"))
    m.save()
    m2 = TemplateManager(p)
    m2.load()
    assert m2.get("x") is not None

def test_manager_seed():
    m = TemplateManager()
    assert m.seed_builtins() == 5
    assert m.seed_builtins() == 0


# ---- Batch 137: TemplateManager 边界（save/load/get/delete/validate） ----


def test_manager_save_without_path_raises_valueerror():
    """TemplateManager() 无 storage_path 时 save() 抛 ValueError。"""
    m = TemplateManager()
    with pytest.raises(ValueError, match="未指定存储路径"):
        m.save()


def test_manager_load_nonexistent_file_returns_empty(tmp_path):
    """load 不存在的文件 → 模板为空（不抛异常）。"""
    m = TemplateManager()
    m.load(tmp_path / "missing.json")
    assert m.list_templates() == []


def test_manager_load_without_path_or_storage_returns_empty():
    """TemplateManager() 无 path 时 load() 返回空（不抛异常）。"""
    m = TemplateManager()
    m.load()
    assert m.list_templates() == []


def test_manager_get_handles_none_name():
    """get(None) strip 后空字符串 → 返回 None（不抛异常）。"""
    m = TemplateManager()
    assert m.get(None) is None


def test_manager_get_strips_whitespace_in_name():
    """get('  x  ') strip 后查找 'x'。"""
    m = TemplateManager()
    m.add(ProjectTemplate(name="x"))
    assert m.get("  x  ") is not None
    assert m.get("  x  ").name == "x"


def test_manager_delete_unknown_returns_false():
    """delete 未知 name 返回 False（不抛异常）。"""
    m = TemplateManager()
    assert m.delete("ghost") is False


def test_manager_add_invokes_validate():
    """add 调用 validate，空 name 抛 ValueError。"""
    m = TemplateManager()
    with pytest.raises(ValueError):
        m.add(ProjectTemplate(name=""))


def test_manager_save_creates_parent_dirs(tmp_path):
    """save 自动创建父目录。"""
    deep_path = tmp_path / "nested" / "deep" / "templates.json"
    m = TemplateManager(deep_path)
    m.add(ProjectTemplate(name="x"))
    m.save()
    assert deep_path.exists()


def test_manager_save_with_explicit_path_overrides_storage(tmp_path):
    """save(path) 用显式路径而非 storage_path。"""
    storage = tmp_path / "storage.json"
    explicit = tmp_path / "explicit.json"
    m = TemplateManager(storage)
    m.add(ProjectTemplate(name="x"))
    m.save(explicit)
    assert explicit.exists()
    assert not storage.exists()


def test_manager_load_with_explicit_path(tmp_path):
    """load(path) 从显式路径加载，覆盖 storage_path。"""
    p1 = tmp_path / "p1.json"
    p2 = tmp_path / "p2.json"
    m1 = TemplateManager(p1)
    m1.add(ProjectTemplate(name="from_p1"))
    m1.save()

    m2 = TemplateManager(p2)
    m2.add(ProjectTemplate(name="from_p2"))
    m2.save()

    # m2 用 storage_path=p2，但显式 load p1
    m2.load(p1)
    assert m2.get("from_p1") is not None
    assert m2.get("from_p2") is None  # p2 的内容被覆盖


def test_manager_seed_builtins_does_not_overwrite_existing():
    """seed_builtins 不覆盖已添加的同名模板。"""
    m = TemplateManager()
    custom = ProjectTemplate(name="UART_AT_Commands", protocol="raw_data")
    m.add(custom)
    added = m.seed_builtins()
    # UART_AT_Commands 已存在，不重复添加
    assert added == 4  # 5 - 1
    # 原模板保留（未被 builtin 覆盖）
    assert m.get("UART_AT_Commands").protocol == "raw_data"
