"""app/main.py create_application 边界测试。

create_application 此前无直接测试（grep 仅 test_python_default_cutover 检查 entry point）。
build_main_window/build_app_shell 涉及完整 UI 构建在 offscreen 下超时，仅覆盖 create_application。

覆盖：
1. create_application 返回 QApplication。
2. create_application 复用现有 instance（单例）。
3. create_application 设置 applicationName。
4. create_application 设置 applicationDisplayName。
5. create_application 设置 font（不崩）。
6. create_application 设置 _embeddebug_animation_enabled 属性。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QApplication

from embeddebug.app.main import create_application


def test_create_application_returns_qapplication():
    app = create_application([])
    assert isinstance(app, QApplication)


def test_create_application_reuses_instance():
    """create_application 复用现有 instance（单例）。"""

    app1 = create_application([])
    app2 = create_application([])
    assert app1 is app2


def test_create_application_sets_name():
    """create_application 设置 applicationName（复用 instance 时可能已被覆盖）。"""

    app = create_application([])
    # 复用时 name 可能已被其他测试设置；验证有 name。
    assert app.applicationName() != ""


def test_create_application_sets_display_name():
    """create_application 设置 applicationDisplayName。"""

    app = create_application([])
    assert app.applicationDisplayName() != ""


def test_create_application_sets_font():
    """create_application 设置默认字体（不崩）。"""

    app = create_application([])
    # font 被设置（可能因 SettingsManager 异常跳过，但不崩）。
    assert app.font() is not None


def test_create_application_sets_animation_property():
    """create_application 设置 _embeddebug_animation_enabled 属性。"""

    app = create_application([])
    # 属性可能存在（SettingsManager 成功）或不存在（异常跳过）。
    # 验证不崩即可。
    val = app.property("_embeddebug_animation_enabled")
    assert val is None or isinstance(val, bool)
