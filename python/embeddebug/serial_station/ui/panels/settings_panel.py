"""设置模式面板 — 主题切换 + 快捷键展示 + 关于页。

ModePanel 实现：QTabWidget 三 Tab（主题/快捷键/关于）。主题切换走
``theme_switcher.apply_theme_by_name``（深/浅双主题均已就绪）；快捷键只读
静态表（真相源在 ``shortcuts.py``）；关于页展示版本/应用名/远程。

约束：只读消费 theme_switcher/shortcuts 常量，不碰 controller/transport。
"""

from __future__ import annotations

import subprocess

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QTableWidget,
    QTableWidgetItem,
    QTabWidget,
    QVBoxLayout,
    QWidget,
)

from embeddebug import __version__
from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.theme.theme_switcher import (
    AVAILABLE_THEMES,
    THEME_DARK,
    THEME_LIGHT,
    apply_theme_by_name,
)

# 快捷键静态映射（真相源：shortcuts.py 的 install_shortcuts + app_shell.keyPressEvent）。
_SHORTCUT_ROWS = (
    ("发送输入", "Ctrl+Return"),
    ("清空日志", "Ctrl+L"),
    ("刷新串口列表", "Ctrl+R"),
    ("打开命令面板", "Ctrl+P"),
    # Batch 15: toast 通知快捷键（AppShell 全局处理）。
    ("关闭最早通知", "Esc"),
    ("清空所有通知", "Ctrl+Shift+Esc"),
)
_FALLBACK_REMOTE = "（未读取到 Git 远程）"


class SettingsPanel:
    """设置 ModePanel：主题/快捷键/关于 三 Tab。"""

    def __init__(self) -> None:
        self._app_controller: AppController | None = None
        self._widget: QWidget | None = None
        self._theme_combo: QComboBox | None = None
        self._theme_status: QLabel | None = None

    def build(self, app_controller: AppController) -> QWidget:
        self._app_controller = app_controller
        widget = QWidget()
        widget.setObjectName("serialStationSettingsPanel")
        self._widget = widget  # 先赋值，供 _build_*_tab 内 self._widget.tr(...) 使用。
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        tabs = QTabWidget(widget)
        tabs.setObjectName("serialStationSettingsTabWidget")
        tabs.addTab(self._build_theme_tab(tabs), widget.tr("主题"))
        tabs.addTab(self._build_shortcuts_tab(tabs), widget.tr("快捷键"))
        tabs.addTab(self._build_about_tab(tabs), widget.tr("关于"))
        layout.addWidget(tabs)
        return widget

    def on_enter(self) -> None:
        """切入设置页：播放入场动画 + 同步当前主题到 combo。"""

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)
        if self._theme_combo is None:
            return
        from embeddebug.serial_station.ui.theme.manager import ThemeManager
        current = ThemeManager().current_theme or THEME_DARK
        if current in AVAILABLE_THEMES:
            self._theme_combo.setCurrentText(current)

    def on_leave(self) -> None:
        """切出设置页：停止入场动画。"""

        from embeddebug.serial_station.ui.panels._enter_anim import stop_panel_enter

        stop_panel_enter(self)

    # ── Tab 构建 ────────────────────────────────────────────────────
    def _build_theme_tab(self, parent: QWidget) -> QWidget:
        tab = QWidget(parent)
        layout = QVBoxLayout(tab)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        row = QHBoxLayout()
        label = QLabel(self._widget.tr("主题"), tab)
        label.setObjectName("serialStationSettingsFieldLabel")
        self._theme_combo = QComboBox(tab)
        self._theme_combo.setObjectName("serialStationSettingsThemeCombo")
        for name in AVAILABLE_THEMES:
            display = self._widget.tr("深色") if name == THEME_DARK else self._widget.tr("浅色")
            self._theme_combo.addItem(display, name)
        apply_btn = QPushButton(self._widget.tr("应用"), tab)
        apply_btn.setObjectName("serialStationSettingsApplyButton")
        apply_btn.clicked.connect(self._apply_theme)
        self._theme_status = QLabel(tab)
        self._theme_status.setObjectName("serialStationSettingsThemeStatusLabel")
        row.addWidget(label)
        row.addWidget(self._theme_combo, 1)
        row.addWidget(apply_btn)
        row.addWidget(self._theme_status, 1)
        layout.addLayout(row)

        hint = QLabel(
            self._widget.tr("切换深色/浅色工业风主题。浅色对齐 EK-OmniProbe 默认玻璃观感。"),
            tab,
        )
        hint.setObjectName("serialStationSettingsFieldLabel")
        hint.setWordWrap(True)
        layout.addWidget(hint)
        layout.addStretch(1)
        return tab

    def _build_shortcuts_tab(self, parent: QWidget) -> QWidget:
        tab = QWidget(parent)
        layout = QVBoxLayout(tab)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(8)

        table = QTableWidget(len(_SHORTCUT_ROWS), 2, tab)
        table.setObjectName("serialStationSettingsShortcutsTable")
        table.setHorizontalHeaderLabels([self._widget.tr("动作"), self._widget.tr("组合键")])
        table.verticalHeader().setVisible(False)
        table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        table.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        table.setSelectionMode(QTableWidget.SelectionMode.NoSelection)
        for row, (action, keys) in enumerate(_SHORTCUT_ROWS):
            table.setItem(row, 0, QTableWidgetItem(action))
            table.setItem(row, 1, QTableWidgetItem(keys))
        table.resizeColumnsToContents()
        layout.addWidget(table)
        layout.addStretch(1)
        return tab

    def _build_about_tab(self, parent: QWidget) -> QWidget:
        tab = QWidget(parent)
        layout = QVBoxLayout(tab)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(8)

        name = QLabel("EmbedDebug", tab)
        name.setObjectName("serialStationSettingsAppNameLabel")
        layout.addWidget(name)

        version = QLabel(self._widget.tr("版本 {version}").format(version=__version__), tab)
        version.setObjectName("serialStationSettingsVersionLabel")
        layout.addWidget(version)

        remote = QLabel(self._read_git_remote(), tab)
        remote.setObjectName("serialStationSettingsRemoteLabel")
        remote.setWordWrap(True)
        layout.addWidget(remote)
        layout.addStretch(1)
        return tab

    # ── 交互 ────────────────────────────────────────────────────────
    def _apply_theme(self) -> None:
        from PyQt6.QtWidgets import QApplication
        if self._theme_combo is None:
            return
        name = self._theme_combo.currentData() or THEME_DARK
        apply_theme_by_name(QApplication.instance(), name)
        if self._theme_status is not None:
            text = self._widget.tr("已应用：深色") if name == THEME_DARK else self._widget.tr("已应用：浅色")
            self._theme_status.setText(text)

    def _read_git_remote(self) -> str:
        try:
            result = subprocess.run(
                ["git", "config", "--get", "remote.origin.url"],
                capture_output=True, text=True, timeout=2,
            )
            url = result.stdout.strip()
            return url or _FALLBACK_REMOTE
        except Exception:
            return _FALLBACK_REMOTE
