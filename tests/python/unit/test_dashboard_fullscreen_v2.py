"""DashboardPanel 双击全屏接线测试（激活 WidgetFullscreenHandler 死代码）。

覆盖：
1. DashboardPanel 构建后初始画布已接 item_added（fullscreen wiring 就绪）。
2. 放置控件 → 新控件装了双击全屏 handler（_fullscreen_handlers 非空）。
3. 放置控件 → mouseDoubleClickEvent 被替换为全屏闭包。
4. 源码级接入断言（attach_double_click_fullscreen 在 DashboardPanel 调用）。
5. fullscreen.py 现已有外部消费者（不再死代码）。
6. WidgetFullscreenHandler 行为（enter/restore/toggle）。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtCore import QPoint

from embeddebug.app.app_controller import AppController


def _make_panel(qtbot):
    from embeddebug.serial_station.ui.panels.dashboard_panel import DashboardPanel

    panel = DashboardPanel()
    widget = panel.build(AppController())
    qtbot.addWidget(widget)
    return panel


# ── 接线 ───────────────────────────────────────────────────────────
def test_panel_builds_with_fullscreen_handlers_list(qtbot):
    """DashboardPanel 构建应初始化 _fullscreen_handlers（fullscreen wiring 就绪）。"""

    panel = _make_panel(qtbot)
    assert hasattr(panel, "_fullscreen_handlers")
    assert panel._fullscreen_handlers == []


def test_initial_canvas_wired_to_item_added(qtbot):
    """初始画布应已接 item_added 信号（id 在 _wired_canvases 中）。"""

    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    assert id(canvas) in panel._wired_canvases


def test_adding_widget_installs_fullscreen_handler(qtbot):
    """放置控件 → 新控件装双击全屏 handler（_fullscreen_handlers 增长）。"""

    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    before = len(panel._fullscreen_handlers)
    canvas.add_widget_at("led", QPoint(20, 20))
    # item_added 信号触发 _on_item_added_fullscreen → 装 handler。
    assert len(panel._fullscreen_handlers) == before + 1


def test_added_widget_has_double_click_override(qtbot):
    """放置后控件的 mouseDoubleClickEvent 应被替换为全屏闭包。"""

    panel = _make_panel(qtbot)
    canvas = panel._tabs.current_canvas()
    canvas.add_widget_at("gauge", QPoint(40, 40))
    item = next(iter(canvas.items.values()))
    # __dict__ 应含被替换的 mouseDoubleClickEvent（实例属性）。
    assert "mouseDoubleClickEvent" in item.widget.__dict__


# ── 源码接入断言 ───────────────────────────────────────────────────
def test_dashboard_panel_calls_attach_double_click_fullscreen():
    """DashboardPanel 源码应调 attach_double_click_fullscreen（死代码激活）。"""

    from embeddebug.serial_station.ui.panels import dashboard_panel

    src = inspect.getsource(dashboard_panel)
    assert "attach_double_click_fullscreen" in src
    assert "item_added" in src
    assert "_on_item_added_fullscreen" in src


def test_fullscreen_module_now_has_external_consumer():
    """fullscreen.py 现应有外部消费者（dashboard_panel），不再是死代码。

    Batch 17 前 dashboard 全模块零外部消费者；Batch 18 接 fullscreen 后反转。
    """

    from pathlib import Path

    ui = Path("python/embeddebug/serial_station/ui")
    fullscreen_consumers: list[str] = []
    for py in ui.rglob("*.py"):
        if py.name == "fullscreen.py" or "dashboard" in py.parts and py.name != "dashboard_panel.py":
            # fullscreen.py 自身 + dashboard 内部其他模块不计；只看 dashboard_panel + 外部。
            if py.name != "dashboard_panel.py":
                continue
        try:
            text = py.read_text(encoding="utf-8")
        except OSError:
            continue
        if "attach_double_click_fullscreen" in text or "WidgetFullscreenHandler" in text:
            fullscreen_consumers.append(str(py))
    assert any("dashboard_panel.py" in p for p in fullscreen_consumers), (
        "dashboard_panel.py should consume fullscreen.py; "
        "regression to dead code would fail this"
    )


# ── WidgetFullscreenHandler 行为 ───────────────────────────────────
def test_fullscreen_handler_toggle_cycle(qtbot):
    """WidgetFullscreenHandler.enter/restore 应正确切换全屏态。"""

    from PyQt6.QtWidgets import QLabel
    from embeddebug.serial_station.ui.dashboard.fullscreen import WidgetFullscreenHandler

    host = QLabel("host")
    qtbot.addWidget(host)
    host.resize(400, 300)
    widget = QLabel("child", host)
    widget.setGeometry(10, 10, 50, 20)
    qtbot.addWidget(widget)

    handler = WidgetFullscreenHandler()
    assert not handler.is_fullscreen
    handler.enter(widget, host=host)
    assert handler.is_fullscreen
    handler.restore()
    assert not handler.is_fullscreen


def test_fullscreen_handler_toggle_returns_state(qtbot):
    """toggle 应返回切换后是否全屏（True 进入，False 恢复）。"""

    from PyQt6.QtWidgets import QLabel
    from embeddebug.serial_station.ui.dashboard.fullscreen import WidgetFullscreenHandler

    host = QLabel("host")
    qtbot.addWidget(host)
    host.resize(400, 300)
    widget = QLabel("child", host)
    widget.setGeometry(10, 10, 50, 20)
    qtbot.addWidget(widget)

    handler = WidgetFullscreenHandler()
    assert handler.toggle(widget, host=host) is True
    assert handler.toggle(widget, host=host) is False
