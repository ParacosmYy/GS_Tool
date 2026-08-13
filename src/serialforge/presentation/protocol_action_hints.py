"""Presentation-owned copy contract for protocol-derived actions."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class ProtocolDerivedActionHints:
    """Stable action copy shared by the builder and its enabled projection."""

    load_profile: str = (
        "打开组件 Profile 或 Codec 选择器；只影响派生解析配置，不修改原始终端或记录。"
    )
    export_component: str = "将当前组件帧表导出为 CSV；不会修改原始记录或发送数据。"
    load_dataset: str = (
        "打开 Dataset 配置选择器；只更新派生 Dataset 配置，不自动发送或连接。"
    )
    export_dataset: str = "将当前 Dataset 样本导出为 CSV；不会修改原始记录或设备状态。"


PROTOCOL_DERIVED_ACTION_HINTS = ProtocolDerivedActionHints()

__all__ = ["PROTOCOL_DERIVED_ACTION_HINTS", "ProtocolDerivedActionHints"]
