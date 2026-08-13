"""Bounded command catalog and enqueue-level batch execution service."""

from __future__ import annotations

import logging
from threading import Event, Lock, Thread
from typing import Final

from ..domain.commands import (
    MAX_COMMAND_BATCHES,
    CommandBatch,
    CommandBatchId,
    CommandBatchRequest,
    CommandBatchSnapshot,
    CommandBatchState,
)
from ..domain.errors import (
    CommandBatchShutdownTimeoutError,
    ConfigurationError,
    ErrorCode,
    ErrorInfo,
    SerialForgeError,
)
from ..domain.models import (
    BleGattTransportConfig,
    BleGattWrite,
    DatagramSend,
    RttDownWrite,
    RttTransportConfig,
    StreamWrite,
    TcpServerSend,
    TcpServerTransportConfig,
    TransportConfig,
    UdpTransportConfig,
)
from ..domain.ports import CommandBatchPort, CommandHistoryPort, SessionPort, TransportWrite

logger = logging.getLogger(__name__)

MAX_COMMAND_BATCH_MESSAGE_LENGTH: Final = 256


def build_transport_write(
    config: TransportConfig,
    payload: bytes,
    *,
    target_peer_id=None,
    ble_characteristic=None,
    ble_write_mode=None,
) -> TransportWrite:
    """Build one typed write without importing any concrete transport adapter."""

    if not payload:
        raise ConfigurationError("发送 payload 不能为空。")
    if isinstance(config, UdpTransportConfig):
        return DatagramSend(config.remote_peer, payload)
    if isinstance(config, TcpServerTransportConfig):
        if target_peer_id is None:
            raise ConfigurationError("发送失败 · 请先选择 TCP Server client。")
        return TcpServerSend(target_peer_id, StreamWrite(payload))
    if isinstance(config, BleGattTransportConfig):
        if ble_characteristic is None or ble_write_mode is None:
            raise ConfigurationError("发送失败 · 请先选择 BLE GATT 可写特征和写模式。")
        return BleGattWrite(
            characteristic=ble_characteristic,
            mode=ble_write_mode,
            payload=payload,
        )
    if isinstance(config, RttTransportConfig):
        return RttDownWrite(channel=config.channel, write=StreamWrite(payload))
    return StreamWrite(payload)


