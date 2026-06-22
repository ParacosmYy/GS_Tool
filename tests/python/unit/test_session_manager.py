"""SessionManager 会话持久化/节流/崩溃恢复/清理边界测试。

覆盖（用 tmp_path 真实文件 IO）：
1. 常量契约（SESSION_EXTENSION / AUTOSAVE_INTERVAL_S / _MARKER_SUFFIX）。
2. save + load 往返（写入 → 读回 → 字段一致）。
3. save 失败路径（只读目录 / 无效 path 返回 False）。
4. load 文件不存在返回 None。
5. autosave 节流（首次立即落盘 True / 间隔内 False / 超间隔 True）。
6. has_crash_recovery（文件存在非空 True / 不存在 False / 空文件 False）。
7. clear 删除会话 + 遗留 .lock.tmp + 返回 True。
8. __init__ now_ns 默认 0 + has_autosaved 初始 False。
"""

from __future__ import annotations

import time
from pathlib import Path

from embeddebug.serial_station.session.manager import (
    AUTOSAVE_INTERVAL_S,
    SESSION_EXTENSION,
    SessionManager,
    _MARKER_SUFFIX,
)
from embeddebug.serial_station.session.state import SessionState


# ── 常量契约 ──────────────────────────────────────────────────────
def test_session_extension_contract():
    assert SESSION_EXTENSION == ".edsession"


def test_autosave_interval_positive():
    assert AUTOSAVE_INTERVAL_S > 0
    assert AUTOSAVE_INTERVAL_S == 5.0


def test_marker_suffix_contract():
    assert _MARKER_SUFFIX == ".lock"


# ── __init__ ──────────────────────────────────────────────────────
def test_init_defaults():
    mgr = SessionManager()
    assert mgr._last_autosave_ns == 0
    assert mgr._has_autosaved is False


def test_init_with_now_ns():
    mgr = SessionManager(now_ns=12345)
    assert mgr._last_autosave_ns == 12345


# ── save + load 往返 ─────────────────────────────────────────────
def test_save_load_roundtrip(tmp_path):
    mgr = SessionManager()
    state = SessionState(
        transport_mode="uart",
        protocol="just_float",
        command_history=["AT+RST", "AT+GMR"],
        log_filter="ERROR",
        active_tab="serial",
        timestamp_ns=999,
    )
    path = tmp_path / f"session{SESSION_EXTENSION}"
    assert mgr.save(state, path) is True
    assert path.is_file()
    loaded = mgr.load(path)
    assert loaded is not None
    assert loaded.transport_mode == "uart"
    assert loaded.protocol == "just_float"
    assert loaded.command_history == ["AT+RST", "AT+GMR"]
    assert loaded.log_filter == "ERROR"
    assert loaded.active_tab == "serial"
    assert loaded.timestamp_ns == 999


def test_save_creates_parent_dirs(tmp_path):
    """save 应自动创建父目录（mkdir parents=True）。"""

    mgr = SessionManager()
    state = SessionState()
    nested = tmp_path / "nested" / "deep" / f"session{SESSION_EXTENSION}"
    assert mgr.save(state, nested) is True
    assert nested.is_file()


def test_save_failure_returns_false(tmp_path):
    """save 到无效路径返回 False（不抛异常）。"""

    mgr = SessionManager()
    state = SessionState()
    # Windows 非法字符路径。
    bad_path = Path("Z:\\nonexistent_root_xyz\\deep\\session.edsession")
    assert mgr.save(state, bad_path) is False


# ── load 边界 ─────────────────────────────────────────────────────
def test_load_nonexistent_returns_none(tmp_path):
    mgr = SessionManager()
    assert mgr.load(tmp_path / "missing.edsession") is None


def test_load_corrupt_does_not_crash(tmp_path):
    """损坏文件（非 JSON）load 不抛异常，返回 None 或默认状态（deserialize 宽容）。"""

    mgr = SessionManager()
    path = tmp_path / "corrupt.edsession"
    path.write_text("not valid json {{{", encoding="utf-8")
    result = mgr.load(path)
    # deserialize 对损坏内容返回默认状态或 None；manager 两种都接受，关键是不崩溃。
    assert result is None or hasattr(result, "transport_mode")


# ── autosave 节流 ────────────────────────────────────────────────
def test_autosave_first_call_writes(tmp_path):
    """首次 autosave 立即落盘（has_autosaved=False）。"""

    mgr = SessionManager()
    state = SessionState()
    path = tmp_path / "auto.edsession"
    assert mgr.autosave(state, path) is True
    assert path.is_file()
    assert mgr._has_autosaved is True


def test_autosave_throttled_within_interval(tmp_path):
    """间隔内第二次 autosave 不落盘（返回 False）。"""

    mgr = SessionManager()
    state = SessionState()
    path = tmp_path / "auto.edsession"
    mgr.autosave(state, path)  # 首次
    # 立即再调：间隔 < 5s → 节流。
    assert mgr.autosave(state, path) is False


def test_autosave_after_interval_writes(tmp_path):
    """超间隔后 autosave 再次落盘（手动后移 _last_autosave_ns）。"""

    mgr = SessionManager()
    state = SessionState()
    path = tmp_path / "auto.edsession"
    mgr.autosave(state, path)
    # 手动把上次保存时间后移到 > 间隔前，模拟时间流逝。
    mgr._last_autosave_ns = time.time_ns() - int(AUTOSAVE_INTERVAL_S * 1_000_000_000) - 1
    assert mgr.autosave(state, path) is True


# ── has_crash_recovery ───────────────────────────────────────────
def test_has_crash_recovery_existing_nonempty(tmp_path):
    path = tmp_path / "recover.edsession"
    path.write_text("data", encoding="utf-8")
    assert SessionManager().has_crash_recovery(path) is True


def test_has_crash_recovery_missing(tmp_path):
    assert SessionManager().has_crash_recovery(tmp_path / "no.edsession") is False


def test_has_crash_recovery_empty_file(tmp_path):
    path = tmp_path / "empty.edsession"
    path.write_text("", encoding="utf-8")
    assert SessionManager().has_crash_recovery(path) is False


# ── clear ─────────────────────────────────────────────────────────
def test_clear_removes_session_file(tmp_path):
    mgr = SessionManager()
    path = tmp_path / "session.edsession"
    mgr.save(SessionState(), path)
    assert path.is_file()
    assert mgr.clear(path) is True
    assert not path.exists()


def test_clear_removes_lock_tmp(tmp_path):
    """clear 应同时删除遗留 .lock.tmp 临时文件。"""

    mgr = SessionManager()
    path = tmp_path / "session.edsession"
    tmp_file = Path(f"{path}{_MARKER_SUFFIX}.tmp")
    tmp_file.write_text("partial", encoding="utf-8")
    assert tmp_file.exists()
    mgr.clear(path)
    assert not tmp_file.exists()


def test_clear_missing_file_returns_true(tmp_path):
    """清理不存在的文件返回 True（幂等）。"""

    assert SessionManager().clear(tmp_path / "no.edsession") is True
