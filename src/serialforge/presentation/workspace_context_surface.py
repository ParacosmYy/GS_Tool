"""Read-only workspace context label for the shared route strip."""

from __future__ import annotations

from .property_refresh import refresh_dynamic_property
from .qt import QLabel, Qt, QWidget

_CONTEXTS = (
    (
        "connection",
        "当前页 · 链路配置",
        "当前工作区：链路配置。可选择 UART、网络、BLE 或 RTT 连接方式。",
    ),
    (
        "protocol",
        "当前页 · 解析与遥测",
        "当前工作区：解析与遥测。可配置协议帧并查看派生数据。",
    ),
    (
        "commands",
        "当前页 · 命令管理",
        "当前工作区：命令管理。可查看发送历史并维护批量命令。",
    ),
    (
        "extension",
        "当前页 · 能力预览",
        "当前工作区：能力预览。查看 OTA、安全和调试输出扩展边界。",
    ),
)


class WorkspaceContextLabel(QLabel):
    """Project the selected workspace tab into a non-interactive context cue."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("workspaceContextLabel")
        self.setAlignment(Qt.AlignmentFlag.AlignVCenter | Qt.AlignmentFlag.AlignLeft)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
        self.setFixedHeight(22)
        self.setMinimumWidth(158)
        self.setMaximumWidth(218)
        self.setAccessibleName("当前工作区")
        self._index = -1
        self._focus_mode = False
        self.set_index(0)

    def set_index(self, index: int) -> None:
        """Reflect the existing bounded Tab index without owning navigation."""

        try:
            normalized = max(0, min(len(_CONTEXTS) - 1, int(index)))
        except (TypeError, ValueError):
            normalized = 0
        if normalized == self._index:
            return
        self._index = normalized
        self._refresh_context()

    def set_mode(self, focus: bool) -> None:
        """Reflect the shell's existing focus/overview layout mode."""

        normalized = bool(focus)
        if normalized == self._focus_mode:
            return
        self._focus_mode = normalized
        self._refresh_context()

    def _refresh_context(self) -> None:
        """Project tab and layout mode without owning either state source."""

        if not 0 <= self._index < len(_CONTEXTS):
            return
        state, page_text, page_description = _CONTEXTS[self._index]
        mode_text = "专注" if self._focus_mode else "总览"
        mode_description = (
            "当前为专注设置模式；实时观测、终端和发送区暂时收起，仍在后台工作。"
            if self._focus_mode
            else "当前为总览模式；实时观测、终端和发送区与工作区同时显示。"
        )
        refresh_dynamic_property(self, "state", state)
        refresh_dynamic_property(self, "mode", "focus" if self._focus_mode else "overview")
        self.setText(f"{mode_text} · {page_text.split(' · ', 1)[-1]}")
        description = f"{mode_description} {page_description}"
        self.setAccessibleDescription(description)
        self.setToolTip(description)


__all__ = ["WorkspaceContextLabel"]
