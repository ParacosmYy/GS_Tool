"""Minimal PyQt application entry for the Python migration lane."""

from __future__ import annotations

import argparse
import os
import sys
from collections.abc import Sequence

from PyQt6.QtCore import QCoreApplication, QEvent
from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.main_window import SerialStationMainWindow


def create_application(argv: Sequence[str] | None = None) -> QApplication:
    """Create or reuse the process QApplication."""

    existing = QApplication.instance()
    if existing is not None:
        return existing

    app = QApplication(list(argv) if argv is not None else list(sys.argv))
    app.setApplicationName("EmbedDebugPy")
    app.setApplicationDisplayName("EmbedDebug")
    # Batch 11/12: 启动期从磁盘恢复上次保存的主题（深/浅）+ 强调色，必须在应用
    # 主题之前，使首帧 QSS 即带上正确 theme + accent recolor。走
    # theme_switcher.apply_theme_by_name（注入 accent recolor；manager.apply_theme
    # 只输出 cyan QSS）。
    from embeddebug.serial_station.ui.theme.accents import restore_active_accent
    from embeddebug.serial_station.ui.theme.theme_store import load_theme_id
    from embeddebug.serial_station.ui.theme.theme_switcher import apply_theme_by_name

    restore_active_accent()
    persisted_theme = load_theme_id()
    apply_theme_by_name(app, persisted_theme)
    # Batch 23: 启动期应用 SettingsManager 中持久化的字体大小；动画开关存到
    # app 属性供动画工厂查询（默认 True，关掉时各动画工厂应跳过创建动画）。
    try:
        from PyQt6.QtGui import QFont

        from embeddebug.serial_station.services.settings_service import SettingsManager

        settings = SettingsManager.instance().get()
        app.setFont(QFont("Microsoft YaHei UI", settings.font_point))
        app.setProperty("_embeddebug_animation_enabled", settings.animation_enabled)
    except Exception:  # noqa: BLE001  偏好应用失败不阻塞启动
        pass
    return app


def build_main_window() -> SerialStationMainWindow:
    """Build the single-window serial station (legacy/compat path).

    保留为向后兼容入口：现有 smoke 测试与单模式启动走此路径。
    多模式应用入口见 ``build_app_shell``。
    """

    return SerialStationMainWindow()


def build_app_shell():
    """Build the multi-mode application shell (nav rail + switchable panels).

    注册首批模式面板（串口 / OTA / RTT / 设置）并返回 AppShell。
    """

    from embeddebug.app.app_shell import AppShell
    from embeddebug.serial_station.ui.panels import register_default_panels

    register_default_panels()
    return AppShell()


def main(argv: Sequence[str] | None = None) -> int:
    """Run the application entry point (multi-mode shell by default)."""

    parser = argparse.ArgumentParser(prog="start-embeddebug")
    parser.add_argument(
        "--smoke",
        action="store_true",
        help="Create the window and process events without entering the event loop.",
    )
    parser.add_argument(
        "--single",
        action="store_true",
        help="Use the legacy single-window serial station instead of the multi-mode shell.",
    )
    args = parser.parse_args(list(argv) if argv is not None else None)

    if args.smoke:
        os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

    app = create_application(["start-embeddebug"] if args.smoke else None)
    # 默认多模式 shell；--single 回退单窗口（兼容 smoke 与旧路径）。
    window = build_main_window() if args.single else build_app_shell()
    window.show()
    app.processEvents()

    if args.smoke:
        window.close()
        app.processEvents()
        window.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        app.processEvents()
        return 0

    return int(app.exec())


if __name__ == "__main__":
    raise SystemExit(main())
