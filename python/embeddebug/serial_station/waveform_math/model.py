"""波形数学引擎数据模型。

封装虚拟通道定义与计算结果。``VirtualChannel`` 描述一条用户定义的数学通道
（表达式 + 名称），``MathResult`` 是对一批样本求值后的产物（一维 float32 数组 +
统计摘要）。仅依赖 numpy，不 import PyQt、不依赖 transport。
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass(frozen=True)
class VirtualChannel:
    """用户定义的数学通道：表达式引用源通道名，求值后得到一条新曲线。

    - ``name``：虚拟通道显示名（如 ``"A-B"`` / ``"derivative(B)"``）。
    - ``expression``：数学表达式字符串（如 ``"ch1 - ch2"``、``"abs(ch1) * 0.5"``），
      由 ``expression.py`` 的安全解析器求值。
    - ``color_hint``：可选颜色提示（面板渲染用，引擎本身不读）。
    """

    name: str
    expression: str
    color_hint: str = ""

    def __post_init__(self) -> None:
        if not self.name.strip():
            raise ValueError("virtual channel name must not be empty")
        if not self.expression.strip():
            raise ValueError("virtual channel expression must not be empty")


@dataclass(frozen=True)
class MathResult:
    """一条虚拟通道对一批样本求值的结果。

    - ``values``：一维 float32 数组（与源样本行数一致）；含 NaN 表示逐点非法
      （如除零、log 负数），不阻断整条曲线。
    - ``error``：非 None 表示整条表达式求值失败（如未知通道名、语法错误），
      此时 ``values`` 为空数组。
    """

    values: np.ndarray
    error: str | None = None

    def __post_init__(self) -> None:
        values = np.asarray(self.values, dtype=np.float32).ravel()
        object.__setattr__(self, "values", values)

    @property
    def ok(self) -> bool:
        """整条表达式是否成功求值（``error is None``）。"""

        return self.error is None

    @property
    def sample_count(self) -> int:
        """结果样本数（失败时为 0）。"""

        return int(self.values.shape[0])


__all__ = ["MathResult", "VirtualChannel"]
