"""Batch 49-5: 连接/刷新按钮的 ProgressRing 加载态 helper。

独立模块，保持 ``connection_actions.py`` 体积可控（铁律 20）。

提供 ``set_button_loading(button, loading)``：替换原 ``_set_loading`` 的
纯文字「…」切换，改为在按钮内嵌一个 indeterminate ProgressRing（小尺寸，
居中覆盖），对齐 spec B49-5「连接按钮内嵌 ProgressRing 旋转动画」要求。

行为：
- ``loading=True``：缓存原文字到 ``button._loading_orig_text``；禁用按钮；
  创建（或复用）ProgressRing 子控件居中显示；隐藏按钮文字（设为空串，
  避免文字与 ring 重叠）。
- ``loading=False``：移除 ProgressRing；恢复原文字；启用按钮。

约束：只依赖 PyQt6 + controls，不访问 controller/transport/protocol。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QPushButton

from embeddebug.serial_station.ui.controls import ProgressRing

# 嵌入按钮的 ProgressRing 尺寸（sizeHint 默认 64×64 太大）。
_RING_SIZE = 18


def set_button_loading(button: QPushButton | None, loading: bool) -> None:
    """切换按钮加载态（内嵌 ProgressRing 旋转动画）。

    Args:
        button: 目标按钮（None 时安全跳过，对齐原 ``_set_loading`` 契约）。
        loading: True 进入加载态，False 退出。
    """

    if button is None:
        return
    if loading:
        _enter_loading(button)
    else:
        _exit_loading(button)


def _enter_loading(button: QPushButton) -> None:
    """进入加载态：缓存文字 + 禁用 + 内嵌 ProgressRing。"""

    button._loading_orig_text = button.text()
    button.setEnabled(False)
    button.setText("")
    ring = getattr(button, "_loading_ring", None)
    if ring is None:
        ring = ProgressRing(parent=button)
        ring.setObjectName("serialStationButtonLoadingRing")
        ring.setFixedSize(_RING_SIZE, _RING_SIZE)
        ring.setIndeterminate(True)
        button._loading_ring = ring
    # 居中放置 ring（按钮 resize 时由 installEventFilter 重定位，此处用 move）。
    _center_ring_on_button(button)
    ring.show()
    ring.raise_()
    from PyQt6.QtWidgets import QApplication
    QApplication.processEvents()


def _exit_loading(button: QPushButton) -> None:
    """退出加载态：移除 ProgressRing + 恢复文字 + 启用。"""

    button.setEnabled(True)
    orig = getattr(button, "_loading_orig_text", None)
    if orig is not None:
        button.setText(orig)
        button._loading_orig_text = None
    ring = getattr(button, "_loading_ring", None)
    if ring is not None:
        ring.hide()


def _center_ring_on_button(button: QPushButton) -> None:
    """把 ProgressRing 居中放置在按钮内（水平+垂直居中）。"""

    ring = getattr(button, "_loading_ring", None)
    if ring is None:
        return
    bw = button.width()
    bh = button.height()
    rw = ring.width()
    rh = ring.height()
    ring.move((bw - rw) // 2, (bh - rh) // 2)
