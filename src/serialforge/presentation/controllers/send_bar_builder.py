"""Semantic two-row builder for the live send controls."""

from __future__ import annotations

from functools import partial

from ...domain.models import CommandMode
from ..action_surface import ActionRailButton
from ..qt import (
    QCheckBox,
    QComboBox,
    QFrame,
    QHBoxLayout,
    QLabel,
    QMenu,
    QSizePolicy,
    Qt,
    QToolButton,
    QVBoxLayout,
    QWidget,
)
from ..send_context_surface import SendContextSurface
from ..send_input_surface import SendInputSurface
from ..send_state_surface import SendStateSurface
from .commands import send_current_action


def _configure_send_layout_sizing(
    *,
    mode: QComboBox,
    input_surface: SendInputSurface,
    send_button: ActionRailButton,
    state_label: SendStateSurface,
    context_surface: SendContextSurface,
    newline_check: QCheckBox,
    quick_button: QToolButton,
    save_quick_button: ActionRailButton,
) -> None:
    """Keep actions intrinsic while letting editable/context surfaces absorb width."""

    for widget in (mode, send_button, newline_check, quick_button, save_quick_button):
        _set_horizontal_policy(widget, QSizePolicy.Policy.Fixed)
    _set_horizontal_policy(input_surface, QSizePolicy.Policy.Expanding)
    _set_horizontal_policy(state_label, QSizePolicy.Policy.Preferred)
    _set_horizontal_policy(context_surface, QSizePolicy.Policy.Expanding)


def _set_horizontal_policy(widget: QWidget, policy: QSizePolicy.Policy) -> None:
    """Change only Qt geometry policy; do not freeze a pixel width."""

    size_policy = widget.sizePolicy()
    size_policy.setHorizontalPolicy(policy)
    widget.setSizePolicy(size_policy)


