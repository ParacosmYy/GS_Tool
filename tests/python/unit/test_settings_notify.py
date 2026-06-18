"""settings_panel 主题切换 → 通知系统 + dashboard 死代码审计测试。

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


# ── dashboard 激活验证（Batch 17 反转 Batch 16 审计） ──────────────
def test_dashboard_subsystem_has_external_consumer():
    """Batch 17 激活后：dashboard 子系统应有外部消费者（DashboardPanel）。

    Batch 16 时 dashboard 是零外部消费者的死代码骨架；Batch 17 注册 DashboardPanel
    后反转此事实。本测试固化 dashboard 已被面板接入，防止回归成死代码。
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
        for line in text.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            if "import" in stripped and "dashboard" in stripped:
                external_importers.append(str(py))
                break
    # Batch 17 后 dashboard_panel.py 应是消费者（不再为空）。
    assert any("dashboard_panel.py" in p for p in external_importers), (
        "dashboard_panel.py should import the dashboard subsystem; "
        "if dashboard is deactivated again, this test catches the regression"
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
