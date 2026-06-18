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
