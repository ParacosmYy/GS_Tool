"""OTA 传输引擎 — 协议无关的块发送/ACK/超时/重传编排。

给定 ``OtaProtocol``（提供握手/下一块/响应处理/EOT）+ ``OtaByteChannel``（读写），
驱动完整传输循环：握手 → 逐块发送等响应 → EOT 收尾 → 返回 ``TransferResult``。

进度通过可选回调上报（blocks_done, total），供 UI 更新进度条。
约束：本模块只依赖协议接口与标准库，不 import ui。
"""

from __future__ import annotations

from collections.abc import Callable

from embeddebug.ota.protocols.base import (
    CAN, OtaByteChannel, OtaProtocol, TransferResult,
)

_ACK_TIMEOUT_MS = 1000
_MAX_RETRIES = 10
ProgressCallback = Callable[[int, int], None]  # (blocks_done, total)


class TransferEngine:
    """驱动一次 OTA 传输（协议无关）。"""

    def __init__(
        self,
        protocol: OtaProtocol,
        channel: OtaByteChannel,
        *,
        ack_timeout_ms: int = _ACK_TIMEOUT_MS,
        max_retries: int = _MAX_RETRIES,
        on_progress: ProgressCallback | None = None,
    ) -> None:
        self._protocol = protocol
        self._channel = channel
        self._ack_timeout_ms = ack_timeout_ms
        self._max_retries = max_retries
        self._on_progress = on_progress
        self._cancelled = False

    def cancel(self) -> None:
        """请求取消（下个循环检查点中止，发 CAN×2）。"""

        self._cancelled = True

    def run(self) -> TransferResult:
        """执行完整传输，返回结果。"""

        blocks_acked = 0
        # 1. 握手。
        if not self._protocol.start(self._channel):
            return TransferResult(
                success=False, blocks_sent=self._protocol.blocks_sent,
                blocks_acked=0, retries=self._protocol.retries,
                error="handshake_failed",
            )
        # 2. 逐块发送。
        total = _protocol_total(self._protocol)
        consecutive_failures = 0
        while not self._cancelled:
            block = self._protocol.next_block()
            if block is None:
                break  # 数据发完，进入 EOT
            if not self._send_block_await_ack(block):
                consecutive_failures += 1
                if consecutive_failures > self._max_retries:
                    self._send_cancel()
                    return TransferResult(
                        success=False, blocks_sent=self._protocol.blocks_sent,
                        blocks_acked=blocks_acked, retries=self._protocol.retries,
                        error="max_retries_exceeded",
                    )
                continue
            consecutive_failures = 0
            blocks_acked += 1
            if self._on_progress is not None:
                self._on_progress(blocks_acked, total)
        if self._cancelled:
            self._send_cancel()
            return TransferResult(
                success=False, blocks_sent=self._protocol.blocks_sent,
                blocks_acked=blocks_acked, retries=self._protocol.retries,
                error="cancelled",
            )
        # 3. EOT 收尾。
        if not self._protocol.finish(self._channel):
            return TransferResult(
                success=False, blocks_sent=self._protocol.blocks_sent,
                blocks_acked=blocks_acked, retries=self._protocol.retries,
                error="eot_not_acked",
            )
        return TransferResult(
            success=True, blocks_sent=self._protocol.blocks_sent,
            blocks_acked=blocks_acked, retries=self._protocol.retries,
        )

    def _send_block_await_ack(self, block) -> bool:
        """发送一帧并等待响应；返回是否被协议确认。"""

        frame = bytes([block.header, block.sequence, (~block.sequence) & 0xFF]) + block.data + block.checksum
        self._channel.write(frame)
        response = self._channel.read(self._ack_timeout_ms)
        return self._protocol.handle_response(response)

    def _send_cancel(self) -> None:
        """发送 CAN×2 中止传输。"""

        self._channel.write(bytes([CAN, CAN]))


def _protocol_total(protocol: OtaProtocol) -> int:
    """尽力取协议的总块数（供进度分母）。"""

    for attr in ("total_blocks", "total_data_blocks"):
        value = getattr(protocol, attr, None)
        if isinstance(value, int):
            return value
    return 0
