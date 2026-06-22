"""Serial Station 图标管理器。

用 ``QSvgRenderer`` 加载 ``resources/icons/lucide/*.svg``，按 palette 着色后
转为 ``QIcon``，给按钮加图标。着色通过把 SVG 的 ``currentColor``/``stroke``
替换为目标颜色实现，结果缓存到 ``(name, color)`` 键，避免重复渲染。

资源定位复用 ``ThemeManager.resolve_resource_path``，兼容 dev 与 PyInstaller onedir。

约束（docs/constraints/08-icon-standard.md）：所有图标走 IconManager，
不散落硬编码 SVG 路径或颜色。本模块只依赖 PyQt6 + 标准库 + theme.palette。
"""

from __future__ import annotations

from pathlib import Path

from PyQt6.QtCore import QByteArray, QSize, Qt
from PyQt6.QtGui import QIcon, QPainter, QPixmap
from PyQt6.QtSvg import QSvgRenderer

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme.manager import ThemeManager

_ICON_REL = Path("resources") / "icons" / "lucide"
_DEFAULT_PIXELS = 18
# 默认着色：次文本色，适配深色底按钮。
_DEFAULT_COLOR = P.TEXT_SECONDARY


class IconManager:
    """单例图标管理器，负责 SVG 加载、着色与缓存。"""

    _instance: IconManager | None = None

    def __new__(cls) -> IconManager:
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._cache = {}  # type: ignore[attr-defined]
        return cls._instance

    def __init__(self) -> None:
        if not hasattr(self, "_cache"):
            # 缓存键必须含 pixels 维度，否则不同尺寸会互相命中返回错误尺寸图标。
            self._cache: dict[tuple[str, str, int], QIcon] = {}

    def icon(
        self,
        name: str,
        color: str = _DEFAULT_COLOR,
        pixels: int = _DEFAULT_PIXELS,
    ) -> QIcon:
        """返回指定名称、颜色、像素尺寸的 QIcon。

        缺失图标时返回空 QIcon（不抛异常），保证 UI 不因缺图崩溃。
        """

        key = (name, color, pixels)
        cached = self._cache.get(key)
        if cached is not None:
            return cached
        icon = self._render_icon(name, color, pixels)
        self._cache[key] = icon
        return icon

    def reset(self) -> None:
        """清空缓存（仅测试用）。"""

        self._cache.clear()

    # ── 内部 ───────────────────────────────────────────────────────
    def _render_icon(self, name: str, color: str, pixels: int) -> QIcon:
        svg_text = self._read_svg(name)
        if svg_text is None:
            return QIcon()
        tinted = self._tint(svg_text, color)
        renderer = QSvgRenderer(QByteArray(tinted.encode("utf-8")))
        if not renderer.isValid():
            return QIcon()
        pixmap = QPixmap(QSize(pixels, pixels))
        pixmap.fill(Qt.GlobalColor.transparent)
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform, True)
        renderer.render(painter)
        painter.end()
        return QIcon(pixmap)

    def _read_svg(self, name: str) -> str | None:
        resource = ThemeManager.resolve_resource_path(_ICON_REL / f"{name}.svg")
        if not resource.is_file():
            return None
        try:
            return resource.read_text(encoding="utf-8")
        except OSError:
            return None

    @staticmethod
    def _tint(svg_text: str, color: str) -> str:
        """把 SVG 的 currentColor 与裸 stroke 颜色替换为目标颜色。

        修正（Batch 3）：旧版 ``count=1`` 只替换第一个 stroke 属性，多 path 图标
        （如 lucide 的多笔画图标）后续 path 仍是原色，着色不全。改为替换全部
        stroke 属性，保证多 path 图标整体着色。
        """

        tinted = svg_text.replace("currentColor", color)
        # 兜底：部分 lucide 变体用裸 stroke="#xxx"，统一覆盖为 stroke="color"。
        # 替换全部 stroke 属性（多 path 图标需整体着色）。
        import re

        tinted = re.sub(r'stroke="[^"]*"', f'stroke="{color}"', tinted)
        return tinted


def button_icon(name: str, color: str = _DEFAULT_COLOR) -> QIcon:
    """便捷函数：返回按钮常用着色图标。"""

    return IconManager().icon(name, color=color)
