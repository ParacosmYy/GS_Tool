"""波形数学函数库：通道算术 + 逐点一元函数 + 微积分。

全部接收/返回一维 ``np.ndarray``（float32），由 ``engine.py`` 从 ``ChannelBatch``
取出列后调用。逐点非法（除零、log 负数）产生 NaN，不抛——曲线照常绘制，坏点断线。

约定（对齐 waveform_measure/fft/histogram）：仅依赖 numpy + 标准库 ``math``；
不 import PyQt、不依赖 transport。
"""

from __future__ import annotations

import numpy as np

# 除零保护：真值小于该阈值视为 0 → 商置 NaN（而非 inf），曲线断点而非尖刺。
_DIV_EPS = 1e-30


# ── 通道算术 ────────────────────────────────────────────────────────
def add(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """逐点 ``a + b``。"""

    return (np.asarray(a) + np.asarray(b)).astype(np.float32, copy=False)


def subtract(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """逐点 ``a - b``。"""

    return (np.asarray(a) - np.asarray(b)).astype(np.float32, copy=False)


def multiply(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """逐点 ``a * b``。"""

    return (np.asarray(a) * np.asarray(b)).astype(np.float32, copy=False)


def divide(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """逐点 ``a / b``；|b| 极小处置 NaN（避免 inf 尖刺）。"""

    a = np.asarray(a, dtype=np.float64)
    b = np.asarray(b, dtype=np.float64)
    out = np.full_like(a, np.nan)
    safe = np.abs(b) > _DIV_EPS
    out[safe] = a[safe] / b[safe]
    return out.astype(np.float32, copy=False)


# ── 逐点一元函数 ──────────────────────────────────────────────────
def absolute(a: np.ndarray) -> np.ndarray:
    """``|a|``。"""

    return np.abs(np.asarray(a)).astype(np.float32, copy=False)


def sqrt(a: np.ndarray) -> np.ndarray:
    """``sqrt(a)``；负数 → NaN（而非抛 ValueError）。"""

    a = np.asarray(a, dtype=np.float64)
    out = np.full_like(a, np.nan)
    nonneg = a >= 0
    out[nonneg] = np.sqrt(a[nonneg])
    return out.astype(np.float32, copy=False)


def log(a: np.ndarray) -> np.ndarray:
    """自然对数；非正数 → NaN。"""

    a = np.asarray(a, dtype=np.float64)
    out = np.full_like(a, np.nan)
    positive = a > 0
    out[positive] = np.log(a[positive])
    return out.astype(np.float32, copy=False)


def sin(a: np.ndarray) -> np.ndarray:
    """``sin(a)``（弧度）。"""

    return np.sin(np.asarray(a)).astype(np.float32, copy=False)


def cos(a: np.ndarray) -> np.ndarray:
    """``cos(a)``（弧度）。"""

    return np.cos(np.asarray(a)).astype(np.float32, copy=False)


# ── 微积分 ──────────────────────────────────────────────────────────
def derivative(a: np.ndarray, dt: float = 1.0) -> np.ndarray:
    """一阶数值导数 ``da/dt``（中心差分，端点用单侧差分）。

    - ``dt``：样本间隔（秒）；默认 1.0（采样率归一化）。
    - 长度 < 2 返回全零数组（无差分定义）。
    """

    a = np.asarray(a, dtype=np.float64)
    if a.shape[0] < 2 or dt <= 0:
        return np.zeros_like(a, dtype=np.float32)
    grad = np.gradient(a, dt)
    return grad.astype(np.float32, copy=False)


def integral(a: np.ndarray, dt: float = 1.0) -> np.ndarray:
    """累积积分 ``∫a dt``（梯形法，cumulative）。

    - ``dt``：样本间隔（秒）；默认 1.0。
    - 返回与输入等长的累积值（第 0 点 = 0，即初始状态前无面积）。
      第 i 点 = ∫₀ⁱ a dt，用梯形法近似。
    """

    a = np.asarray(a, dtype=np.float64)
    if a.shape[0] == 0 or dt <= 0:
        return np.zeros_like(a, dtype=np.float32)
    # 累积梯形：每段面积 = (a[k-1]+a[k])/2 * dt，从第 1 点开始累加。
    # 第 0 点 = 0（无历史）。用 cumsum 前面补 0 实现等长输出。
    midpoints = (a[:-1] + a[1:]) / 2.0
    cumulative = np.concatenate(([0.0], np.cumsum(midpoints) * dt))
    return cumulative.astype(np.float32, copy=False)


# 表达式解析器可调用的函数名 → 实现映射（engine/expression 共用）。
UNARY_FUNCTIONS = {
    "abs": absolute,
    "sqrt": sqrt,
    "log": log,
    "sin": sin,
    "cos": cos,
    "derivative": derivative,
    "integral": integral,
}

BINARY_OPERATORS = {
    "+": add,
    "-": subtract,
    "*": multiply,
    "/": divide,
}

__all__ = [
    "BINARY_OPERATORS",
    "UNARY_FUNCTIONS",
    "absolute",
    "add",
    "cos",
    "derivative",
    "divide",
    "integral",
    "log",
    "multiply",
    "sin",
    "sqrt",
    "subtract",
]
