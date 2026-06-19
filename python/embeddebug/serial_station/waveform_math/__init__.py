"""波形数学引擎（Wave 58）。

提供虚拟通道数学运算：通道间四则（A+B/A-B/A*B/A/B）、逐点一元函数
（abs/sqrt/log/sin/cos）、微积分（derivative/integral）、基于 ``ast`` 的安全
表达式解析器。输入 ``ChannelBatch``（2D float32 [n_samples, n_channels]），
输出 ``MathResult``（一维 float32 数组 + 错误信息）。

仅依赖 numpy + 标准库；不 import PyQt、不依赖 transport。UI/面板层只 import
本 ``__init__`` 聚合的公共 API。

公开符号：
- ``VirtualChannel`` / ``MathResult``：数据模型。
- ``evaluate`` / ``ExpressionError``：表达式求值入口与异常。
- ``evaluate_channel`` / ``evaluate_channels`` / ``build_scope``：引擎编排。
- 函数库（``add/subtract/...`` 等）直接导出，供程序化构造无需表达式时使用。
"""

from __future__ import annotations

from embeddebug.serial_station.waveform_math.engine import (
    build_scope,
    evaluate_channel,
    evaluate_channels,
)
from embeddebug.serial_station.waveform_math.expression import (
    ExpressionError,
    evaluate,
)
from embeddebug.serial_station.waveform_math.functions import (
    absolute,
    add,
    cos,
    derivative,
    divide,
    integral,
    log,
    multiply,
    sin,
    sqrt,
    subtract,
)
from embeddebug.serial_station.waveform_math.model import MathResult, VirtualChannel

__all__ = [
    "ExpressionError",
    "MathResult",
    "VirtualChannel",
    "absolute",
    "add",
    "build_scope",
    "cos",
    "derivative",
    "divide",
    "evaluate",
    "evaluate_channel",
    "evaluate_channels",
    "integral",
    "log",
    "multiply",
    "sin",
    "sqrt",
    "subtract",
]
