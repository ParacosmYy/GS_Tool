"""主题偏好持久化（Batch 12）—— theme + accent 统一落盘。

Batch 11 引入 ``accent_store`` 单独持久化 accent id；Batch 12 把深/浅主题选择
也持久化，使重启后同时恢复 theme + accent。为避免两个 JSON 文件分散且易失同步，
本模块用单个 ``theme_prefs.json`` 统一保存两者：

.. code-block:: json

    {"theme": "serial_station_light", "accent": "purple"}

持久化约定与 ``ui/panels/_dashboard_layout_store`` 一致：
- 目录：``QStandardPaths.AppDataLocation/embeddebug/``（兜底 ``~/.embeddebug``）。
- 写入：原子替换（``.tmp`` + ``os.replace``）。
- 缺失/损坏回退各自默认（theme=深色，accent=cyan）。

向后兼容：``accent_store``（Batch 11）保留为薄 shim，其 save/load 转发到本模块，
使既有 Batch 11 测试无需改动。本模块不 import accents/theme_switcher（避免循环）。
"""

from __future__ import annotations

import json
import os
from pathlib import Path

# 单一偏好文件名（替代 Batch 11 的 accent.json）。
PREFS_FILENAME = "theme_prefs.json"

# 旧 Batch 11 单 accent 文件名（迁移期向后兼容读取）。
_LEGACY_ACCENT_FILENAME = "accent.json"

# 与 _dashboard_layout_store 约定的子目录名一致（同一应用数据根）。
_SUBDIR = "embeddebug"

DEFAULT_THEME_ID = "serial_station_dark"
DEFAULT_ACCENT_ID = "cyan"


def _app_data_dir() -> Path:
    """解析应用数据目录（QStandardPaths.AppDataLocation），兜底 ~/.embeddebug。

    与 ``_dashboard_layout_store._app_data_dir`` / ``accent_store._app_data_dir``
    同一逻辑（未直接复用以保持模块自包含；三者落点相同，由 QStandardPaths 决定）。
    """

    from PyQt6.QtCore import QStandardPaths

    locations = QStandardPaths.standardLocations(QStandardPaths.StandardLocation.AppDataLocation)
    for loc in locations:
        if loc:
            return Path(loc)
    return Path.home() / ".embeddebug"


def prefs_path() -> Path:
    """统一主题偏好 JSON 文件路径。"""

    return _app_data_dir() / _SUBDIR / PREFS_FILENAME


def _legacy_accent_path() -> Path:
    """Batch 11 旧 accent.json 路径（迁移期读取用，不再写入）。"""

    return _app_data_dir() / _SUBDIR / _LEGACY_ACCENT_FILENAME


def _read_prefs_dict() -> dict:
    """读取偏好 dict；不存在/损坏返回空 dict。

    迁移兼容：若新文件不存在但旧 accent.json 存在，把旧 accent 并入返回 dict，
    使首次升级后 accent 不丢失。
    """

    p = prefs_path()
    if p.exists():
        try:
            raw = json.loads(p.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return {}
        return raw if isinstance(raw, dict) else {}
    # 迁移：旧 accent.json 存在则继承其 accent（theme 仍走默认深色）。
    legacy = _legacy_accent_path()
    if legacy.exists():
        try:
            lraw = json.loads(legacy.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return {}
        if isinstance(lraw, dict) and isinstance(lraw.get("accent"), str) and lraw["accent"]:
            return {"accent": lraw["accent"]}
    return {}


def load_theme_prefs() -> dict:
    """读取完整偏好 dict（含 theme + accent）；缺失回退默认。"""

    raw = _read_prefs_dict()
    theme = raw.get("theme")
    accent = raw.get("accent")
    return {
        "theme": theme if isinstance(theme, str) and theme else DEFAULT_THEME_ID,
        "accent": accent if isinstance(accent, str) and accent else DEFAULT_ACCENT_ID,
    }


def save_theme_prefs(theme: str | None = None, accent: str | None = None) -> bool:
    """原子写入偏好（合并写：保留未传入项的旧值）。

    Args:
        theme: 主题 id（``serial_station_dark`` / ``serial_station_light``）；
            ``None`` 保留旧值不变。
        accent: accent id；``None`` 保留旧值不变。

    一次写入包含两者，避免半写状态（如只写 theme 丢了 accent）。
    """

    current = load_theme_prefs()
    new_prefs = {
        "theme": theme if theme is not None else current["theme"],
        "accent": accent if accent is not None else current["accent"],
    }
    p = prefs_path()
    try:
        p.parent.mkdir(parents=True, exist_ok=True)
        tmp = p.with_suffix(p.suffix + ".tmp")
        tmp.write_text(
            json.dumps(new_prefs, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )
        os.replace(tmp, p)
    except OSError:
        return False
    return True


# ── 细粒度便捷访问（与 accent_store 旧 API 对齐） ──────────────────
def load_theme_id(default: str = DEFAULT_THEME_ID) -> str:
    """读取已保存的主题 id；不存在/损坏回退 ``default``。"""

    raw = _read_prefs_dict()
    theme = raw.get("theme")
    if isinstance(theme, str) and theme:
        return theme
    return default


def save_theme_id(theme_id: str) -> bool:
    """保存主题 id（保留当前 accent 不变）。"""

    return save_theme_prefs(theme=theme_id)


def load_accent_id(default: str = DEFAULT_ACCENT_ID) -> str:
    """读取已保存的 accent id；不存在/损坏回退 ``default``。"""

    raw = _read_prefs_dict()
    accent = raw.get("accent")
    if isinstance(accent, str) and accent:
        return accent
    return default


def save_accent_id(accent_id: str) -> bool:
    """保存 accent id（保留当前 theme 不变）。"""

    return save_theme_prefs(accent=accent_id)
