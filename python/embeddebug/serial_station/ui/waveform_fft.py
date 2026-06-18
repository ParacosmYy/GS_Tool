"""实时 FFT 计算（对齐 VOFA+ 频域图）。

支持：
- 点数可设（256~16384，2 的幂）。
- 窗函数可选：矩形（无窗）、汉宁、汉明、布莱克曼。
- 返回幅度谱（dB 或线性）+ 频率轴。

约束：本模块只依赖 numpy + 标准库，不 import PyQt（纯计算，便于单测）。
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

# 支持的 FFT 点数（2 的幂，覆盖 VOFA+ 典型范围）。
SUPPORTED_FFT_POINTS: tuple[int, ...] = (256, 512, 1024, 2048, 4096, 8192, 16384)
DEFAULT_FFT_POINTS = 1024

WINDOW_RECT = "rect"      # 矩形窗（无窗）
WINDOW_HANN = "hann"      # 汉宁窗
WINDOW_HAMMING = "hamming"  # 汉明窗
WINDOW_BLACKMAN = "blackman"  # 布莱克曼窗
SUPPORTED_WINDOWS: tuple[str, ...] = (
    WINDOW_RECT, WINDOW_HANN, WINDOW_HAMMING, WINDOW_BLACKMAN,
)


@dataclass(frozen=True)
class FFTResult:
    """一次 FFT 计算结果。"""

    frequencies: np.ndarray   # 频率轴（Hz），长度 N/2
    magnitudes: np.ndarray    # 幅度谱（线性），长度 N/2
    magnitudes_db: np.ndarray  # 幅度谱（dB），长度 N/2


def window_coefficients(points: int, window: str) -> np.ndarray:
    """返回指定窗函数系数，矩形窗返回全 1。"""

    if window == WINDOW_RECT or window not in SUPPORTED_WINDOWS:
        return np.ones(points, dtype=np.float32)
    if window == WINDOW_HANN:
        return np.hanning(points).astype(np.float32)
    if window == WINDOW_HAMMING:
        return np.hamming(points).astype(np.float32)
    # blackman
    return np.blackman(points).astype(np.float32)


def compute_fft(
    signal: np.ndarray,
    sample_rate: float,
    points: int = DEFAULT_FFT_POINTS,
    window: str = WINDOW_HANN,
) -> FFTResult:
    """对单通道信号做 FFT，返回幅度谱与频率轴。

    - 信号不足 points 时末尾补零，超过时取末尾 points 个样本。
    - 应用窗函数后做 rfft（实信号，返回 N/2+1 个 bin）。
    - 幅度归一化（除以 N/2），dB 版本用 20*log10。
    """

    if points <= 0 or sample_rate <= 0:
        return FFTResult(
            frequencies=np.array([], dtype=np.float32),
            magnitudes=np.array([], dtype=np.float32),
            magnitudes_db=np.array([], dtype=np.float32),
        )
    n = min(points, signal.shape[0])
    if n == 0:
        return FFTResult(
            frequencies=np.array([], dtype=np.float32),
            magnitudes=np.array([], dtype=np.float32),
            magnitudes_db=np.array([], dtype=np.float32),
        )
    # 取末尾 points 个样本，不足补零。
    frame = np.zeros(points, dtype=np.float32)
    frame[:n] = signal[-n:].astype(np.float32)
    win = window_coefficients(points, window)
    windowed = frame * win
    spectrum = np.fft.rfft(windowed)
    half = points / 2.0
    magnitudes = (np.abs(spectrum) / half).astype(np.float32)
    # 防 log10(0)。
    safe = np.where(magnitudes > 1e-12, magnitudes, 1e-12)
    magnitudes_db = (20.0 * np.log10(safe)).astype(np.float32)
    frequencies = (np.fft.rfftfreq(points, d=1.0 / sample_rate)).astype(np.float32)
    return FFTResult(
        frequencies=frequencies,
        magnitudes=magnitudes,
        magnitudes_db=magnitudes_db,
    )
