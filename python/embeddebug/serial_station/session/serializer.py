"""会话状态序列化器。

负责 ``SessionState`` 与 JSON 字符串之间的相互转换。引入 ``version``
字段以便后续结构演进时保持向前兼容；解析阶段对缺失字段使用默认值，
避免因旧会话文件结构不完整而崩溃。
"""

from __future__ import annotations

import json
from typing import Any

from embeddebug.serial_station.session.state import SessionState

FORMAT_VERSION = 1
"""当前序列化格式版本，向前兼容解析时依据该字段选择回退策略。"""

_MIN_SUPPORTED_VERSION = 1
_MAX_SUPPORTED_VERSION = 1


class SessionSerializer:
    """会话状态 JSON 序列化器。

    该类不持有可变状态，可作为无副作用工具类使用，便于在不同进程间
    安全共享（例如崩溃恢复守护进程）。
    """

    @staticmethod
    def serialize(state: SessionState) -> str:
        """将 ``SessionState`` 序列化为 JSON 字符串。

        Args:
            state: 待序列化的会话状态。

        Returns:
            包含 ``version`` 与 ``payload`` 字段的紧凑 JSON 文本。
        """

        document = {
            "version": FORMAT_VERSION,
            "payload": state.to_dict(),
        }
        return json.dumps(document, ensure_ascii=False, separators=(",", ":"))

    @staticmethod
    def deserialize(json_str: str) -> SessionState:
        """从 JSON 字符串解析出 ``SessionState``。

        对解析失败或结构缺失的情况均返回默认状态，不抛出异常，从而保证
        调用方在崩溃恢复路径上无需额外捕获异常。

        Args:
            json_str: 由 :meth:`serialize` 产出的 JSON 文本。

        Returns:
            解析得到的会话状态；任何异常都退化为默认 ``SessionState``。
        """

        if not isinstance(json_str, str) or not json_str.strip():
            return SessionState()

        try:
            document: Any = json.loads(json_str)
        except (ValueError, TypeError):
            return SessionState()

        if isinstance(document, dict) and "payload" in document and isinstance(document["payload"], dict):
            payload = document["payload"]
            version = document.get("version", FORMAT_VERSION)
            if not _is_supported_version(version):
                # 未知版本仍尝试按当前结构解析，缺失字段由 from_dict 兜底。
                pass
            return SessionState.from_dict(payload)

        # 兼容直接以状态字典形式保存的旧文件。
        if isinstance(document, dict):
            return SessionState.from_dict(document)

        return SessionState()


def _is_supported_version(version: Any) -> bool:
    """判断版本号是否落在当前实现可识别的区间内。"""

    try:
        numeric = int(version)
    except (TypeError, ValueError):
        return False
    return _MIN_SUPPORTED_VERSION <= numeric <= _MAX_SUPPORTED_VERSION
