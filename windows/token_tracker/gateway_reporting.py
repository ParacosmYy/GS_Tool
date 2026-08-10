"""Bounded Usage Ingest delivery for the local Gateway.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Retry transient center-service failures with an optional encrypted queue.
Module: Local integration / usage reporting infrastructure
"""

from __future__ import annotations

from collections import deque
from dataclasses import dataclass
import threading
from time import monotonic, time
from typing import Any

import requests

from . import ingest_auth
from .gateway_contracts import UsageReport
from .gateway_queue import EncryptedUsageQueue, QueueItem


MAX_MODEL_LENGTH = 200
MAX_IDEMPOTENCY_KEY_LENGTH = 160
DEFAULT_MAX_PENDING = 256
DEFAULT_MAX_ATTEMPTS = 5
DEFAULT_RETRY_BASE_SECONDS = 2.0
MAX_RETRY_DELAY_SECONDS = 300.0


@dataclass
class _PendingReport:
    """One bounded retry item kept only for the current process lifetime."""

    report: UsageReport
    attempts: int
    next_attempt_at: float


@dataclass(frozen=True)
class _PostResult:
    """Classify one delivery without retaining response body or secrets."""

    delivered: bool
    retryable: bool


class UsageReporter:
    """Deliver usage synchronously once, then retry transient failures safely."""

    def __init__(
        self,
        ingest_url: str,
        ingest_token: str,
        report_timeout: int,
        *,
        max_pending: int = DEFAULT_MAX_PENDING,
        max_attempts: int = DEFAULT_MAX_ATTEMPTS,
        retry_base_seconds: float = DEFAULT_RETRY_BASE_SECONDS,
        queue_path: str | None = None,
    ) -> None:
        self._ingest_url = ingest_url
        self._ingest_token = ingest_token
        self._report_timeout = max(1, int(report_timeout))
        self._max_pending = _positive_bound(max_pending, "max_pending", 10000)
        self._max_attempts = _positive_bound(max_attempts, "max_attempts", 20)
        self._retry_base_seconds = _positive_float(retry_base_seconds, "retry_base_seconds")
        self._pending: deque[_PendingReport] = deque()
        self._condition = threading.Condition()
        self._worker: threading.Thread | None = None
        self._stopping = False
        self._persistent_queue = (
            EncryptedUsageQueue(queue_path, self._max_pending) if queue_path else None
        )
        if self._persistent_queue and self._persistent_queue.pending_count:
            with self._condition:
                self._ensure_worker_locked()

    def report(
        self,
        model: Any,
        input_tokens: Any,
        output_tokens: Any,
        idempotency_key: Any,
    ) -> str:
        """Return ``recorded``, ``queued``, ``missing`` or ``report-failed``."""

        report = _normalize_report(model, input_tokens, output_tokens, idempotency_key)
        if report is None:
            return "missing"
        result = self._post(report)
        if result.delivered:
            return "recorded"
        if not result.retryable:
            return "report-failed"
        return self._enqueue(report)

    def close(self, timeout: float = 1.0) -> None:
        """Stop the daemon worker while leaving persistent items recoverable."""

        with self._condition:
            self._stopping = True
            self._condition.notify_all()
            worker = self._worker
        if worker is not None:
            worker.join(timeout=max(0.0, float(timeout)))

    @property
    def pending_count(self) -> int:
        """Expose a bounded operational count without exposing report contents."""

        with self._condition:
            if self._persistent_queue:
                return self._persistent_queue.pending_count
            return len(self._pending)

    def _enqueue(self, report: UsageReport) -> str:
        with self._condition:
            if self._stopping:
                return "report-failed"
            if self._persistent_queue:
                stored = self._persistent_queue.enqueue(
                    report,
                    attempts=1,
                    next_attempt_at=time() + self._retry_base_seconds,
                )
                if not stored:
                    return "report-failed"
                self._ensure_worker_locked()
                self._condition.notify_all()
                return "queued"
            if len(self._pending) >= self._max_pending:
                return "report-failed"
            self._pending.append(
                _PendingReport(
                    report=report,
                    attempts=1,
                    next_attempt_at=monotonic() + self._retry_base_seconds,
                )
            )
            if self._worker is None or not self._worker.is_alive():
                self._worker = threading.Thread(
                    target=self._run,
                    name="ai-token-tracker-usage-reporter",
                    daemon=True,
                )
                self._worker.start()
            self._condition.notify_all()
        return "queued"

    def _run(self) -> None:
        while True:
            pending = self._next_persistent() if self._persistent_queue else self._next_pending()
            if pending is None:
                return
            result = self._post(pending.report)
            if result.delivered or not result.retryable:
                if self._persistent_queue:
                    self._persistent_queue.remove(pending.item_id)
                continue
            if pending.attempts >= self._max_attempts:
                if self._persistent_queue:
                    self._persistent_queue.remove(pending.item_id)
                continue
            pending.attempts += 1
            delay = min(
                MAX_RETRY_DELAY_SECONDS,
                self._retry_base_seconds * (2 ** (pending.attempts - 1)),
            )
            pending.next_attempt_at = (time() if self._persistent_queue else monotonic()) + delay
            if self._persistent_queue:
                self._persistent_queue.reschedule(
                    pending.item_id,
                    pending.attempts,
                    pending.next_attempt_at,
                )
                with self._condition:
                    self._condition.notify_all()
                continue
            with self._condition:
                if not self._stopping and len(self._pending) < self._max_pending:
                    self._pending.append(pending)
                    self._condition.notify_all()

    def _next_persistent(self) -> QueueItem | None:
        with self._condition:
            while not self._stopping:
                pending = self._persistent_queue.next_due(time())
                if pending is not None:
                    return pending
                next_attempt_at = self._persistent_queue.next_attempt_at()
                if next_attempt_at is None:
                    self._condition.wait()
                    continue
                self._condition.wait(timeout=max(0.0, next_attempt_at - time()))
            return None

    def _next_pending(self) -> _PendingReport | None:
        with self._condition:
            while not self._stopping:
                if not self._pending:
                    self._condition.wait()
                    continue
                pending = self._pending[0]
                wait_seconds = pending.next_attempt_at - monotonic()
                if wait_seconds > 0:
                    self._condition.wait(timeout=wait_seconds)
                    continue
                return self._pending.popleft()
            return None

    def _ensure_worker_locked(self) -> None:
        """Start one daemon worker; the caller must hold the condition lock."""

        if self._worker is None or not self._worker.is_alive():
            self._worker = threading.Thread(
                target=self._run,
                name="ai-token-tracker-usage-reporter",
                daemon=True,
            )
            self._worker.start()

    def _post(self, report: UsageReport) -> _PostResult:
        try:
            response = requests.post(
                self._ingest_url,
                headers={
                    ingest_auth.INGEST_TOKEN_HEADER: self._ingest_token,
                    "Idempotency-Key": report.idempotency_key,
                    "Content-Type": "application/json",
                },
                json={
                    "model": report.model,
                    "input_tokens": report.input_tokens,
                    "output_tokens": report.output_tokens,
                    "note": report.note,
                },
                timeout=self._report_timeout,
                allow_redirects=False,
                stream=True,
            )
        except requests.RequestException:
            return _PostResult(delivered=False, retryable=True)
        try:
            status_code = response.status_code
            if status_code in {200, 201}:
                return _PostResult(delivered=True, retryable=False)
            retryable = status_code in {408, 425, 429} or 500 <= status_code <= 599
            return _PostResult(delivered=False, retryable=retryable)
        finally:
            response.close()


