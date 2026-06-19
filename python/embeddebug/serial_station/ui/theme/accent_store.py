"""强调色选择持久化（Batch 11）—— 向后兼容 shim。

Batch 12 把 theme+accent 统一迁到 ``theme_store``（单一 ``theme_prefs.json``）。
本模块保留为薄 shim，把 save/load 转发到 ``theme_store``，使 Batch 11 既有调用
（``accents.set_active_accent`` 持久化路径）与测试（``test_accent_store``）无需改动。

历史：Batch 11 用单独的 ``accent.json``；Batch 12 起并入 ``theme_prefs.json``，
``accent_path`` 仍指向旧文件名仅供迁移读取（``theme_store._read_prefs_dict`` 会
回退读取旧 accent.json）。

约束：只依赖标准库 + theme_store，不访问 controller/transport，不 import accents。
"""

from __future__ import annotations

# 旧文件名保留导出（测试 / 迁移读取引用）。
ACCENT_FILENAME = "accent.json"

from embeddebug.serial_station.ui.theme import theme_store  # noqa: E402


def accent_path():
    """旧 accent.json 路径（迁移期读取兼容；新写入走 theme_store.prefs_path）。"""

    return theme_store._legacy_accent_path()


def load_accent_id(default: str = "cyan") -> str:
    """读取已保存的 accent id；不存在/损坏回退 ``default``。

    委托 ``theme_store.load_accent_id``（从统一 theme_prefs.json 读，含旧 accent.json
    迁移回退）。
    """

    return theme_store.load_accent_id(default=default)


def save_accent_id(accent_id: str) -> bool:
    """保存 accent id（保留当前 theme 不变）。委托 ``theme_store``。"""

    return theme_store.save_accent_id(accent_id)
