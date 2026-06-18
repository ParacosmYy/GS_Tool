"""工程模板管理子包。"""
from __future__ import annotations
from embeddebug.serial_station.project_templates.builtins import BuiltInTemplates
from embeddebug.serial_station.project_templates.manager import TemplateManager
from embeddebug.serial_station.project_templates.template import ProjectTemplate
__all__ = ["ProjectTemplate", "TemplateManager", "BuiltInTemplates"]
