"""域面板入场动画 helper（Batch 7-6）。

诊断报告：7 个域面板（ble/can/rtt/ota/automation/settings）未接入 stagger 入场动画，
仅 PlaceholderPanel 有入场动画。本 helper 提供 ``play_panel_enter``，让各域面板
on_enter 时对顶层 widget 做 card_enter（淡入+上滑）入场。

设计要点：
- 每个面板持有 ``_enter_anims`` 列表（防 GC），on_enter 调 play_panel_enter。
- on_leave 调 stop_panel_enter 停止进行中的动画。
- 动画失败不阻塞（try/except），与 app_shell 的容错一致。

约束：只依赖 PyQt6 + panel_animations，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.panel_animations import card_enter


def play_panel_enter(panel: object) -> None:
    """播放面板入场动画（card_enter：淡入+上滑），动画引用存 panel._enter_anims。

    Args:
        panel: 域面板实例，需有 ``_widget: QWidget`` 和 ``_enter_anims: list`` 属性。
    """

    widget = getattr(panel, "_widget", None)
    if widget is None or not isinstance(widget, QWidget):
        return
    # 停止上一组动画（连续快速切换时）。
    stop_panel_enter(panel)
    try:
        anims = card_enter(widget)
        setattr(panel, "_enter_anims", anims)
        for anim in anims:
            anim.start()
    except Exception:
        # 动画是锦上添花，失败不阻塞面板进入。
        setattr(panel, "_enter_anims", [])


def stop_panel_enter(panel: object) -> None:
    """停止面板进行中的入场动画。"""

    anims = getattr(panel, "_enter_anims", None)
    if not anims:
        return
    for anim in anims:
        try:
            anim.stop()
        except Exception:
            pass
    setattr(panel, "_enter_anims", [])