class CommandBatchService(CommandBatchPort):
    """Own finite command definitions and one cancellable enqueue worker."""

    def __init__(self, *, session: SessionPort, history: CommandHistoryPort) -> None:
        self._session = session
        self._history = history
        self._batches: list[CommandBatch] = []
        self._lock = Lock()
        self._cancelled = Event()
        self._thread: Thread | None = None
        self._snapshot = CommandBatchSnapshot()

    def add_batch(self, batch: CommandBatch) -> None:
        if not isinstance(batch, CommandBatch):
            raise ConfigurationError("批量命令目录只接受 CommandBatch。")
        with self._lock:
            self._reject_if_running()
            for index, existing in enumerate(self._batches):
                if existing.batch_id == batch.batch_id:
                    self._batches[index] = batch
                    return
            if len(self._batches) >= MAX_COMMAND_BATCHES:
                raise ConfigurationError(
                    f"批量命令最多保存 {MAX_COMMAND_BATCHES} 个，请先删除旧命令。"
                )
            self._batches.insert(0, batch)

    def batches(self) -> tuple[CommandBatch, ...]:
        with self._lock:
            return tuple(self._batches)

    def get_batch(self, batch_id: CommandBatchId) -> CommandBatch | None:
        with self._lock:
            return next((batch for batch in self._batches if batch.batch_id == batch_id), None)

    def remove_batch(self, batch_id: CommandBatchId) -> None:
        with self._lock:
            self._reject_if_running()
            for index, batch in enumerate(self._batches):
                if batch.batch_id == batch_id:
                    del self._batches[index]
                    return
            raise ConfigurationError("要删除的批量命令不存在。")

    def start_batch(self, request: CommandBatchRequest) -> CommandBatchId:
        """Pre-build every typed write before the first byte can be queued."""

        if isinstance(request.config, RttTransportConfig):
            raise ConfigurationError("J-Link RTT 批量命令留到最后阶段，当前仅支持单条发送。")
        try:
            writes = tuple(
                build_transport_write(
                    request.config,
                    step.wire_payload,
                    target_peer_id=request.target_peer_id,
                    ble_characteristic=request.ble_characteristic,
                    ble_write_mode=request.ble_write_mode,
                )
                for step in request.batch.steps
            )
        except SerialForgeError as exc:
            with self._lock:
                self._snapshot = CommandBatchSnapshot(
                    batch_id=request.batch.batch_id,
                    state=CommandBatchState.FAILED,
                    total_steps=len(request.batch.steps),
                    message="执行前校验失败 · 未发送任何步骤",
                    error=exc.info,
                )
            raise
        with self._lock:
            self._reject_if_running()
            self._cancelled = Event()
            self._snapshot = CommandBatchSnapshot(
                batch_id=request.batch.batch_id,
                state=CommandBatchState.RUNNING,
                total_steps=len(request.batch.steps),
                message=f"执行中 · 0/{len(request.batch.steps)} 步已提交",
            )
            thread = Thread(
                target=self._run,
                args=(request, writes, self._cancelled),
                name="serialforge-command-batch",
                daemon=True,
            )
            self._thread = thread
        try:
            thread.start()
        except RuntimeError:
            with self._lock:
                self._thread = None
                self._snapshot = CommandBatchSnapshot(
                    batch_id=request.batch.batch_id,
                    state=CommandBatchState.FAILED,
                    total_steps=len(request.batch.steps),
                    message="批量命令 worker 启动失败。",
                    error=ErrorInfo(
                        code=ErrorCode.UNKNOWN,
                        message="批量命令 worker 启动失败。",
                        recoverable=False,
                    ),
                )
            raise
        return request.batch.batch_id

    def cancel_batch(self, batch_id: CommandBatchId | None = None) -> None:
        with self._lock:
            if self._snapshot.state is not CommandBatchState.RUNNING:
                return
            if batch_id is not None and batch_id != self._snapshot.batch_id:
                return
            self._cancelled.set()

    def snapshot(self) -> CommandBatchSnapshot:
        with self._lock:
            return self._snapshot

    def shutdown(self, timeout: float | None = 3.0) -> None:
        if timeout is not None and timeout < 0:
            raise ValueError("timeout must be non-negative or None")
        self.cancel_batch()
        with self._lock:
            thread = self._thread
        if thread is None:
            return
        thread.join(timeout=timeout)
        if thread.is_alive():
            raise CommandBatchShutdownTimeoutError()

    def _run(
        self,
        request: CommandBatchRequest,
        writes: tuple[TransportWrite, ...],
        cancelled: Event,
    ) -> None:
        accepted = 0
        total = len(writes)
        try:
            for index, (step, write) in enumerate(zip(request.batch.steps, writes, strict=True)):
                step_number = index + 1
                if cancelled.is_set():
                    self._finish_stopped(request.batch.batch_id, total, accepted)
                    return
                self._update_running(
                    request.batch.batch_id,
                    total,
                    accepted,
                    current_step=step_number,
                    message=f"执行中 · 第 {step_number}/{total} 步",
                )
                try:
                    self._session.send(request.session_id, write)
                except SerialForgeError as exc:
                    self._finish_failed(
                        request.batch.batch_id,
                        total,
                        accepted,
                        step_number,
                        exc.info,
                    )
                    return
                accepted += 1
                try:
                    self._history.add_history(step.entry)
                except Exception:
                    # History is a UI convenience; it must not turn an accepted
                    # transport write into a false batch failure.
                    logger.exception("command batch history update failed")
                self._update_running(
                    request.batch.batch_id,
                    total,
                    accepted,
                    current_step=step_number,
                    message=f"已提交 · {accepted}/{total} 步",
                )
                if index == total - 1 or step.delay_after_ms == 0:
                    continue
                if cancelled.wait(step.delay_after_ms / 1000.0):
                    self._finish_stopped(request.batch.batch_id, total, accepted)
                    return
            self._finish_completed(request.batch.batch_id, total, accepted)
        except Exception as exc:
            logger.exception("command batch worker failed")
            self._finish_failed(
                request.batch.batch_id,
                total,
                accepted,
                None,
                ErrorInfo(
                    code=ErrorCode.UNKNOWN,
                    message="批量命令 worker 发生未处理错误。",
                    recoverable=False,
                    detail=f"{type(exc).__name__}: {exc}",
                ),
            )
        finally:
            with self._lock:
                self._thread = None

    def _reject_if_running(self) -> None:
        if self._snapshot.state is CommandBatchState.RUNNING:
            raise ConfigurationError("已有批量命令正在执行，请先停止。")

    def _update_running(
        self,
        batch_id: CommandBatchId,
        total: int,
        accepted: int,
        *,
        current_step: int | None,
        message: str,
    ) -> None:
        with self._lock:
            self._snapshot = CommandBatchSnapshot(
                batch_id=batch_id,
                state=CommandBatchState.RUNNING,
                total_steps=total,
                accepted_steps=accepted,
                current_step=current_step,
                message=message[:MAX_COMMAND_BATCH_MESSAGE_LENGTH],
            )

    def _finish_completed(self, batch_id: CommandBatchId, total: int, accepted: int) -> None:
        with self._lock:
            self._snapshot = CommandBatchSnapshot(
                batch_id=batch_id,
                state=CommandBatchState.COMPLETED,
                total_steps=total,
                accepted_steps=accepted,
                message=f"已完成 · {accepted}/{total} 步已提交",
            )

    def _finish_stopped(self, batch_id: CommandBatchId, total: int, accepted: int) -> None:
        with self._lock:
            self._snapshot = CommandBatchSnapshot(
                batch_id=batch_id,
                state=CommandBatchState.STOPPED,
                total_steps=total,
                accepted_steps=accepted,
                message=f"已停止 · {accepted}/{total} 步已提交；已入队数据不会撤回",
            )

    def _finish_failed(
        self,
        batch_id: CommandBatchId,
        total: int,
        accepted: int,
        failed_step: int | None,
        error: ErrorInfo,
    ) -> None:
        detail = f"第 {failed_step}/{total} 步失败" if failed_step is not None else "执行失败"
        with self._lock:
            self._snapshot = CommandBatchSnapshot(
                batch_id=batch_id,
                state=CommandBatchState.FAILED,
                total_steps=total,
                accepted_steps=accepted,
                failed_step=failed_step,
                message=f"{detail} · 后续步骤未执行",
                error=error,
            )
