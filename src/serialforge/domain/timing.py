"""Transport-neutral timing facts and Modbus RTU timing derivation.

The application may observe when two read chunks were delivered, but that is
not the same thing as measuring the interval between bytes on the UART wire.
Keeping the quality on the value object prevents a host scheduling gap from
being silently presented as a standards-level RTU timing measurement.
"""

from __future__ import annotations

import math
from dataclasses import dataclass
from enum import StrEnum

from .errors import ConfigurationError
from .models import UartParity, UartStopBits, UartTransportConfig


class TimingQuality(StrEnum):
    """Provenance of an observed inter-unit gap."""

    NONE = "none"
    HOST_READ_GAP = "host_read_gap"
    DEVICE_TIMESTAMP = "device_timestamp"


@dataclass(frozen=True, slots=True)
class GapObservation:
    """One optional gap before an ingress unit.

    ``HOST_READ_GAP`` is useful for diagnostics and deterministic replay of
    read scheduling, but it must not be confused with a per-byte UART capture.
    ``DEVICE_TIMESTAMP`` is reserved for a future adapter that can provide a
    trustworthy wire-level timestamp source.
    """

    gap_before: float | None = None
    quality: TimingQuality = TimingQuality.NONE

    def __post_init__(self) -> None:
        if not isinstance(self.quality, TimingQuality):
            raise ConfigurationError("gap quality 必须使用 TimingQuality。")
        if self.gap_before is None:
            if self.quality is not TimingQuality.NONE:
                raise ConfigurationError("有 timing quality 时必须提供 gap_before。")
            return
        if (
            isinstance(self.gap_before, bool)
            or not isinstance(self.gap_before, (int, float))
            or not math.isfinite(self.gap_before)
            or self.gap_before < 0
        ):
            raise ConfigurationError("gap_before 必须是有限非负数字。")
        if self.quality is TimingQuality.NONE:
            raise ConfigurationError("提供 gap_before 时必须声明 timing quality。")


@dataclass(frozen=True, slots=True)
class ModbusRtuTiming:
    """Derive Modbus RTU t1.5/t3.5 thresholds from active UART framing.

    Modbus Serial Line V1.02 specifies character-time thresholds up to
    19,200 bps and fixed 750 us / 1.750 ms values above that rate.  The
    decoder still needs a gap observation carrying an explicit quality.
    """

    baud_rate: int = 115_200
    data_bits: int = 8
    parity: UartParity = UartParity.NONE
    stop_bits: UartStopBits = UartStopBits.ONE

    def __post_init__(self) -> None:
        if isinstance(self.baud_rate, bool) or not isinstance(self.baud_rate, int):
            raise ConfigurationError("Modbus RTU baud_rate 必须是整数。")
        if not 0 < self.baud_rate <= 4_000_000:
            raise ConfigurationError("Modbus RTU baud_rate 必须在 1 到 4000000 内。")
        if isinstance(self.data_bits, bool) or self.data_bits not in {5, 6, 7, 8}:
            raise ConfigurationError("Modbus RTU data_bits 必须是 5、6、7 或 8。")
        if not isinstance(self.parity, UartParity):
            raise ConfigurationError("Modbus RTU parity 必须使用 UartParity。")
        if not isinstance(self.stop_bits, UartStopBits):
            raise ConfigurationError("Modbus RTU stop_bits 必须使用 UartStopBits。")

    @classmethod
    def from_uart_config(cls, config: UartTransportConfig) -> ModbusRtuTiming:
        """Copy only UART wire-format fields; never copy the port identity."""

        if not isinstance(config, UartTransportConfig):
            raise ConfigurationError("Modbus RTU timing 需要 UartTransportConfig。")
        return cls(
            baud_rate=config.baud_rate,
            data_bits=config.data_bits,
            parity=config.parity,
            stop_bits=config.stop_bits,
        )

    @property
    def character_bits(self) -> float:
        """Return start + data + parity + stop bits per UART character."""

        parity_bits = 0 if self.parity is UartParity.NONE else 1
        stop_bits = {
            UartStopBits.ONE: 1.0,
            UartStopBits.ONE_POINT_FIVE: 1.5,
            UartStopBits.TWO: 2.0,
        }[self.stop_bits]
        return 1.0 + self.data_bits + parity_bits + stop_bits

    @property
    def t1_5_seconds(self) -> float:
        """Return the maximum in-frame inter-character interval."""

        if self.baud_rate > 19_200:
            return 0.000750
        return self.character_bits * 1.5 / self.baud_rate

    @property
    def t3_5_seconds(self) -> float:
        """Return the minimum silent interval that terminates an RTU ADU."""

        if self.baud_rate > 19_200:
            return 0.001750
        return self.character_bits * 3.5 / self.baud_rate


__all__ = ["GapObservation", "ModbusRtuTiming", "TimingQuality"]
