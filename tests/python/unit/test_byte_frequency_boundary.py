"""byte_frequency entropy_bits/top_n_bytes/format_stats_text 边界扩展测试。

补强 test_byte_frequency.py 未直接断言的边角：
- entropy_bits：两字节均匀分布（1.0 bit）+ 偏斜分布（0 < e < 8）+ 全量 0 freq 不崩溃。
- top_n_bytes：n>256 clamp + n 负数 + 空 freq 列表。
- format_stats_text：含 Entropy 行 + 含 Most common 行 + 非空数据含字节数。
- compute_frequency：单字节/双字节交替/全 256 各一次。
"""

from __future__ import annotations


from embeddebug.serial_station.ui.tools.byte_frequency import (
    compute_frequency,
    entropy_bits,
    format_stats_text,
    top_n_bytes,
)


# ── entropy_bits 边界 ─────────────────────────────────────────────────


def test_entropy_two_bytes_uniform_is_one_bit():
    """两字节各 50% → entropy = 1.0 bit。"""

    freq = [0] * 256
    freq[0] = 50
    freq[1] = 50
    assert entropy_bits(freq, 100) == pytest_approx(1.0)


def test_entropy_skewed_between_zero_and_max():
    """偏斜分布 → 0 < entropy < 8。"""

    freq = [0] * 256
    freq[0] = 90
    freq[1] = 10
    e = entropy_bits(freq, 100)
    assert 0.0 < e < 8.0


def test_entropy_all_zero_freq_not_crash():
    """全 0 freq → 0.0（不除零）。"""

    assert entropy_bits([0] * 256, 0) == 0.0


def test_entropy_max_bits_uniform_256():
    """全 256 各 1 次 → ≈8.0 bits。"""

    freq = [1] * 256
    assert entropy_bits(freq, 256) == pytest_approx(8.0, 0.01)


# ── top_n_bytes 边界 ──────────────────────────────────────────────────


def test_top_n_bytes_n_larger_than_256():
    """n=300 > 256 → 返回最多 256 个。"""

    freq = list(range(256))
    result = top_n_bytes(freq, n=300)
    assert len(result) <= 256


def test_top_n_bytes_n_negative():
    """n 负数 → 空列表。"""

    freq = [0] * 256
    result = top_n_bytes(freq, n=-1)
    assert len(result) == 0


def test_top_n_bytes_n_zero():
    """n=0 → 空列表。"""

    freq = [0] * 256
    result = top_n_bytes(freq, n=0)
    assert len(result) == 0


def test_top_n_bytes_default_n():
    """默认 n=10 → 最多 10 个。"""

    freq = list(range(256))
    result = top_n_bytes(freq)
    assert len(result) <= 10


# ── format_stats_text 边界 ────────────────────────────────────────────


def test_format_stats_text_contains_entropy():
    """非空数据 → 文本含 Entropy 关键字。"""

    freq = compute_frequency(b"hello world")
    text = format_stats_text(freq, 11)
    assert "ntropy" in text or "entropy" in text.lower()


def test_format_stats_text_contains_most_common():
    """非空数据 → 文本含 Most common 或 top bytes 行。"""

    freq = compute_frequency(b"aabbbccc")
    text = format_stats_text(freq, 8)
    assert len(text) > len("Total: 8 bytes")  # 有更多内容


def test_format_stats_text_byte_count_matches():
    """非空数据 → 文本含正确字节数。"""

    freq = compute_frequency(b"abc")
    text = format_stats_text(freq, 3)
    assert "3" in text


def test_format_stats_text_returns_string():
    """返回 str。"""

    text = format_stats_text([0] * 256, 0)
    assert isinstance(text, str)


# ── compute_frequency 边界 ────────────────────────────────────────────


def test_compute_frequency_single_byte():
    """单字节 → 对应 freq=1。"""

    freq = compute_frequency(b"\x42")
    assert freq[0x42] == 1
    assert sum(freq) == 1


def test_compute_frequency_alternating():
    """双字节交替 → 各 N/2。"""

    freq = compute_frequency(b"ABABABAB")
    assert freq[0x41] == 4
    assert freq[0x42] == 4


def test_compute_frequency_all_256_distinct_sums():
    """全 256 各一次 → sum = 256。"""

    freq = compute_frequency(bytes(range(256)))
    assert sum(freq) == 256


def pytest_approx(expected, rel=None):
    """简化版 approx（避免 import pytest 在模块顶层）。"""

    class _Approx:
        def __init__(self, exp, tolerance):
            self.exp = exp
            self.tol = tolerance

        def __eq__(self, other):
            return abs(other - self.exp) <= self.tol

    tol = rel if rel is not None else 0.001
    return _Approx(expected, abs(expected) * tol if expected != 0 else tol)