def build_send_bar(window) -> QFrame:
    """Build send input/actions with input priority and a separate status rail."""

    band = QFrame(window)
    band.setObjectName("sendControlBand")
    band.setFrameShape(QFrame.Shape.NoFrame)
    band.setProperty("role", "stationBand")
    band.setProperty("source", "live")
    band.setProperty("state", "blocked")
    layout = QVBoxLayout(band)
    layout.setContentsMargins(12, 8, 12, 8)
    layout.setSpacing(8)

    input_row = QHBoxLayout()
    input_row.setContentsMargins(0, 0, 0, 0)
    input_row.setSpacing(10)
    send_title = QLabel("发送控制")
    send_title.setProperty("role", "section")
    send_title.setProperty("scope", "station")
    send_title.setMinimumWidth(132)
    send_title.setMaximumWidth(150)
    send_title.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
    input_row.addWidget(send_title)

    window._send_mode = QComboBox()
    window._send_mode.setAccessibleName("发送格式")
    window._send_mode.setEditable(False)
    window._send_mode.setToolTip("选择发送内容格式：文本或 Hex（十六进制）。")
    window._send_mode.setAccessibleDescription(
        "选择发送内容的解释格式：文本或十六进制；不会绕过连接和发送校验。"
    )
    window._send_mode.addItem("文本", CommandMode.TEXT)
    window._send_mode.addItem("Hex（十六进制）", CommandMode.HEX)
    window._send_mode.currentIndexChanged.connect(window._on_send_input_changed)
    window._send_mode.setMinimumWidth(110)
    window._send_mode.setMaximumWidth(170)
    input_row.addWidget(window._send_mode)

    window._send_input = SendInputSurface()
    window._send_input.setAccessibleName("发送内容")
    window._send_input.setToolTip("输入要发送的文本或 Hex 内容；发送仍受连接状态和格式校验约束。")
    window._send_input.setAccessibleDescription(
        "输入要发送的文本或十六进制内容；不会在输入时自动发送。"
    )
    window._send_input.setPlaceholderText("输入文本，或输入 Hex：AA 55 01")
    window._send_input.textChanged.connect(window._on_send_input_changed)
    send_action = partial(send_current_action, window)
    window._send_input.returnPressed.connect(send_action)
    window._send_input.setMinimumWidth(220)
    input_row.addWidget(window._send_input, 1)

    send_button = ActionRailButton("发送")
    send_button.setAccessibleName("发送当前内容")
    send_button.setToolTip("发送当前内容；必须先建立可发送会话。")
    send_button.setAccessibleDescription("发送当前内容；必须先建立可发送会话。")
    send_button.setObjectName("primaryButton")
    send_button.clicked.connect(send_action)
    input_row.addWidget(send_button)
    window._send_button = send_button
    layout.addLayout(input_row)

    status_row = QHBoxLayout()
    status_row.setContentsMargins(0, 0, 0, 0)
    status_row.setSpacing(10)
    shortcut_hint = QLabel("Ctrl+Enter 发送")
    shortcut_hint.setObjectName("sendShortcutHint")
    shortcut_hint.setProperty("role", "subtle")
    shortcut_hint.setAccessibleName("发送快捷键提示")
    shortcut_hint.setAccessibleDescription("按 Ctrl+Enter 发送当前内容。")
    shortcut_hint.setToolTip("按 Ctrl+Enter 发送当前内容。")
    shortcut_hint.setFocusPolicy(Qt.FocusPolicy.NoFocus)
    shortcut_hint.setAlignment(Qt.AlignmentFlag.AlignCenter)
    shortcut_hint.setMaximumWidth(150)
    shortcut_hint.setSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Preferred)
    status_row.addWidget(shortcut_hint)

    window._send_state_label = SendStateSurface("等待连接")
    window._send_state_label.setObjectName("sendState")
    window._send_state_label.setProperty("role", "status")
    window._send_state_label.setProperty("state", "blocked")
    window._send_state_label.setAccessibleName("发送状态")
    window._send_state_label.setAccessibleDescription("建立可发送会话后才能发送。")
    window._send_state_label.setToolTip("建立可发送会话后才能发送。")
    window._send_state_label.setMinimumWidth(100)
    window._send_state_label.setMaximumWidth(180)
    window._send_state_label.setSizePolicy(QSizePolicy.Policy.Ignored, QSizePolicy.Policy.Preferred)
    status_row.addWidget(window._send_state_label)

    window._send_context_surface = SendContextSurface()
    window._send_context_surface.setObjectName("sendContext")
    window._send_context_surface.setProperty("state", "empty")
    status_row.addWidget(window._send_context_surface, 1)

    window._newline_check = QCheckBox("追加 CRLF")
    window._newline_check.setAccessibleName("发送内容追加 CRLF")
    window._newline_check.setToolTip("发送 payload 后追加 CRLF 换行字节；不会单独触发发送。")
    window._newline_check.setAccessibleDescription(
        "控制发送 payload 后是否追加 CRLF 换行字节；勾选只改变发送内容。"
    )
    window._newline_check.toggled.connect(window._on_send_input_changed)
    status_row.addWidget(window._newline_check)

    window._quick_button = QToolButton()
    window._quick_button.setText("快捷命令")
    window._quick_button.setAccessibleName("快捷命令菜单")
    window._quick_button.setToolTip("打开快捷命令菜单；选择命令只会填入发送区，不会自动发送。")
    window._quick_button.setAccessibleDescription(
        "打开快捷命令菜单；选择快捷命令只填入发送区，不会自动发送。"
    )
    window._quick_menu = QMenu(window)
    window._quick_button.setMenu(window._quick_menu)
    window._quick_button.setPopupMode(QToolButton.ToolButtonPopupMode.InstantPopup)
    window._quick_button.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
    status_row.addWidget(window._quick_button)

    save_quick_button = ActionRailButton("保存快捷")
    save_quick_button.setAccessibleName("保存当前内容为快捷命令")
    save_quick_button.setToolTip("将当前发送内容保存为快捷命令；不会立即发送。")
    save_quick_button.setAccessibleDescription(
        "将当前发送内容保存为快捷命令；保存不会立即向设备发送数据。"
    )
    save_quick_button.clicked.connect(window._save_current_quick)
    save_quick_button.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Preferred)
    status_row.addWidget(save_quick_button)
    window._save_quick_button = save_quick_button
    _configure_send_layout_sizing(
        mode=window._send_mode,
        input_surface=window._send_input,
        send_button=send_button,
        state_label=window._send_state_label,
        context_surface=window._send_context_surface,
        newline_check=window._newline_check,
        quick_button=window._quick_button,
        save_quick_button=save_quick_button,
    )
    layout.addLayout(status_row)
    window._send_control_band = band
    return band


__all__ = ["build_send_bar"]
