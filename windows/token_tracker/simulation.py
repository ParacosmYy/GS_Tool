"""Real-time / Live Simulation engine for the token tracker.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Generate reproducible usage streams, aggregate them, and persist them
         to the user's own records on explicit commit only.
Module: Simulation engine boundary

This module owns the entire "Live Simulation" use case: job lifecycle, the
background worker that produces events, aggregation, and the commit boundary
that calls back into ``services.add_usage_result``. It is deliberately free of
HTTP, templates, and Flask ``g`` state so it can run inside a daemon thread.

Security / data invariants enforced here:
- Events are only persisted when ``commit_job`` is called by an authenticated
  caller; the engine never writes on its own.
- A job may be committed at most once (``events_committed`` guard).
- ``get_state`` never serializes the ``threading.Lock`` or the ``_stop`` flag,
  so the snapshot is safe to hand to ``jsonify``.
- No API key, secret, or credential is ever stored on a ``JobState``.
"""

from __future__ import annotations

import threading
import time
import uuid
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import Any

from flask import current_app, has_app_context

from . import services


# Models used by the synthetic stream. Purely for display / shape, not a
# provider allowlist, so this list does not widen any real security boundary.
MODEL_POOL: list[str] = [
    "moonshot-v1-8k",
    "gpt-4o-mini",
    "claude-3-5-sonnet",
    "deepseek-chat",
    "qwen-max",
    "glm-4-plus",
]

# Display-only unit prices (USD per 1k tokens). These are illustrative and are
# NEVER used for billing; they exist so the simulation shows a plausible cost
# trend. Real provider pricing is not embedded here.
PRICE_PER_1K: dict[str, dict[str, float]] = {
    "moonshot-v1-8k": {"in": 0.012, "out": 0.012},
    "gpt-4o-mini": {"in": 0.00015, "out": 0.0006},
    "claude-3-5-sonnet": {"in": 0.003, "out": 0.015},
    "deepseek-chat": {"in": 0.00027, "out": 0.0011},
    "qwen-max": {"in": 0.004, "out": 0.012},
    "glm-4-plus": {"in": 0.005, "out": 0.005},
}

# Module-level job registry. Jobs are identified by a random hex id and live for
# the process lifetime; there is intentionally no cross-process persistence.
JOBS: dict[str, "JobState"] = {}

# Hard safety bound so a single job can never grow unbounded in memory or time.
MAX_ROUNDS = 100_000
MIN_SPEED_MS = 10
MAX_SPEED_MS = 500


@dataclass
class SimulationEvent:
    """One synthetic model call produced by a running simulation.

    All fields are plain, JSON-serializable scalars so an event can be returned
    directly through the API envelope without further transformation.
    """

    model: str
    input_tokens: int
    output_tokens: int
    cost: float
    timestamp: str  # ISO-8601 wall-clock text

    def to_dict(self) -> dict[str, Any]:
        """Project the event as a JSON-safe mapping (same keys as attributes)."""

        return {
            "model": self.model,
            "input_tokens": self.input_tokens,
            "output_tokens": self.output_tokens,
            "cost": self.cost,
            "timestamp": self.timestamp,
        }


@dataclass
class JobState:
    """Mutable, thread-safe state for one simulation run.

    Invariants:
    - ``_lock`` guards every read-modify-write of the aggregates below.
    - ``_stop`` is the only flag the worker checks to exit early.
    - ``events_committed`` flips to True exactly once and is irreversible.
    """

    job_id: str
    total_rounds: int
    speed_ms: int
    status: str = "running"
    round: int = 0
    total_input_tokens: int = 0
    total_output_tokens: int = 0
    total_cost: float = 0.0
    per_model: dict[str, dict[str, Any]] = field(default_factory=dict)
    trend: list[float] = field(default_factory=list)
    events: list[SimulationEvent] = field(default_factory=list)
    events_committed: bool = False
    _stop: bool = False
    _lock: threading.Lock = field(default_factory=threading.Lock)

    def snapshot(self) -> dict[str, Any]:
        """Return a JSON-safe projection that excludes lock/stop internals."""

        return {
            "job_id": self.job_id,
            "status": self.status,
            "round": self.round,
            "total_rounds": self.total_rounds,
            "total_input_tokens": self.total_input_tokens,
            "total_output_tokens": self.total_output_tokens,
            "total_tokens": self.total_input_tokens + self.total_output_tokens,
            "total_cost": self.total_cost,
            "per_model": self.per_model,
            "trend": self.trend,
            "events_committed": self.events_committed,
        }


