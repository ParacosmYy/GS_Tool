"""Serial Station 主题管理器。

职责：
1. 程序化生成默认深色工业风 QSS（``qss_builder``），作为打包友好的主路径。
2. 可选从 ``resources/themes/*.qss`` 加载外部主题（开发期手改 QSS 的编辑路径）。
3. 提供运行时切换与当前主题查询，供后续 B2 的主题切换入口使用。

资源定位策略（兼容 dev 与 PyInstaller onedir）：
- 优先用 ``qss_builder.build_qss()`` 程序化生成，零文件依赖、打包无忧。
- 外部 ``.qss`` 通过 ``resolve_resource_path`` 定位：开发期相对仓库根，
  打包期相对 ``sys._MEIPASS``（PyInstaller 注入）或可执行文件目录。

约束：本模块只依赖 PyQt6 + 标准库 + 本包内 palette/tokens/qss_builder，
不 import controller/core/protocols/services。
"""

from __future__ import annotations

import sys
from pathlib import Path

from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.theme.qss_builder import build_qss

DEFAULT_THEME = "serial_station_dark"
BUILTIN_THEMES: tuple[str, ...] = (DEFAULT_THEME,)
_THEME_RESOURCE_REL = Path("resources") / "themes"


class ThemeManager:
    """单例主题管理器，负责 QSS 生成、加载与应用。"""

    _instance: "ThemeManager | None" = None

    def __new__(cls) -> "ThemeManager":
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._current_theme = None  # type: ignore[attr-defined]
        return cls._instance

    def __init__(self) -> None:
        # __new__ 已初始化 _current_theme；此处仅保证属性存在。
        if not hasattr(self, "_current_theme"):
            self._current_theme: str | None = None

    # ── 公共 API ───────────────────────────────────────────────────
    @property
    def current_theme(self) -> str | None:
        """返回当前已应用的主题名，未应用时为 None。"""

        return self._current_theme

    def apply_theme(self, app: QApplication, name: str = DEFAULT_THEME) -> str:
        """应用指定主题到 QApplication，返回生效的 QSS 文本。

        - 程序化主题（``serial_station_dark``）走 ``build_qss``，零文件依赖。
        - 若 ``resources/themes/<name>.qss`` 存在，则优先加载外部文件，
          便于设计期手改 QSS 调试；加载失败时回退到程序化生成并记录。
        """

        qss = self.load_qss(name)
        app.setStyleSheet(qss)
        self._current_theme = name
        return qss

    def load_qss(self, name: str = DEFAULT_THEME) -> str:
        """加载指定主题的 QSS 文本。

        外部文件优先（设计期编辑路径），缺失或读取失败时回退到程序化生成。
        程序化生成是打包期的唯一可靠路径。
        """

        external = self._read_external_qss(name)
        if external is not None:
            return external
        return build_qss()

    def reset(self) -> None:
        """重置单例状态（仅测试用）。"""

        self._current_theme = None

    # ── 资源定位 ───────────────────────────────────────────────────
    @staticmethod
    def resolve_resource_path(relative: Path | str) -> Path:
        """解析仓库内资源路径，兼容 dev 与 PyInstaller onedir。

        查找顺序：
        1. PyInstaller ``sys._MEIPASS``（打包态，资源被收集到该临时目录）。
        2. 可执行文件同级（``sys.executable`` 目录，onedir 散装）。
        3. 仓库根（开发态：从本文件向上回溯到含 ``resources/`` 的目录）。
        """

        candidates: list[Path] = []
        meipass = getattr(sys, "_MEIPASS", None)
        if meipass:
            candidates.append(Path(meipass))
        candidates.append(Path(sys.executable).resolve().parent)
        # 开发态：python/embeddebug/serial_station/ui/theme/manager.py
        # 向上 6 级回到仓库根。
        candidates.append(Path(__file__).resolve().parents[5])

        rel = Path(relative)
        for base in candidates:
            candidate = base / rel
            if candidate.exists():
                return candidate
        # 兜底返回最后一个候选（调用方自行判断 exists）。
        return candidates[-1] / rel

    # ── 内部 ───────────────────────────────────────────────────────
    def _read_external_qss(self, name: str) -> str | None:
        """尝试读取外部 ``resources/themes/<name>.qss``，失败返回 None。"""

        resource = self.resolve_resource_path(_THEME_RESOURCE_REL / f"{name}.qss")
        if not resource.is_file():
            return None
        try:
            return resource.read_text(encoding="utf-8")
        except OSError:
            return None


def apply_theme(app: QApplication, name: str = DEFAULT_THEME) -> str:
    """便捷函数：应用默认主题到给定 QApplication。"""

    return ThemeManager().apply_theme(app, name)


def current_theme_name() -> str | None:
    """便捷函数：返回当前已应用的主题名。"""

    return ThemeManager().current_theme
