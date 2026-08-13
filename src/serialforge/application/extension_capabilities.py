"""Immutable capability projections for the future embedded tool station.

The catalog is deliberately descriptive.  It does not instantiate an OTA
adapter, open a debug endpoint, load a cryptographic backend, or inspect the
local machine.  Presentation can render this DTO without importing any
vendor, transport, or security implementation.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import StrEnum

from ..debug.contracts import DebugLogBackend
from ..ota.contracts import OtaTransferProtocol
from ..ota.security.contracts import OtaSecuritySuite

MAX_CAPABILITY_KEY_CHARS = 96
MAX_CAPABILITY_TITLE_CHARS = 96
MAX_CAPABILITY_TEXT_CHARS = 320


class ExtensionCapabilityGroup(StrEnum):
    """Bounded sections shown by the embedded tool station."""

    OTA_TRANSFER = "ota_transfer"
    OTA_SECURITY = "ota_security"
    DEBUG_OUTPUT = "debug_output"


class ExtensionCapabilityState(StrEnum):
    """Honest availability states for contract-only extension slots."""

    CONTRACT_ONLY = "contract_only"
    ATTACH_ONLY = "attach_only"


type CapabilityReference = OtaTransferProtocol | OtaSecuritySuite | DebugLogBackend


@dataclass(frozen=True, slots=True)
class ExtensionCapability:
    """One UI-neutral capability card with an explicit ownership boundary."""

    key: str
    title: str
    group: ExtensionCapabilityGroup
    reference: CapabilityReference
    state: ExtensionCapabilityState
    summary: str
    boundary: str

    def __post_init__(self) -> None:
        for field_name, value, limit in (
            ("key", self.key, MAX_CAPABILITY_KEY_CHARS),
            ("title", self.title, MAX_CAPABILITY_TITLE_CHARS),
            ("summary", self.summary, MAX_CAPABILITY_TEXT_CHARS),
            ("boundary", self.boundary, MAX_CAPABILITY_TEXT_CHARS),
        ):
            if not isinstance(value, str) or not value.strip() or len(value) > limit:
                raise ValueError(f"extension capability {field_name} is outside the bounded range")
        if not isinstance(self.group, ExtensionCapabilityGroup):
            raise ValueError("extension capability group is invalid")
        if not isinstance(self.state, ExtensionCapabilityState):
            raise ValueError("extension capability state is invalid")
        expected_reference = {
            ExtensionCapabilityGroup.OTA_TRANSFER: OtaTransferProtocol,
            ExtensionCapabilityGroup.OTA_SECURITY: OtaSecuritySuite,
            ExtensionCapabilityGroup.DEBUG_OUTPUT: DebugLogBackend,
        }[self.group]
        if not isinstance(self.reference, expected_reference):
            raise ValueError("extension capability reference does not match its group")
        expected_state = (
            ExtensionCapabilityState.ATTACH_ONLY
            if self.group is ExtensionCapabilityGroup.DEBUG_OUTPUT
            else ExtensionCapabilityState.CONTRACT_ONLY
        )
        if self.state is not expected_state:
            raise ValueError("extension capability state does not match its group")


_CAPABILITY_CATALOG: tuple[ExtensionCapability, ...] = (
    ExtensionCapability(
        key="ota.transport.xmodem",
        title="XMODEM",
        group=ExtensionCapabilityGroup.OTA_TRANSFER,
        reference=OtaTransferProtocol.XMODEM,
        state=ExtensionCapabilityState.CONTRACT_ONLY,
        summary="经典块传输适配槽位，保留有界进度与取消契约。",
        boundary="未绑定目标 bootloader、块大小、超时、重试和恢复策略。",
    ),
    ExtensionCapability(
        key="ota.transport.ymodem",
        title="YMODEM",
        group=ExtensionCapabilityGroup.OTA_TRANSFER,
        reference=OtaTransferProtocol.YMODEM,
        state=ExtensionCapabilityState.CONTRACT_ONLY,
        summary="带文件元数据的块传输适配槽位。",
        boundary="未绑定目标包格式、目标身份、失败恢复或硬件验收证据。",
    ),
    ExtensionCapability(
        key="ota.transport.tftp",
        title="TFTP",
        group=ExtensionCapabilityGroup.OTA_TRANSFER,
        reference=OtaTransferProtocol.TFTP,
        state=ExtensionCapabilityState.CONTRACT_ONLY,
        summary="无连接文件传输适配槽位，保留独立协议 owner。",
        boundary="不表示网络升级可用；目标授权、可靠性和 bootloader 语义尚未绑定。",
    ),
    ExtensionCapability(
        key="ota.security.aes-256-gcm",
        title="AES-256-GCM",
        group=ExtensionCapabilityGroup.OTA_SECURITY,
        reference=OtaSecuritySuite.AES_256_GCM,
        state=ExtensionCapabilityState.CONTRACT_ONLY,
        summary="带认证加密的安全策略候选。",
        boundary="只保留 key reference；签名、摘要、nonce、anti-rollback 和激活仍须 fail-closed。",
    ),
    ExtensionCapability(
        key="ota.security.aes-128-ccm",
        title="AES-128-CCM",
        group=ExtensionCapabilityGroup.OTA_SECURITY,
        reference=OtaSecuritySuite.AES_128_CCM,
        state=ExtensionCapabilityState.CONTRACT_ONLY,
        summary="面向受限设备的认证加密策略候选。",
        boundary="没有目标密钥托管、版本策略、签名链和掉电恢复证据前不得激活。",
    ),
    ExtensionCapability(
        key="debug.output.rtt",
        title="RTT 原始打印",
        group=ExtensionCapabilityGroup.DEBUG_OUTPUT,
        reference=DebugLogBackend.RTT,
        state=ExtensionCapabilityState.ATTACH_ONLY,
        summary="保留有界 channel/payload/time 的原始日志入口。",
        boundary="仅 attach-only；不提供 memory、halt、run、reset、flash 或任意 probe 命令。",
    ),
    ExtensionCapability(
        key="debug.output.jlink-telnet",
        title="J-Link Telnet",
        group=ExtensionCapabilityGroup.DEBUG_OUTPUT,
        reference=DebugLogBackend.JLINK_TELNET,
        state=ExtensionCapabilityState.ATTACH_ONLY,
        summary="连接已有 J-Link RTT Telnet 服务的原始输出桥接。",
        boundary="不启动 vendor 工具、不加载 SDK/DLL；实际连接仍受 attach-only 授权边界约束。",
    ),
)


def extension_capabilities() -> tuple[ExtensionCapability, ...]:
    """Return the immutable, bounded catalog for a presentation consumer."""

    return _CAPABILITY_CATALOG


__all__ = [
    "ExtensionCapability",
    "ExtensionCapabilityGroup",
    "ExtensionCapabilityState",
    "extension_capabilities",
]
