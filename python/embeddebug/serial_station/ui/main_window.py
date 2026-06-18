"""Top-level Serial Station window for the Python/PyQt migration lane."""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QMainWindow, QSplitter

from embeddebug.serial_station.controllers import (
    SerialWorkbenchController,
    SerialWorkbenchLogEntry,
)
from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui import (
    command_actions,
    connection_actions,
    endpoint_connection_actions,
    injection_actions,
    lifecycle_actions,
    log_actions,
    measurement_actions,
    protocol_actions,
    session_actions,
    shortcuts,
    status_actions,
)
from embeddebug.serial_station.ui import button_icons
from embeddebug.serial_station.ui.command_palette import CommandItem, CommandPalette
from embeddebug.serial_station.ui.responsive_layout import ResponsiveLayout
from embeddebug.serial_station.ui.sections import build_main_layout


class SerialStationMainWindow(QMainWindow):
    """PyQt Serial Station MVP window."""

    def __init__(self, app_controller: object | None = None) -> None:
        super().__init__()
        # 支持注入共享 AppController（多模式 shell 复用串口连接）；
        # 不传时自建 controller，保持单窗口向后兼容。
        if app_controller is not None:
            self._controller = app_controller.serial_controller
        else:
            self._controller = SerialWorkbenchController()
        self._controller.on_log_entry(self._append_log_entry)
        self._controller.on_error(self._show_error)
        self._controller.on_measurement_batch(self._append_measurement_batch)

        self.setObjectName("embeddebugPySerialStationWindow")
        self.setWindowTitle(self.tr("EmbedDebug PyQt"))
        self.resize(1320, 820)

        self.setCentralWidget(build_main_layout(self, self._controller))
        button_icons.apply_button_icons(self)
        # Batch 7-3: 为所有可聚焦输入控件装 focus_ring（激活 micro_interactions 死代码）。
        button_icons.apply_focus_rings(self)
        self._install_command_palette()
        self._install_responsive_layout()
        # Batch 21: 卡片错峰淡入入场（激活 panel_animations.stagger_fade 死代码）。
        self._stagger_enter_cards()

    def _stagger_enter_cards(self) -> None:
        """工作台卡片错峰淡入（Batch 21）。

        查找所有 serialStationCard 子控件，调 panel_animations.stagger_fade
        做纯透明度错峰淡入（不 move 控件，对布局安全）。动画失败不阻塞。
        """

        try:
            from PyQt6.QtWidgets import QFrame
            from embeddebug.serial_station.ui.panel_animations import stagger_fade

            cards = self.findChildren(QFrame, "serialStationCard")
            if cards:
                self._card_enter_anims = stagger_fade(cards, delay_ms=70)
        except Exception:
            # 错峰淡入是锦上添花，失败不阻塞窗口构建。
            self._card_enter_anims = []

    def _install_command_palette(self) -> None:
        """装配命令面板（Ctrl+P）并注册常用命令。"""

        self._command_palette = CommandPalette(self)
        self._command_palette.set_commands(self._build_command_items())

    def _build_command_items(self) -> tuple[CommandItem, ...]:
        """构建命令面板可用命令。"""

        return (
            CommandItem(self.tr("Connect (Fake)"), self._connect_fake, self.tr("Transport")),
            CommandItem(self.tr("Disconnect"), self._disconnect, self.tr("Transport")),
            CommandItem(self.tr("Refresh Ports"), self._refresh_serial_ports, self.tr("Transport")),
            CommandItem(self.tr("Clear Log"), self._clear_log, self.tr("Session")),
            CommandItem(self.tr("Save Log"), self._export_log, self.tr("Session")),
            CommandItem(self.tr("Replay Log"), self._replay_log, self.tr("Session")),
            CommandItem(self.tr("Save Profile"), self._save_profile, self.tr("Profile")),
            CommandItem(self.tr("Load Profile"), self._load_profile, self.tr("Profile")),
        )

    def _install_responsive_layout(self) -> None:
        """装配响应式布局控制器，绑定顶层窗口监听 resize。

        Batch 4 修复：旧版只靠 self.resizeEvent 驱动，但在 AppShell 多模式 shell 下
        本窗口内容被 reparent 到 AppShell.stack，真正 resize 的是 AppShell，
        self.resizeEvent 不触发，响应式从未运行。现在用 ``attach_to_top_level``
        监听 effective 顶层窗口（``self.window()`` 在 reparent 后返回 AppShell），
        无论被 reparent 到哪都生效。
        """

        splitter = self.findChild(QSplitter, "serialStationMainSplitter")
        if splitter is None:
            return
        self._responsive = ResponsiveLayout(splitter, self)
        # 绑定 effective 顶层窗口（reparent 后 self.window() 返回 AppShell）。
        top_level = self.window() or self
        self._responsive.attach_to_top_level(top_level)

    def _open_command_palette(self) -> None:
        """打开命令面板（Ctrl+P）。"""

        palette = getattr(self, "_command_palette", None)
        if palette is None:
            return
        palette.set_commands(self._build_command_items())
        palette.open()

    def resizeEvent(self, event: object) -> None:
        """窗口 resize 时驱动响应式断点。"""

        super().resizeEvent(event)
        responsive = getattr(self, "_responsive", None)
        if responsive is not None:
            responsive.on_window_resized(self.width())

    def keyPressEvent(self, event: object) -> None:
        if shortcuts.handle_key_press(self, event):
            return
        super().keyPressEvent(event)

    def _set_protocol(self, name: str) -> None:
        protocol_actions.select_protocol(self, name)

    def _refresh_serial_ports(self) -> None:
        connection_actions.refresh_serial_ports(self)

    def _connect_fake(self) -> None:
        connection_actions.connect_fake(self)

    def _connect_serial(self) -> None:
        connection_actions.connect_serial(self)

    def _connect_tcp(self) -> None:
        endpoint_connection_actions.connect_tcp(self)

    def _connect_udp(self) -> None:
        endpoint_connection_actions.connect_udp(self)

    def _disconnect(self) -> None:
        connection_actions.disconnect(self)

    def _set_connected_controls(self, connected: bool) -> None:
        connection_actions.set_connected_controls(self, connected)

    def _has_serial_ports(self) -> bool:
        return connection_actions.has_serial_ports(self)

    def _send_text(self) -> None:
        command_actions.send_text(self)

    def _refresh_command_history(self) -> None:
        command_actions.refresh_command_history(self)

    def _select_command_history(self, text: str) -> None:
        command_actions.select_command_history(self, text)

    def _inject_received(self) -> None:
        injection_actions.inject_received(self)

    def _append_log_entry(self, entry: SerialWorkbenchLogEntry) -> None:
        log_actions.append_log_entry(self, entry)

    def _append_log_line(self, entry: SerialWorkbenchLogEntry) -> None:
        log_actions.append_log_line(self, entry)

    def _render_log_entries(self) -> None:
        log_actions.render_log_entries(self)

    def _log_entry_visible(self, entry: SerialWorkbenchLogEntry) -> bool:
        return log_actions.log_entry_visible(self, entry)

    def _update_log_stats(self) -> None:
        log_actions.update_log_stats(self)

    def _append_measurement_batch(self, batch: ChannelBatch) -> None:
        measurement_actions.append_measurement_batch(self, batch)

    def _clear_log(self) -> None:
        session_actions.clear_log(self)

    def _export_log(self) -> None:
        session_actions.export_log(self)

    def _replay_log(self) -> None:
        session_actions.replay_log(self)

    def _save_profile(self) -> None:
        session_actions.save_profile(self)

    def _load_profile(self) -> None:
        session_actions.load_profile(self)

    def _apply_profile_controls(self, profile: dict[str, object]) -> None:
        session_actions.apply_profile_controls(self, profile)

    def _show_error(self, message: str) -> None:
        status_actions.show_error(self, message)

    def _notify(self, level: str, title: str, message: str, timeout_ms: int = 3000) -> None:
        """把通知意图委托给 effective 顶层窗口（AppShell）的 toast 系统（Batch 13）。

        reparent 到 AppShell.stack 后 ``self.window()`` 返回 AppShell；AppShell.notify
        经 NotificationManager → ToastContainer 渲染 toast。无顶层 notify 时静默返回，
        不影响无 AppShell 的单窗口/测试场景。
        """

        top = self.window()
        notify_fn = getattr(top, "notify", None)
        if callable(notify_fn):
            try:
                notify_fn(level, title, message, timeout_ms=timeout_ms)
            except Exception:
                pass  # toast 是锦上添花，失败不阻塞连接工作流。

    def closeEvent(self, event: object) -> None:
        lifecycle_actions.close_window(self)
        super().closeEvent(event)
