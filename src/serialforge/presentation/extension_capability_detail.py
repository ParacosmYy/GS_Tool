"""Read-only detail surface for one embedded station capability."""

from __future__ import annotations

from ..application.extension_capabilities import (
    ExtensionCapability,
    ExtensionCapabilityGroup,
    ExtensionCapabilityState,
)
from .property_refresh import refresh_dynamic_property
from .qt import QFrame, QHBoxLayout, QLabel, QSizePolicy, QVBoxLayout

_GROUP_LABELS = {
    ExtensionCapabilityGroup.OTA_TRANSFER: "OTA 传输",
    ExtensionCapabilityGroup.OTA_SECURITY: "OTA 安全",
    ExtensionCapabilityGroup.DEBUG_OUTPUT: "调试输出",
}
_STATE_LABELS = {
    ExtensionCapabilityState.CONTRACT_ONLY: "契约预留",
    ExtensionCapabilityState.ATTACH_ONLY: "仅附着",
}


class ExtensionCapabilityDetail(QFrame):
    """Project an immutable capability without creating an activation path."""

    def __init__(
        self,
        capability: ExtensionCapability,
        parent: QFrame | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("extensionCapabilityDetail")
        self.setProperty("role", "surface")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        self.setMinimumHeight(126)

        root = QVBoxLayout(self)
        root.setContentsMargins(12, 10, 12, 10)
        root.setSpacing(5)

        header = QHBoxLayout()
        header.setContentsMargins(0, 0, 0, 0)
        eyebrow = QLabel("能力详情")
        eyebrow.setObjectName("extensionCapabilityDetailEyebrow")
        eyebrow.setProperty("role", "section")
        header.addWidget(eyebrow)
        header.addStretch(1)
        self._state = QLabel()
        self._state.setObjectName("extensionCapabilityDetailState")
        self._state.setProperty("role", "status")
        header.addWidget(self._state)
        root.addLayout(header)

        title_row = QHBoxLayout()
        title_row.setContentsMargins(0, 0, 0, 0)
        self._title = QLabel()
        self._title.setObjectName("extensionCapabilityDetailTitle")
        title_row.addWidget(self._title)
        title_row.addStretch(1)
        self._reference = QLabel()
        self._reference.setObjectName("extensionCapabilityDetailReference")
        title_row.addWidget(self._reference)
        root.addLayout(title_row)

        self._identity = QLabel()
        self._identity.setObjectName("extensionCapabilityDetailIdentity")
        self._identity.setProperty("role", "subtle")
        self._identity.setWordWrap(True)
        root.addWidget(self._identity)

        self._summary = QLabel()
        self._summary.setObjectName("extensionCapabilityDetailSummary")
        self._summary.setProperty("role", "subtle")
        self._summary.setWordWrap(True)
        root.addWidget(self._summary)

        self._boundary = QLabel()
        self._boundary.setObjectName("extensionCapabilityDetailBoundary")
        self._boundary.setProperty("role", "subtle")
        self._boundary.setWordWrap(True)
        root.addWidget(self._boundary)

        guard = QLabel("只读说明 · 不执行升级、解密、签名激活或探针控制。")
        guard.setObjectName("extensionCapabilityDetailGuard")
        guard.setProperty("role", "subtle")
        guard.setWordWrap(True)
        root.addWidget(guard)

        self.set_capability(capability)

    def set_capability(self, capability: ExtensionCapability) -> None:
        """Render a new immutable capability selection."""

        state = capability.state.value
        group = _GROUP_LABELS[capability.group]
        reference = str(capability.reference.value).replace("_", "-").upper()
        self._title.setText(capability.title)
        self._state.setText(_STATE_LABELS[capability.state])
        self._reference.setText(f"{group} · {reference}")
        self._identity.setText(
            f"key={capability.key} · group={capability.group.value} · "
            f"reference={capability.reference.value} · state={state}"
        )
        self._summary.setText(capability.summary)
        self._boundary.setText(f"当前边界：{capability.boundary}")

        refresh_dynamic_property(self, "state", state)
        refresh_dynamic_property(self._state, "state", state)
        self.setAccessibleName(f"{capability.title} 能力详情")
        self.setAccessibleDescription(
            f"key {capability.key}，group {capability.group.value}，"
            f"reference {capability.reference.value}，state {state}。"
            f"{group}，{_STATE_LABELS[capability.state]}。"
            f"{capability.summary}{capability.boundary}"
        )


__all__ = ["ExtensionCapabilityDetail"]
