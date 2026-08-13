"""Presentation-only summary of the currently selected UART wire format."""

from __future__ import annotations

from ..domain.models import UartFlowControl, UartParity, UartStopBits
from .qt import QLabel, QSizePolicy, QWidget

_PARITY_CODES = {
    UartParity.NONE: "N",
    UartParity.ODD: "O",
    UartParity.EVEN: "E",
    UartParity.MARK: "M",
    UartParity.SPACE: "S",
}
_PARITY_LABELS = {
    UartParity.NONE: "无校验",
    UartParity.ODD: "奇校验",
    UartParity.EVEN: "偶校验",
    UartParity.MARK: "Mark 校验",
    UartParity.SPACE: "Space 校验",
}
_STOP_LABELS = {
    UartStopBits.ONE: "1",
    UartStopBits.ONE_POINT_FIVE: "1.5",
    UartStopBits.TWO: "2",
}
_FLOW_LABELS = {
    UartFlowControl.NONE: "无流控",
    UartFlowControl.XON_XOFF: "XON/XOFF",
    UartFlowControl.RTS_CTS: "RTS/CTS",
    UartFlowControl.DSR_DTR: "DSR/DTR",
}


class UartTimingSummarySurface(QLabel):
    """Render a compact, read-only confirmation of the visible UART choices."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("uartTimingSummary")
        self.setProperty("role", "status")
        self.setWordWrap(True)
        # The summary is a compact confirmation rail, not a content viewport.
        # A vertical Preferred policy lets the parent connection grid absorb
        # spare height in focus mode and turns one line into a large empty card.
        self.setSizePolicy(QSizePolicy.Policy.Ignored, QSizePolicy.Policy.Fixed)
        self.setAccessibleName("当前 UART 参数")

    def set_timing(
        self,
        *,
        baud_rate: int,
        data_bits: int,
        parity: UartParity,
        stop_bits: UartStopBits,
        flow_control: UartFlowControl,
    ) -> None:
        """Project existing combo values without owning connection state."""

        parity_code = _PARITY_CODES[parity]
        stop_label = _STOP_LABELS[stop_bits]
        flow_label = _FLOW_LABELS[flow_control]
        summary = (
            f"当前 UART · {baud_rate} baud · "
            f"{data_bits}{parity_code}{stop_label} · {flow_label}"
        )
        detail = (
            f"当前 UART 参数：{baud_rate} baud，{data_bits} 数据位，"
            f"{_PARITY_LABELS[parity]}，{stop_label} 停止位，{flow_label}。"
        )
        self.setText(summary)
        self.setToolTip(detail)
        self.setAccessibleDescription(detail)

    def set_invalid(self) -> None:
        """Keep a deterministic static fallback if a combo is temporarily incomplete."""

        text = "当前 UART · 参数待选择"
        self.setText(text)
        self.setToolTip(text)
        self.setAccessibleDescription(text)


__all__ = ["UartTimingSummarySurface"]
