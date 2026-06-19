"""主题切换动作（Batch 13）—— 快捷键 Ctrl+Shift+T 的落地逻辑。

把 MainWindow/快捷键路径与 ThemeSwitcher 解耦：本模块提供 ``toggle_theme(owner)``
执行「windowOpacity 暗淡 + 回亮」过渡 + ``ThemeSwitcher.toggle()``，使快捷键切换
与设置页点选观感一致（Batch 10 的过渡动画对齐）。

为何独立成 *_actions 模块（而非内联 MainWindow）：
- 守铁律 #3（MainWindow 只做装配/导航，不写业务逻辑）。
- 对齐既有 ``session_actions`` / ``log_actions`` / ``connection_actions`` 的委托范式
  （MainWindow._xxx -> xxx_actions.xxx(self)），架构测试（
  test_serial_station_ui_architecture）按此模式校验。

约束：只依赖 PyQt6 + theme 子包，不访问 controller/transport。
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from PyQt6.QtWidgets import QApplication, QWidget

if TYPE_CHECKING:
    from embeddebug.serial_station.ui.main_window import SerialStationMainWindow


def toggle_theme(owner: "SerialStationMainWindow | QWidget") -> None:
    """切换深/浅主题，带 windowOpacity 暗淡+回亮过渡动画。

    在过渡透明度谷值时执行 ``ThemeSwitcher.toggle()``（换 QSS + 落盘 theme，
    Batch 12 的持久化在此自动触发）。防重入：上次过渡未完成时同步执行 toggle
    （不丢操作）。
    """

    app = QApplication.instance()
    if app is None:
        return

    from embeddebug.serial_station.ui.theme.theme_switcher import ThemeSwitcher
    from embeddebug.serial_station.ui.theme.theme_transition import transition_theme

    switcher = ThemeSwitcher(app)
    # 同步 switcher 内部 _current 到 ThemeManager 实际状态（toggle 依赖它判断方向）。
    from embeddebug.serial_station.ui.theme.manager import ThemeManager

    switcher._current = ThemeManager().current_theme or switcher._current

    def apply_fn() -> None:
        switcher.toggle()

    transition_theme(app, apply_fn)


__all__ = ["toggle_theme"]
