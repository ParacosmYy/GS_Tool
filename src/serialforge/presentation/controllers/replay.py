"""Historical replay controller.

Replay owns source identity, playback state, pause/stop affordances, and
historical-surface gating. It does not mutate live transport configuration.
"""

from __future__ import annotations

from ...domain.replay import ReplaySnapshot, ReplayState
from ..connection_bindings import connection_shell_bindings_for
from ..file_dialog_surface import get_open_file_name
from ..qt import Qt
from ..replay_activity_surface import ReplayActivityProjection

REPLAY_ACTIVITY_MS = 520
REPLAY_TERMINAL_ACTIVITY_MS = 480


def _request_replay_activity(window, duration_ms: int) -> None:
    """Acknowledge visible replay transitions through the shared motion clock."""

    if window._closing or not window.isVisible() or window.isMinimized():
        return
    motion_controller = getattr(window, "_motion_controller", None)
    if motion_controller is not None:
        motion_controller.request_activity(duration_ms)


def start_replay(window) -> None:
    """Start replay."""
    path, _ = get_open_file_name(
        window,
        "选择历史 JSONL",
        "",
        "SerialForge JSONL (*.jsonl);;All files (*)",
    )
    if path:
        window._view_model.start_replay(
            path,
            float(window._replay_speed.currentData(Qt.ItemDataRole.UserRole)),
        )


def toggle_replay_pause(window) -> None:
    """Toggle replay pause."""
    if window._view_model.replay_snapshot.state is ReplayState.PAUSED:
        window._view_model.resume_replay()
    else:
        window._view_model.pause_replay()


def on_replay_changed(window, snapshot: object) -> None:
    """On replay changed."""
    if window._closing:
        return
    if not isinstance(snapshot, ReplaySnapshot):
        return
    previous_history_source = window._history_source_active
    previous_history_file = window._history_file_selected
    window._history_file_selected = snapshot.state is not ReplayState.EMPTY and bool(snapshot.path)
    window._history_source_active = window._history_file_selected and (
        snapshot.state in {ReplayState.PLAYING, ReplayState.PAUSED} or snapshot.records_emitted > 0
    )
    if (
        previous_history_source != window._history_source_active
        or previous_history_file != window._history_file_selected
    ):
        window._reset_data_activity()
        shell = connection_shell_bindings_for(window)
        window._on_transport_changed(shell.transport_combo.currentIndex() if shell else -1)
    labels = {
        ReplayState.EMPTY: "未加载",
        ReplayState.PLAYING: "播放中",
        ReplayState.PAUSED: "已暂停",
        ReplayState.EOF: "已结束",
        ReplayState.STOPPED: "已停止",
        ReplayState.ERROR: "错误",
    }
    error = f" · {snapshot.error.message}" if snapshot.error else ""
    path = snapshot.path or ""
    filename = path.replace("\\", "/").rsplit("/", 1)[-1] if path else ""
    if snapshot.state is ReplayState.EMPTY:
        text = "未选择历史文件 · 点击“选择并回放”"
    else:
        text = (
            f"历史数据 · {labels[snapshot.state]} · RX {snapshot.records_emitted} 条"
            f" · 无效 {snapshot.invalid_records} · 跳过 {snapshot.skipped_records}"
        )
        if snapshot.state is ReplayState.EOF and snapshot.records_emitted == 0:
            text += " · 没有可回放 RX 记录"
        if filename:
            text += f" · {filename}"
        text += error
    window._replay_status.setText(text)
    window._replay_status.setToolTip(path or text)
    window._replay_status.setAccessibleDescription(f"{text}。完整路径：{path}" if path else text)
    window._replay_pause_button.setText("继续" if snapshot.state is ReplayState.PAUSED else "暂停")
    active = snapshot.state in {ReplayState.PLAYING, ReplayState.PAUSED}
    pause_hint = (
        "继续当前历史回放。"
        if snapshot.state is ReplayState.PAUSED
        else ("暂停当前历史回放。" if active else "当前没有正在播放的历史回放。")
    )
    start_hint = (
        "当前回放进行中；停止后可选择新的历史文件。"
        if active
        else "选择一个 JSONL 原始记录并开始历史回放。"
    )
    stop_hint = "停止当前历史回放。" if active else "当前没有正在播放的历史回放。"
    for widget, hint in (
        (window._replay_pause_button, pause_hint),
        (window._replay_start_button, start_hint),
        (window._replay_stop_button, stop_hint),
    ):
        widget.setAccessibleDescription(hint)
        widget.setToolTip(hint)
    replay_surface_states = {
        ReplayState.EMPTY: "idle",
        ReplayState.PLAYING: "active",
        ReplayState.PAUSED: "paused",
        ReplayState.EOF: "history",
        ReplayState.STOPPED: "history",
        ReplayState.ERROR: "error",
    }
    replay_surface_state = replay_surface_states[snapshot.state]
    window._replay_pause_button.set_busy(snapshot.state is ReplayState.PLAYING)
    window._replay_status.set_projection(
        ReplayActivityProjection(
            state=replay_surface_state,
            records_emitted=snapshot.records_emitted,
        )
    )
    if snapshot.state is ReplayState.PLAYING:
        _request_replay_activity(window, REPLAY_ACTIVITY_MS)
    elif snapshot.state in {ReplayState.EOF, ReplayState.STOPPED, ReplayState.ERROR}:
        _request_replay_activity(window, REPLAY_TERMINAL_ACTIVITY_MS)
    window._status_surfaces.set_state("replay", replay_surface_states[snapshot.state])
    window._replay_pause_button.setEnabled(active)
    window._replay_stop_button.setEnabled(active)
    window._replay_speed.setEnabled(not active)
    window._replay_start_button.setEnabled(not active)
    window._update_source_badge()
    window._update_pipeline_summary()
    window._refresh_connection_controls(window._view_model.state)
