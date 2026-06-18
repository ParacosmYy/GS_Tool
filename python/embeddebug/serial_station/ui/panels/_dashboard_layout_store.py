"""仪表盘布局自动持久化 helper（Batch 25）。

DashboardPanel 此前只有手动「保存/加载布局」按钮，关掉应用后布局丢失。
本 helper 提供自动持久化：build 时从应用数据目录恢复上一次布局，add/remove
控件时自动写回。布局存为 JSON 到 ``QStandardPaths.AppDataLocation`` 下的
``embeddebug/dashboard_layout.json``。

约束：只依赖 PyQt6 + dashboard canvas + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

import json
import os
from pathlib import Path

LAYOUT_FILENAME = "dashboard_layout.json"


def _app_data_dir() -> Path:
    """解析应用数据目录（QStandardPaths.AppDataLocation）。"""

    from PyQt6.QtCore import QStandardPaths

    locations = QStandardPaths.standardLocations(QStandardPaths.StandardLocation.AppDataLocation)
    for loc in locations:
        if loc:
            return Path(loc)
    # 兜底：用户家目录下的 .embeddebug。
    return Path.home() / ".embeddebug"


def layout_path() -> Path:
    """仪表盘布局 JSON 文件路径。"""

    return _app_data_dir() / "embeddebug" / LAYOUT_FILENAME


def load_layout_dict() -> dict:
    """读取已保存的布局 dict；不存在或损坏返回空 dict。"""

    p = layout_path()
    if not p.exists():
        return {}
    try:
        raw = json.loads(p.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    return raw if isinstance(raw, dict) else {}


def save_layout_dict(layout: dict) -> bool:
    """原子写入布局 dict，返回是否成功。"""

    p = layout_path()
    try:
        p.parent.mkdir(parents=True, exist_ok=True)
        tmp = p.with_suffix(p.suffix + ".tmp")
        tmp.write_text(json.dumps(layout, ensure_ascii=False, indent=2), encoding="utf-8")
        os.replace(tmp, p)
    except OSError:
        return False
    return True


def restore_to_canvas(canvas) -> int:
    """把已保存布局恢复到画布，返回恢复的控件数。

    用 canvas.load_layout 接口（接受 JSON 文件路径）的内存变体：先写到临时路径
    再调 load_layout，避免重复解析逻辑。
    """

    layout = load_layout_dict()
    items = layout.get("items")
    if not isinstance(items, list) or not items:
        return 0
    # 写到临时文件再用 canvas.load_layout（复用其 type/x/y/width/height/config 解析）。
    import tempfile

    try:
        fd, tmp_name = tempfile.mkstemp(suffix=".json", prefix="dashboard_restore.")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as fh:
                json.dump(layout, fh)
            return canvas.load_layout(tmp_name)
        finally:
            try:
                os.remove(tmp_name)
            except OSError:
                pass
    except OSError:
        return 0


def persist_from_canvas(canvas) -> bool:
    """把画布当前布局持久化（add/remove 后自动保存），返回是否成功。"""

    try:
        layout = canvas.to_layout_dict()
    except Exception:
        return False
    return save_layout_dict(layout)
