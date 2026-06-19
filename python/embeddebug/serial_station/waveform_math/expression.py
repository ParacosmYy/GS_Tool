"""安全表达式求值器（stdlib ``ast``，白名单节点遍历）。

把用户输入的数学表达式（如 ``"ch1 - ch2"``、``"abs(ch1) * 0.5"``、
``"derivative(ch1, 0.001)"``）解析成 AST，仅放行白名单节点（数字常量、通道名、
二元四则、一元正负、已注册函数调用），其余（属性访问、下标、赋值、lambda、
任意函数）一律拒。求值时通道名解析为 ``ChannelBatch`` 的列。

安全设计要点：
- 不用 ``eval`` / ``compile`` 直接执行；只遍历 AST 结构。
- ``Call`` 仅允许 ``UNARY_FUNCTIONS``/``BINARY_OPERATORS`` 中的已知名，
  未知函数 → ``ValueError``。
- ``Name`` 解析为调用方提供的 ``scope``（通道名 → 一维数组）。
- 整数参数（如 derivative 的 dt）允许传给单参函数，解析器按位置绑定。
"""

from __future__ import annotations

import ast
from typing import Callable

import numpy as np

from embeddebug.serial_station.waveform_math.functions import (
    BINARY_OPERATORS,
    UNARY_FUNCTIONS,
)


class ExpressionError(ValueError):
    """表达式不合法（语法错误 / 越界节点 / 未知名）。"""


# 白名单：ast 节点类型 → 处理器（在 _Evaluator 方法里分发）。
_ALLOWED_NODES = (
    ast.Expression,
    ast.BinOp,
    ast.UnaryOp,
    ast.Call,
    ast.Name,
    ast.Constant,
    ast.Add,
    ast.Sub,
    ast.Mult,
    ast.Div,
    ast.UAdd,
    ast.USub,
    ast.Load,
)


def _validate_tree(node: ast.AST) -> None:
    """递归校验 AST 只含白名单节点；越界 → ExpressionError。"""

    for child in ast.walk(node):
        if not isinstance(child, _ALLOWED_NODES):
            raise ExpressionError(
                f"不允许的语法节点: {type(child).__name__}（仅支持四则运算/已知函数/数字/通道名）"
            )


class _Evaluator:
    """AST → numpy 数组的求值器（节点方法分发）。"""

    def __init__(self, scope: dict[str, np.ndarray]) -> None:
        self._scope = scope

    def eval(self, expr: str) -> np.ndarray:
        try:
            tree = ast.parse(expr, mode="eval")
        except SyntaxError as exc:
            raise ExpressionError(f"表达式语法错误: {exc.msg}") from exc
        _validate_tree(tree)
        return self._visit(tree.body)

    def _visit(self, node: ast.AST) -> np.ndarray | float:
        if isinstance(node, ast.Constant):
            if isinstance(node.value, (int, float)):
                return float(node.value)
            raise ExpressionError(f"不支持的常量类型: {type(node.value).__name__}")
        if isinstance(node, ast.Name):
            return self._resolve_name(node.id)
        if isinstance(node, ast.UnaryOp):
            operand = self._visit(node.operand)
            if isinstance(node.op, ast.UAdd):
                return operand
            if isinstance(node.op, ast.USub):
                return -np.asarray(operand, dtype=np.float64)
            raise ExpressionError("未知一元运算符")
        if isinstance(node, ast.BinOp):
            return self._binop(node)
        if isinstance(node, ast.Call):
            return self._call(node)
        raise ExpressionError(f"未处理的节点: {type(node).__name__}")

    def _resolve_name(self, name: str) -> np.ndarray:
        if name in self._scope:
            return np.asarray(self._scope[name], dtype=np.float64)
        raise ExpressionError(f"未知通道名: {name}")

    def _binop(self, node: ast.BinOp) -> np.ndarray:
        op_symbol = _OP_SYMBOL.get(type(node.op))
        if op_symbol is None or op_symbol not in BINARY_OPERATORS:
            raise ExpressionError(f"未知二元运算符: {type(node.op).__name__}")
        left = self._visit(node.left)
        right = self._visit(node.right)
        return BINARY_OPERATORS[op_symbol](
            np.asarray(left, dtype=np.float64), np.asarray(right, dtype=np.float64)
        )

    def _call(self, node: ast.Call) -> np.ndarray:
        if not isinstance(node.func, ast.Name):
            raise ExpressionError("仅支持直接函数调用（不允许属性/方法调用）")
        fname = node.func.id
        func = _lookup_function(fname)
        if func is None:
            raise ExpressionError(f"未知函数: {fname}")
        # 关键字参数一律拒绝（数学函数无 kwargs）。
        if node.keywords:
            raise ExpressionError("不支持关键字参数")
        args = [self._visit(arg) for arg in node.args]
        return func(*args)


_OP_SYMBOL = {
    ast.Add: "+",
    ast.Sub: "-",
    ast.Mult: "*",
    ast.Div: "/",
}


def _lookup_function(name: str) -> Callable | None:
    """函数名 → 实现（合并一元/二元函数表）。"""

    if name in UNARY_FUNCTIONS:
        return UNARY_FUNCTIONS[name]
    return None


def evaluate(expression: str, scope: dict[str, np.ndarray]) -> np.ndarray:
    """安全求值：``expression`` 在 ``scope``（通道名→数组）下求值 → float32 数组。

    抛 ``ExpressionError``：语法错误 / 越界节点 / 未知通道名或函数。
    """

    evaluator = _Evaluator(scope)
    return np.asarray(evaluator.eval(expression), dtype=np.float32)


__all__ = ["ExpressionError", "evaluate"]
