"""用户偏好持久化服务 — 主题/accent/字体/数据目录/默认波特率/动画开关。

落点：services/ 层（L2），不依赖 UI/controller。使用 ``QSettings`` 作为主存储
（Windows 注册表 / macOS plist / Linux ini），无 JSON 文件依赖，避免与
``theme_store`` 的 ``theme_prefs.json`` 冲突。

设计要点：
- 真相源单一：本服务只负责"广义用户偏好"。theme/accent 已由 ``theme_store``
  持久化（既有测试与 ``theme_switcher`` 路径依赖），本服务**镜像**这两个字段
  用于统一读取入口；写入时**双写**（SettingsManager + theme_store），保证
  两条路径读到一致值。
- ``QSettings`` 失败时（注册表锁、权限问题）静默回退默认值，不阻塞 UI 启动。
- 单例（``instance()``）避免多实例并发写竞争。

约束：只依赖 PyQt6.QtCore.QSettings + 标准库 + theme_store（同包 services 不算
跨层），不 import UI/controller/transport/protocol。
"""

from __future__ import annotations

import logging
from dataclasses import asdict, dataclass
from typing import Any

from PyQt6.QtCore import QSettings

_log = logging.getLogger(__name__)

# ── 默认值（与 theme_store / accent_store 当前默认对齐） ────────────
DEFAULT_THEME = "serial_station_dark"
DEFAULT_ACCENT = "cyan"
DEFAULT_FONT_POINT = 13
DEFAULT_DATA_DIR = ""  # 空串 = 使用平台默认（QStandardPaths）
DEFAULT_BAUDRATE = 115200
DEFAULT_ANIMATION_ENABLED = True

# QSettings 组织/应用名（与 main.create_application 对齐）。
_ORG_NAME = "EmbedDebug"
_APP_NAME = "EmbedDebug"


@dataclass
class UserSettings:
    """用户偏好数据模型（不可变值对象语义，写由 SettingsManager 负责）。"""

    theme: str = DEFAULT_THEME
    accent: str = DEFAULT_ACCENT
    font_point: int = DEFAULT_FONT_POINT
    data_dir: str = DEFAULT_DATA_DIR
    default_baudrate: int = DEFAULT_BAUDRATE
    animation_enabled: bool = DEFAULT_ANIMATION_ENABLED


class SettingsManager:
    """用户偏好管理器（单例）。

    ``QSettings`` 为主存储，``theme_store`` 为 theme/accent 镜像写入路径（向后
    兼容既有 ``theme_switcher.apply_*`` 的读路径）。
    """

    _instance: SettingsManager | None = None

    def __init__(self) -> None:
        self._settings = QSettings(_ORG_NAME, _APP_NAME)
        self._cache: UserSettings = self._load()

    @classmethod
    def instance(cls) -> SettingsManager:
        """返回单例（首次调用时构造）。"""

        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    @classmethod
    def reset_singleton(cls) -> None:
        """清掉单例缓存（测试用；产品代码不应调用）。"""

        cls._instance = None

    def get(self) -> UserSettings:
        """返回当前缓存的偏好快照（不可变值对象）。"""

        return self._cache

    def update(self, **kwargs: Any) -> UserSettings:
        """更新若干字段，立即持久化（QSettings + theme_store 双写）。

        Args:
            **kwargs: ``UserSettings`` 字段名 → 新值。

        Raises:
            AttributeError: 传入字段名不在 ``UserSettings`` 中。
        """

        for key, value in kwargs.items():
            if not hasattr(self._cache, key):
                raise AttributeError(f"Unknown setting: {key}")
            setattr(self._cache, key, value)
            try:
                self._settings.setValue(key, value)
            except Exception:  # noqa: BLE001  QSettings 失败不应阻断内存更新
                _log.warning("QSettings setValue failed for %s", key, exc_info=True)
        # theme/accent 双写到 theme_store，使 theme_switcher 读路径一致。
        self._mirror_theme_store()
        try:
            self._settings.sync()
        except Exception:  # noqa: BLE001  sync 失败不阻塞
            _log.warning("QSettings sync failed", exc_info=True)
        return self._cache

    def reset(self) -> UserSettings:
        """恢复全部默认值（清掉 QSettings 中所有 key + theme_store 同步）。"""

        self._cache = UserSettings()
        for key in asdict(self._cache).keys():
            try:
                self._settings.remove(key)
            except Exception:  # noqa: BLE001
                _log.warning("QSettings remove failed for %s", key, exc_info=True)
        try:
            self._settings.sync()
        except Exception:  # noqa: BLE001
            _log.warning("QSettings sync failed", exc_info=True)
        # theme_store 也回到默认（避免下次启动 theme_switcher 读到旧主题）。
        self._mirror_theme_store()
        return self._cache

    def reload(self) -> UserSettings:
        """从 QSettings 重新加载缓存（测试与外部修改后同步用）。"""

        self._cache = self._load()
        return self._cache

    # ── 内部 ────────────────────────────────────────────────────────
    def _load(self) -> UserSettings:
        """从 QSettings 读取；任意字段读取异常 → 该字段回退默认。"""

        try:
            theme = self._read_str("theme", DEFAULT_THEME)
            accent = self._read_str("accent", DEFAULT_ACCENT)
            font_point = self._read_int("font_point", DEFAULT_FONT_POINT)
            data_dir = self._read_str("data_dir", DEFAULT_DATA_DIR)
            baudrate = self._read_int("default_baudrate", DEFAULT_BAUDRATE)
            animation = self._read_bool("animation_enabled", DEFAULT_ANIMATION_ENABLED)
            return UserSettings(
                theme=theme,
                accent=accent,
                font_point=font_point,
                data_dir=data_dir,
                default_baudrate=baudrate,
                animation_enabled=animation,
            )
        except Exception:  # noqa: BLE001  顶层兜底：构造失败也不阻塞启动
            _log.warning("settings load failed, using defaults", exc_info=True)
            return UserSettings()

    def _read_str(self, key: str, default: str) -> str:
        val = self._settings.value(key, default, type=str)
        return val if isinstance(val, str) and val else default

    def _read_int(self, key: str, default: int) -> int:
        val = self._settings.value(key, default, type=int)
        try:
            return int(val)
        except (TypeError, ValueError):
            return default

    def _read_bool(self, key: str, default: bool) -> bool:
        val = self._settings.value(key, default, type=bool)
        if isinstance(val, bool):
            return val
        if isinstance(val, str):
            return val.lower() in ("true", "1", "yes")
        return default

    def _mirror_theme_store(self) -> None:
        """把当前 theme/accent 镜像写入 theme_store（双写保一致）。

        失败静默：theme_store 持久化是 best-effort，不影响 SettingsManager 的
        内存缓存与 QSettings 存储。
        """

        try:
            from embeddebug.serial_station.ui.theme import theme_store

            theme_store.save_theme_prefs(
                theme=self._cache.theme,
                accent=self._cache.accent,
            )
        except Exception:  # noqa: BLE001  镜像失败不阻塞主写路径
            _log.debug("theme_store mirror failed", exc_info=True)
