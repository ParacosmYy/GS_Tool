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
    QToolButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug import __version__
from embeddebug.app.app_controller import AppController
from embeddebug.serial_station.ui.theme.theme_switcher import (
    AVAILABLE_THEMES,
    THEME_DARK,
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
        # Batch 10: 强调色选择色点（7 套 accent 变体）+ 当前选中态记录。
        self._accent_swatches: list[QToolButton] = []
        # Batch 23: 「高级」Tab 控件（字体/动画/波特率/重置）—— 构建与同步由
        # ``_settings_advanced_tab`` 助手模块负责（守 300 行门禁）。
        self._font_spin = None  # type: ignore[assignment]
        self._animation_check = None  # type: ignore[assignment]
        self._baudrate_combo = None  # type: ignore[assignment]
        self._reset_button = None  # type: ignore[assignment]

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
        tabs.addTab(self._build_advanced_tab(tabs), widget.tr("高级"))
        tabs.addTab(self._build_about_tab(tabs), widget.tr("关于"))
        layout.addWidget(tabs)
        # Batch 23: 把 panel 反向引用挂到 widget 上，供 _settings_advanced_tab
        # 的「恢复默认」按钮回调找到 panel 实例刷新控件显示。
        widget._settings_panel_ref = self  # type: ignore[attr-defined]
        return widget

    def on_enter(self) -> None:
        """切入设置页：播放入场动画 + 同步当前主题到 combo。

        Batch 19: 同步时用 blockSignals 暂停实时预览，避免 setCurrentText 触发
        _preview_theme 在入场动画/构建竞态下应用主题（曾导致 Qt 销毁崩溃）。
        """

        from embeddebug.serial_station.ui.panels._enter_anim import play_panel_enter

        play_panel_enter(self)
        if self._theme_combo is None:
            return
        from embeddebug.serial_station.ui.theme.manager import ThemeManager
        current = ThemeManager().current_theme or THEME_DARK
        if current in AVAILABLE_THEMES:
            self._theme_combo.blockSignals(True)
            self._theme_combo.setCurrentText(current)
            self._theme_combo.blockSignals(False)
        # Batch 23: 同步「高级」Tab 控件（外部修改 SettingsManager 后保持显示一致）。
        from embeddebug.serial_station.ui.panels._settings_advanced_tab import sync_advanced_tab

        sync_advanced_tab(self)

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
        # Batch 15: Apply 按钮接入 scale 弹性反馈（按下陷下、松手回弹），让
        # Batch 14 的动画缩放在设置页对用户可见。
        from embeddebug.serial_station.ui.micro_interactions import install_scale_press

        install_scale_press(apply_btn)
        # Batch 19: 实时预览 —— 切换 combo 选择即应用主题（无需点「应用」）。
        # 预览不弹 toast（避免噪音），「应用」按钮保留为带 toast 的确认。
        self._theme_combo.currentIndexChanged.connect(self._preview_theme)
        self._theme_status = QLabel(tab)
        self._theme_status.setObjectName("serialStationSettingsThemeStatusLabel")
        row.addWidget(label)
        row.addWidget(self._theme_combo, 1)
        row.addWidget(apply_btn)
        row.addWidget(self._theme_status, 1)
        layout.addLayout(row)

        # Batch 10: 强调色选择行（7 套 accent 变体，点击即时切换 + 过渡动画）。
        layout.addLayout(self._build_accent_row(tab))

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

    def _build_accent_row(self, parent: QWidget) -> QHBoxLayout:
        """Batch 10: 强调色选择行。委托给 ``_accent_row`` 模块（守 300 行门禁）。"""

        from embeddebug.serial_station.ui.panels._accent_row import build_accent_row

        return build_accent_row(self, parent)

    def _build_advanced_tab(self, parent: QWidget) -> QWidget:
        """Batch 23: 「高级」Tab。委托给 ``_settings_advanced_tab``（守门禁）。"""

        from embeddebug.serial_station.ui.panels._settings_advanced_tab import build_advanced_tab

        return build_advanced_tab(self, parent)

    # ── 交互 ────────────────────────────────────────────────────────
    def _preview_theme(self, _index: int) -> None:
        """实时预览：切换 combo 选择即应用主题（Batch 19，不弹 toast）。

        与 ``_apply_theme`` 区别：预览只应用 QSS + 更新状态标签，不弹 success toast
        （避免 combo 每次切换都弹通知）。用户点「应用」按钮才确认（带 toast）。
        """

        from PyQt6.QtWidgets import QApplication

        if self._theme_combo is None:
            return
        name = self._theme_combo.currentData() or THEME_DARK
        try:
            apply_theme_by_name(QApplication.instance(), name)
        except Exception:
            return  # 预览失败静默（_apply_theme 才报错）
        if self._theme_status is not None:
            text = self._widget.tr("预览：深色") if name == THEME_DARK else self._widget.tr("预览：浅色")
            self._theme_status.setText(text)

    def _apply_theme(self) -> None:
        from embeddebug.serial_station.ui.panels._notify import panel_notify
        from PyQt6.QtWidgets import QApplication

        if self._theme_combo is None:
            return
        name = self._theme_combo.currentData() or THEME_DARK
        try:
            # Batch 10: 主题切换带 windowOpacity 暗淡+回亮过渡（掩盖 QSS 硬切闪烁）。
            from embeddebug.serial_station.ui.theme.theme_transition import transition_theme

            def apply_fn() -> None:
                apply_theme_by_name(QApplication.instance(), name)

            transition_theme(QApplication.instance(), apply_fn)
        except Exception as exc:
            if self._theme_status is not None:
                self._theme_status.setText(self._widget.tr("主题应用失败"))
            # Batch 16: 主题应用异常 → error toast。
            panel_notify(self._widget, "error", self._widget.tr("主题切换失败"),
                         self._widget.tr("{err}").format(err=exc))
            return
        # Batch 23: 把选择持久化到 SettingsManager（双写 theme_store 镜像）。
        try:
            from embeddebug.serial_station.services.settings_service import SettingsManager

            SettingsManager.instance().update(theme=name)
        except Exception:
            pass
        if self._theme_status is not None:
            text = self._widget.tr("已应用：深色") if name == THEME_DARK else self._widget.tr("已应用：浅色")
            self._theme_status.setText(text)
        # Batch 16: 主题切换成功 → success toast（用户可能切到别的页，需可见反馈）。
        theme_label = self._widget.tr("深色") if name == THEME_DARK else self._widget.tr("浅色")
        panel_notify(self._widget, "success", self._widget.tr("主题已切换"),
                     self._widget.tr("已应用 {theme} 主题").format(theme=theme_label))

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
