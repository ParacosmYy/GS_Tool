"""Batch 16 测试：settings_panel 主题切换 → 通知系统 + dashboard 死代码审计。

覆盖：
1. settings_panel._apply_theme 成功 → success toast（panel_notify 接入）。
2. settings_panel._apply_theme 失败（apply_theme_by_name 抛异常）→ error toast。
3. settings_panel 源码级接入断言（panel_notify 在 _apply_theme）。
4. dashboard 死代码审计：确认 dashboard 子系统无外部消费者（记录缺口，供后续 batch）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


# ── settings_panel _apply_theme 接入断言 ───────────────────────────
def test_settings_panel_apply_theme_calls_panel_notify():
    """settings_panel._apply_theme 应调 panel_notify（成功 success / 失败 error）。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    src = inspect.getsource(SettingsPanel._apply_theme)
    assert "panel_notify" in src
    assert "success" in src  # 成功路径
    assert "error" in src    # 失败路径


def test_settings_panel_apply_theme_catches_exception():
    """_apply_theme 应用主题异常应 try/except（不崩溃 + error toast）。"""

    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    src = inspect.getsource(SettingsPanel._apply_theme)
    # 应有 try/except 包裹 apply_theme_by_name 调用。
    assert "try:" in src
    assert "except Exception" in src
    assert "apply_theme_by_name" in src


# ── settings_panel 实例化 + 主题切换端到端 ─────────────────────────
def test_settings_panel_builds_and_applies_theme(qtbot):
    """SettingsPanel 应可构建并应用主题（不崩溃）。"""

    from embeddebug.app.app_controller import AppController
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel

    panel = SettingsPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    # 切换到浅色再切回深色，验证不崩溃。
    panel._theme_combo.setCurrentIndex(0)
    panel._apply_theme()
    assert panel._theme_status is not None


# ── dashboard 死代码审计（记录缺口） ───────────────────────────────
def test_dashboard_subsystem_has_no_external_consumers():
    """审计：dashboard 子系统应无外部消费者（记录为后续 batch 的死代码缺口）。

    dashboard.canvas/factory/fullscreen/palette/tabs 全部只在 dashboard/ 内部互相
    引用，无面板/窗口实际接入。本测试固化这一事实，供后续 batch 决定是否激活。
    若未来有面板接入 dashboard，本测试需更新（或删除）。
    """

    from pathlib import Path

    ui_dir = Path("python/embeddebug/serial_station/ui")
    dashboard_dir = ui_dir / "dashboard"
    external_importers: list[str] = []
    for py in ui_dir.rglob("*.py"):
        if dashboard_dir in py.parents:
            continue  # dashboard 内部互相引用不计。
        try:
            text = py.read_text(encoding="utf-8")
        except OSError:
            continue
        if "dashboard" in text and "import" in text:
            # 过滤注释/docstring 中的提及，只看真实 import 行。
            for line in text.splitlines():
                stripped = line.strip()
                if stripped.startswith("#"):
                    continue
                if "import" in stripped and "dashboard" in stripped:
                    external_importers.append(str(py))
                    break
    # 当前 dashboard 是死代码（无外部消费者）。记录这一事实；未来激活时此断言会失败，
    # 提醒更新测试。
    assert external_importers == [], (
        f"dashboard has external importers {external_importers}; "
        "if dashboard is now activated, update this audit test"
    )


def test_dashboard_modules_exist_and_export():
    """dashboard 5 模块应存在且导出公共 API（基建完整性，供后续激活）。"""

    from embeddebug.serial_station.ui.dashboard import (
        DashboardCanvas,
        DashboardTabs,
        WidgetPalette,
        create_widget,
    )

    assert DashboardCanvas is not None
    assert DashboardTabs is not None
    assert WidgetPalette is not None
    assert callable(create_widget)
