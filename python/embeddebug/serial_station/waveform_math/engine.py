"""波形数学引擎：把 ``VirtualChannel`` 应用到 ``ChannelBatch`` 得到 ``MathResult``。

编排层：从 ``ChannelBatch.values``（2D [n_samples, n_channels]）按 ``channel_names``
取出列，组装成表达式求值器的 scope，再调 ``expression.evaluate`` 求值。逐通道独立
求值，单个通道失败（未知名/语法错）返回带 ``error`` 的 ``MathResult``，不影响其他通道。

不 import PyQt、不依赖 transport。``ChannelBatch`` 来自 ``core.measurements``。
"""

from __future__ import annotations

import numpy as np

from embeddebug.serial_station.core.measurements import ChannelBatch
from embeddebug.serial_station.waveform_math.expression import (
    ExpressionError,
    evaluate,
)
from embeddebug.serial_station.waveform_math.model import MathResult, VirtualChannel


def build_scope(batch: ChannelBatch) -> dict[str, np.ndarray]:
    """``ChannelBatch`` → 表达式 scope（通道名 → 一维 float64 数组）。

    名字查找大小写敏感；重名时后者覆盖（SVD/数据源一般无重名）。
    """

    scope: dict[str, np.ndarray] = {}
    values = batch.values
    for index, name in enumerate(batch.channel_names):
        scope[name] = np.asarray(values[:, index], dtype=np.float64)
    return scope


def evaluate_channel(
    channel: VirtualChannel, batch: ChannelBatch
) -> MathResult:
    """对单条虚拟通道在 ``batch`` 上求值 → ``MathResult``。

    - 表达式语法错 / 未知通道名 / 未知函数 → ``MathResult(error=...)``，values 空。
    - 逐点非法（除零、log 负数）已在 ``functions`` 内置为 NaN，不报错。
    - 空批次（0 样本）→ 空 values，``ok``。
    """

    if batch.values.shape[0] == 0:
        return MathResult(values=np.empty(0, dtype=np.float32))
    scope = build_scope(batch)
    try:
        result = evaluate(channel.expression, scope)
    except ExpressionError as exc:
        return MathResult(values=np.empty(0, dtype=np.float32), error=str(exc))
    return MathResult(values=result)


def evaluate_channels(
    channels: list[VirtualChannel], batch: ChannelBatch
) -> list[MathResult]:
    """批量求值（顺序与 ``channels`` 一致）；单通道失败不影响其他。"""

    return [evaluate_channel(ch, batch) for ch in channels]


__all__ = [
    "build_scope",
    "evaluate_channel",
    "evaluate_channels",
]