def create_job(rounds: int, speed_ms: int, seed: int | None) -> str:
    """Start one simulation job and return its id.

    Args:
        rounds: Number of synthetic rounds to generate. Clamped/validated to
            ``1 <= rounds <= 100_000``; caller-provided defaults are 1000.
        speed_ms: Delay between rounds in milliseconds, ``10 <= speed_ms <= 500``;
            default 40.
        seed: Optional RNG seed for reproducibility; ``None`` means non-repeatable.

    Returns:
        The new job's ``job_id`` (hex uuid4).

    Failure semantics:
        Raises ``ValueError`` when ``rounds`` or ``speed_ms`` is out of bounds.
        The job is always registered as ``"running"`` and a daemon worker thread
        is spawned immediately.

    Permission: none (auth is enforced by the API layer before calling this).
    """

    if not isinstance(rounds, int) or isinstance(rounds, bool) or rounds < 1 or rounds > MAX_ROUNDS:
        raise ValueError(f"rounds must be an integer between 1 and {MAX_ROUNDS}")
    if not isinstance(speed_ms, int) or isinstance(speed_ms, bool) or speed_ms < MIN_SPEED_MS or speed_ms > MAX_SPEED_MS:
        raise ValueError(f"speed_ms must be an integer between {MIN_SPEED_MS} and {MAX_SPEED_MS}")

    job_id = uuid.uuid4().hex
    state = JobState(job_id=job_id, total_rounds=rounds, speed_ms=speed_ms)
    JOBS[job_id] = state
    worker = threading.Thread(target=run_job, args=(job_id, seed), daemon=True)
    worker.start()
    return job_id


def run_job(job_id: str, seed: int | None) -> None:
    """Background worker: produce ``total_rounds`` events and aggregate them.

    Args:
        job_id: Id of the job created by ``create_job``.
        seed: Optional RNG seed; ``None`` uses system entropy (non-reproducible).

    Behavior:
        - Uses ``random.Random(seed)`` so the same seed yields the same stream.
        - Each round: pick a model, sample token counts, compute display cost,
          and append an event under ``state._lock``.
        - ``timestamp`` increments by ``speed_ms`` so the trend looks time-based.
        - Honors ``state._stop`` (set ``True`` via ``stop_job``) by marking the
          job ``"stopped"`` and breaking early.
        - On natural completion sets ``status = "done"``.

    Failure semantics:
        Exceptions are swallowed per-round except for fatal ones, which mark the
        job ``"stopped"`` rather than crashing the daemon thread. The registry is
        left intact so the caller can still fetch a final snapshot.
    """

    state = JOBS.get(job_id)
    if state is None:
        return

    rng = __import__("random").Random(seed)
    start = datetime.now()
    step_seconds = state.speed_ms / 1000.0

    for index in range(state.total_rounds):
        with state._lock:
            if state._stop:
                state.status = "stopped"
                return
        try:
            model = rng.choice(MODEL_POOL)
            input_tokens = rng.randint(200, 8000)
            output_tokens = rng.randint(100, 4000)
            prices = PRICE_PER_1K.get(model, {"in": 0.0, "out": 0.0})
            cost = (prices["in"] * input_tokens + prices["out"] * output_tokens) / 1000.0
            event_time = start + timedelta(seconds=step_seconds * index)
            event = SimulationEvent(
                model=model,
                input_tokens=input_tokens,
                output_tokens=output_tokens,
                cost=cost,
                timestamp=event_time.replace(microsecond=0).isoformat(),
            )
        except Exception:
            # A malformed round must not kill the daemon thread.
            with state._lock:
                state.status = "stopped"
            return

        with state._lock:
            if state._stop:
                state.status = "stopped"
                return
            state.round += 1
            state.total_input_tokens += input_tokens
            state.total_output_tokens += output_tokens
            state.total_cost += cost
            bucket = state.per_model.setdefault(model, {"calls": 0, "tokens": 0, "cost": 0.0})
            bucket["calls"] += 1
            bucket["tokens"] += input_tokens + output_tokens
            bucket["cost"] += cost
            state.trend.append(state.total_input_tokens + state.total_output_tokens)
            state.events.append(event)

        time.sleep(step_seconds)

    with state._lock:
        if state._stop:
            state.status = "stopped"
        else:
            state.status = "done"


