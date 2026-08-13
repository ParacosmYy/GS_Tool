"""Application-owned summary for the future embedded extension station."""

from __future__ import annotations

from collections.abc import Iterable
from dataclasses import dataclass

from .extension_capabilities import (
    ExtensionCapability,
    ExtensionCapabilityGroup,
    ExtensionCapabilityState,
    extension_capabilities,
)

MAX_STATION_ACTION_CHARS = 64
MAX_STATION_PREREQUISITE_CHARS = 512
MAX_STATION_GROUPS = 8
MAX_STATION_GROUP_KEY_CHARS = 64
MAX_STATION_GROUP_LABEL_CHARS = 64


@dataclass(frozen=True, slots=True)
class ExtensionStationGroupSummary:
    """Immutable count projection for one future station capability group."""

    key: str
    label: str
    capability_count: int
    active_backend_count: int = 0

    def __post_init__(self) -> None:
        for field_name, value, limit in (
            ("key", self.key, MAX_STATION_GROUP_KEY_CHARS),
            ("label", self.label, MAX_STATION_GROUP_LABEL_CHARS),
        ):
            if not isinstance(value, str) or not value.strip() or len(value) > limit:
                raise ValueError(
                    f"extension station group {field_name} is outside the bounded range"
                )
        for field_name, value in (
            ("capability_count", self.capability_count),
            ("active_backend_count", self.active_backend_count),
        ):
            if not isinstance(value, int) or isinstance(value, bool) or value < 0:
                raise ValueError(
                    f"extension station group {field_name} must be a non-negative integer"
                )
        if self.active_backend_count > self.capability_count:
            raise ValueError("extension station group active count cannot exceed capability count")


@dataclass(frozen=True, slots=True)
class ExtensionStationSummary:
    """Immutable, presentation-neutral station readiness projection."""

    capability_count: int
    active_backend_count: int
    current_action: str = "无"
    prerequisite: str = (
        "接入前需补齐：目标型号、bootloader、公开一手资料、授权、失败恢复与硬件验收证据。"
    )
    read_only: bool = True
    group_summaries: tuple[ExtensionStationGroupSummary, ...] = ()

    def __post_init__(self) -> None:
        for field_name, value in (
            ("capability_count", self.capability_count),
            ("active_backend_count", self.active_backend_count),
        ):
            if not isinstance(value, int) or isinstance(value, bool) or value < 0:
                raise ValueError(f"extension station {field_name} must be a non-negative integer")
        if self.active_backend_count > self.capability_count:
            raise ValueError("active backend count cannot exceed capability count")
        if len(self.group_summaries) > MAX_STATION_GROUPS:
            raise ValueError("extension station group summary count exceeds the bounded range")
        if any(
            not isinstance(group, ExtensionStationGroupSummary)
            for group in self.group_summaries
        ):
            raise ValueError("extension station group summary is invalid")
        keys = tuple(group.key for group in self.group_summaries)
        if len(set(keys)) != len(keys):
            raise ValueError("extension station group summary keys must be unique")
        if self.group_summaries and sum(
            group.capability_count for group in self.group_summaries
        ) != self.capability_count:
            raise ValueError("extension station group counts must match the capability count")
        if self.group_summaries and sum(
            group.active_backend_count for group in self.group_summaries
        ) != self.active_backend_count:
            raise ValueError("extension station group active counts must match the active count")
        if (
            not isinstance(self.current_action, str)
            or not self.current_action.strip()
            or len(self.current_action) > MAX_STATION_ACTION_CHARS
        ):
            raise ValueError("extension station current action is outside the bounded range")
        if (
            not isinstance(self.prerequisite, str)
            or not self.prerequisite.strip()
            or len(self.prerequisite) > MAX_STATION_PREREQUISITE_CHARS
        ):
            raise ValueError("extension station prerequisite is outside the bounded range")
        if self.read_only is not True:
            raise ValueError("extension station summary must remain read-only")


def extension_station_summary(
    capabilities: Iterable[ExtensionCapability] | None = None,
) -> ExtensionStationSummary:
    """Derive one bounded summary from the application capability catalog."""

    items = tuple(extension_capabilities() if capabilities is None else capabilities)
    active_count = sum(
        capability.state not in {
            ExtensionCapabilityState.CONTRACT_ONLY,
            ExtensionCapabilityState.ATTACH_ONLY,
        }
        for capability in items
    )
    group_labels = {
        ExtensionCapabilityGroup.OTA_TRANSFER: "OTA 传输",
        ExtensionCapabilityGroup.OTA_SECURITY: "OTA 安全",
        ExtensionCapabilityGroup.DEBUG_OUTPUT: "调试输出",
    }
    group_summaries = tuple(
        ExtensionStationGroupSummary(
            key=group.value,
            label=group_labels[group],
            capability_count=sum(capability.group is group for capability in items),
            active_backend_count=sum(
                capability.group is group
                and capability.state
                not in {
                    ExtensionCapabilityState.CONTRACT_ONLY,
                    ExtensionCapabilityState.ATTACH_ONLY,
                }
                for capability in items
            ),
        )
        for group in group_labels
    )
    return ExtensionStationSummary(
        capability_count=len(items),
        active_backend_count=active_count,
        group_summaries=group_summaries,
    )


__all__ = [
    "ExtensionStationGroupSummary",
    "ExtensionStationSummary",
    "extension_station_summary",
]