def _normalize_report(
    model: Any,
    input_tokens: Any,
    output_tokens: Any,
    idempotency_key: Any,
) -> UsageReport | None:
    model_value = str(model or "").strip()
    key_value = str(idempotency_key or "").strip()
    input_value = _non_negative_int(input_tokens)
    output_value = _non_negative_int(output_tokens)
    if not 1 <= len(model_value) <= MAX_MODEL_LENGTH:
        return None
    if not key_value or len(key_value) > MAX_IDEMPOTENCY_KEY_LENGTH:
        return None
    if any(ord(character) < 32 or ord(character) == 127 for character in key_value):
        return None
    if input_value is None or output_value is None:
        return None
    return UsageReport(model_value, input_value, output_value, key_value)


def _non_negative_int(value: Any) -> int | None:
    if value is None or isinstance(value, bool):
        return None
    try:
        parsed = int(str(value).strip())
    except (TypeError, ValueError):
        return None
    return parsed if parsed >= 0 else None


def _positive_bound(value: Any, field_name: str, maximum: int) -> int:
    try:
        parsed = int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{field_name} must be a positive integer") from exc
    if not 1 <= parsed <= maximum:
        raise ValueError(f"{field_name} must be between 1 and {maximum}")
    return parsed


def _positive_float(value: Any, field_name: str) -> float:
    try:
        parsed = float(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{field_name} must be positive") from exc
    if parsed <= 0:
        raise ValueError(f"{field_name} must be positive")
    return parsed