def get_state(job_id: str) -> dict[str, Any] | None:
    """Return a JSON-safe snapshot of a job, or ``None`` if unknown.

    Args:
        job_id: Id previously returned by ``create_job``.

    Returns:
        Mapping with job status, aggregates, ``per_model``, ``trend`` and
        ``events_committed``. Never contains the ``Lock`` or ``_stop`` flag.

    Failure semantics:
        Unknown job ids return ``None`` (the API layer maps this to 404).
    """

    state = JOBS.get(job_id)
    if state is None:
        return None
    return state.snapshot()


def stop_job(job_id: str) -> bool:
    """Request early termination of a running job.

    Args:
        job_id: Id of the job to stop.

    Returns:
        ``True`` if a job with that id exists (and the stop flag was set),
        ``False`` if the job is unknown.

    Failure semantics:
        Idempotent and safe to call on an already-finished job; it only sets the
        cooperative ``_stop`` flag.
    """

    state = JOBS.get(job_id)
    if state is None:
        return False
    with state._lock:
        state._stop = True
    return True


def commit_job(job_id: str, user_id: int) -> int:
    """Persist a job's synthetic events into the caller's own usage records.

    Args:
        job_id: Id of a (running, paused, stopped, or done) job.
        user_id: Authenticated user that owns the resulting records. The engine
            never infers an owner; it trusts the API layer to pass the verified
            ``g.user["id"]``.

    Returns:
        The number of events committed (length of ``state.events``).

    Failure semantics:
        - Raises ``ValueError("already committed")`` if the job was committed
          before; the API layer turns this into a 409.
        - Unknown job ids raise ``KeyError`` rather than silently committing
          nothing, so the caller must ``get_state`` first for a clean 404.
        - Each event is written through ``services.add_usage_result`` (parameterized
          SQL inside ``db``); the database path resolves from the active Flask app
          config when available, otherwise falls back to the default resolver.

    Permission: caller must be authenticated; the engine itself enforces only the
    single-commit invariant, not RBAC (that is the API layer's job).
    """

    state = JOBS.get(job_id)
    if state is None:
        raise KeyError(job_id)
    if state.events_committed:
        raise ValueError("already committed")

    # Resolve the configured database path inside the request's app context so the
    # simulation writes to the same SQLite file the rest of the app uses. Outside
    # an app context this falls back to the default resolver (path=None).
    db_path: str | None = None
    if has_app_context():
        try:
            db_path = str(current_app.config.get("DATABASE"))
        except Exception:
            db_path = None

    committed = 0
    for event in state.events:
        services.add_usage_result(
            user_id=user_id,
            model=event.model,
            input_tokens=event.input_tokens,
            output_tokens=event.output_tokens,
            timestamp=event.timestamp,
            note="simulation",
            source="simulation",
            path=db_path,
        )
        committed += 1

    with state._lock:
        state.events_committed = True
    return committed
