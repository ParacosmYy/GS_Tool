"""强调色选择持久化（Batch 11）。

Batch 10 的 accent 选择是纯运行时状态（``accents._active_override`` 模块级单例），
每次重启回到 cyan。本模块把用户选定的 accent 持久化到应用数据目录下的 JSON，
重启后恢复，让「多配色」真正落地为用户可长期使用的偏好。

持久化约定与 ``ui/panels/_dashboard_layout_store`` 一致：
- 目录：``QStandardPaths.AppDataLocation/embeddebug/``（兜底 ``~/.embeddebug``）。
- 文件：``accent.json``，内容 ``{"accent": "<id>"}``。
- 写入：原子替换（``.tmp`` + ``os.replace``），损坏/缺失回退 cyan 默认。

约束：本模块只依赖标准库 + PyQt6 的 QStandardPaths（仅路径解析），不访问
controller/transport，不 import accents（避免循环：accents 反向调用本模块持久化）。
"""

from __future__ import annotations

import json
import os
from pathlib import Path

ACCENT_FILENAME = "accent.json"

# 与 _dashboard_layout_store 约定的子目录名一致（同一应用数据根）。
_SUBDIR = "embeddebug"


def _app_data_dir() -> Path:
    """解析应用数据目录（QStandardPaths.AppDataLocation），兜底 ~/.embeddebug。

    与 ``_dashboard_layout_store._app_data_dir`` 同一逻辑（未直接复用以避免
    ui.theme → ui.panels 反向依赖；二者落点相同，由 QStandardPaths 决定）。
    """

    from PyQt6.QtCore import QStandardPaths

    locations = QStandardPaths.standardLocations(QStandardPaths.StandardLocation.AppDataLocation)
    for loc in locations:
        if loc:
            return Path(loc)
    return Path.home() / ".embeddebug"


def accent_path() -> Path:
    """accent 偏好 JSON 文件路径。"""

    return _app_data_dir() / _SUBDIR / ACCENT_FILENAME


def load_accent_id(default: str = "cyan") -> str:
    """读取已保存的 accent id；不存在/损坏回退 ``default``。"""

    p = accent_path()
    if not p.exists():
        return default
    try:
        raw = json.loads(p.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return default
    accent = raw.get("accent") if isinstance(raw, dict) else None
    if isinstance(accent, str) and accent:
        return accent
    return default


def save_accent_id(accent_id: str) -> bool:
    """原子写入 accent id，返回是否成功。"""

    p = accent_path()
    try:
        p.parent.mkdir(parents=True, exist_ok=True)
        tmp = p.with_suffix(p.suffix + ".tmp")
        tmp.write_text(
            json.dumps({"accent": accent_id}, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )
        os.replace(tmp, p)
    except OSError:
        return False
    return True
