"""Badge 状态徽章控件 — kind 着色的小型圆角胶囊。

用于在面板/工具栏中显示紧凑的状态标签（如 ``已连接``/``离线``/``校验失败``），
背景色随 ``BadgeKind`` 变化（INFO/WARNING/ERROR/SUCCESS），文本居中。

设计要点：
- 自绘 QWidget（不依赖 QSS），paintEvent 画 kind 软底色圆角矩形 + 居中文本，
  色相对齐 palette 状态色 token，无散落硬编码（参见 05-ui-standard §颜色集中管理）。
- ``sizeHint`` 基线 24×24，宽度随文本水平 advance 增长（QFontMetrics 精确测量）。
- API 对齐 ``InfoBanner`` 的 ``set_text``/``text``/``set_kind``/``kind`` getter/setter
  习惯，但 ``BadgeKind`` 与 ``BannerKind`` 是独立类型，避免跨控件耦合。

约束：只依赖 PyQt6 + theme.palette，无业务逻辑、无动画、无定时器。
"""

from __future__ import annotations

import enum

from PyQt6.QtCore import QRectF, QSize, Qt
from PyQt6.QtGui import QColor, QFontMetrics, QPainter, QPaintEvent
from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.theme import palette as P


# ── 布局常量 ────────────────────────────────────────────────────────
# 基线高度 24px（规格要求），左右 padding 让文本不贴边，圆角取半高形成胶囊。
_BADGE_HEIGHT = 24
_BADGE_PADDING_H = 10
_BORDER_RADIUS = _BADGE_HEIGHT // 2


class BadgeKind(enum.Enum):
    """徽章类型枚举（语义对齐 BannerKind，但独立以避免跨控件耦合）。"""

    INFO = "info"
    WARNING = "warning"
    ERROR = "error"
    SUCCESS = "success"


class Badge(QWidget):
    """kind 着色的小型状态徽章。

    圆角胶囊形自绘控件，背景色取 palette ``*_SOFT`` token，文本色取对应状态
    accent token，宽度随文本自适应。常用于连接状态、协议校验结果、OTA 进度
    等场景的紧凑标签。

    用法::

        badge = Badge("已连接", BadgeKind.SUCCESS)
        layout.addWidget(badge)
        badge.set_kind(BadgeKind.ERROR)
        badge.set_text("校验失败")

    Args:
        text: 徽章显示的文本（数据字符串，不做翻译）。
        kind: 初始 kind，决定 (bg_color, text_color) 配色，默认 INFO。
        parent: 父控件。
    """

    # kind -> (bg_color, text_color)。bg 走 palette *_SOFT 软底 token，
    # text 走对应状态 accent token（保持 kind 色相可辨识）。
    KIND_COLORS: dict[BadgeKind, tuple[str, str]] = {
        BadgeKind.INFO: (P.INFO_SOFT, P.INFO),
        BadgeKind.WARNING: (P.WARNING_SOFT, P.WARNING),
        BadgeKind.ERROR: (P.ERROR_SOFT, P.ERROR),
        BadgeKind.SUCCESS: (P.SUCCESS_SOFT, P.SUCCESS),
    }

    def __init__(
        self,
        text: str = "",
        kind: BadgeKind = BadgeKind.INFO,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationBadge")

        self._text: str = text
        self._kind: BadgeKind = kind

        # 固定高度，宽度随 sizeHint 自适应。
        self.setFixedHeight(_BADGE_HEIGHT)

    # ── 公共 API ─────────────────────────────────────────────────────
    def set_text(self, text: str) -> None:
        """更新徽章文本并触发重绘与重新布局（宽度可能变化）。"""

        self._text = text
        self.updateGeometry()
        self.update()

    def text(self) -> str:
        """返回当前徽章文本。"""

        return self._text

    def set_kind(self, kind: BadgeKind) -> None:
        """更新 kind（决定 bg/text 配色）并触发重绘。"""

        self._kind = kind
        self.update()

    def kind(self) -> BadgeKind:
        """返回当前 kind。"""

        return self._kind

    # ── 尺寸 ─────────────────────────────────────────────────────────
    def sizeHint(self) -> QSize:
        """基于文本宽度返回建议尺寸：基线 24×24，宽度随文本增长。

        文本宽度由 ``QFontMetrics.horizontalAdvance`` 精确测量，加上左右
        ``_BADGE_PADDING_H`` padding；最小宽度取基线高度，保证空徽章也是正圆。
        """

        fm = QFontMetrics(self.font())
        text_w = fm.horizontalAdvance(self._text)
        width = _BADGE_PADDING_H * 2 + text_w
        return QSize(max(width, _BADGE_HEIGHT), _BADGE_HEIGHT)

    # ── 绘制 ─────────────────────────────────────────────────────────
    def paintEvent(self, event: QPaintEvent) -> None:
        """自绘：kind 软底色圆角矩形 + 居中文本。

        背景圆角取半高形成胶囊；文本水平+垂直居中。配色全部走 palette token，
        未识别 kind 回退到 INFO 配色（防御性，正常路径不会触发）。
        """

        bg_color, text_color = self.KIND_COLORS.get(
            self._kind, (P.ACCENT_SOFT, P.ACCENT)
        )

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        rect = QRectF(self.rect()).adjusted(0.5, 0.5, -0.5, -0.5)

        # 1. kind 软底色圆角矩形（胶囊形）。
        painter.setPen(Qt.PenStyle.NoPen)
        painter.setBrush(QColor(bg_color))
        painter.drawRoundedRect(rect, _BORDER_RADIUS, _BORDER_RADIUS)

        # 2. 居中文本（水平 + 垂直居中）。
        painter.setPen(QColor(text_color))
        painter.drawText(
            rect,
            int(Qt.AlignmentFlag.AlignCenter),
            self._text,
        )
