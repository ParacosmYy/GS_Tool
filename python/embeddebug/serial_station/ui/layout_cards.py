"""Serial Station 卡片化面板构建器。

卡片（``serialStationCard``）是 VOFA+ 风格分区布局的基本单位：
带圆角、阴影边框、标题行（图标 + 标题）和主体内容区。

设计要点：
- 卡片是 ``QFrame``，objectName 统一为 ``serialStationCard``，
  由 QSS 提供圆角、边框、背景、内边距。
- 标题行的标题/图标可选，便于灵活装配。
- ``wrap_layout`` 把现有 ``QLayout``（如 ``build_send_row`` 返回的行）
  装进卡片主体，保持 action 模块的 builder 契约不变。

约束：本模块只构建 UI 容器，不访问 controller/transport/protocol/service。
"""

from __future__ import annotations

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QLayout, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.theme import tokens as T

_CARD_OBJECT_NAME = "serialStationCard"
_CARD_TITLE_OBJECT_NAME = "serialStationCardTitle"
_CARD_HEADER_OBJECT_NAME = "serialStationCardHeader"
_CARD_BODY_OBJECT_NAME = "serialStationCardBody"


def build_card(
    parent: QWidget,
    title: str | None = None,
    icon_name: str | None = None,
) -> tuple[QFrame, QVBoxLayout]:
    """构建一张卡片，返回 (卡片本体, 主体内容 layout 供调用方填充)。"""

    card = QFrame(parent)
    card.setObjectName(_CARD_OBJECT_NAME)
    card_layout = QVBoxLayout(card)
    card_layout.setContentsMargins(T.SPACING_INT_LG, T.SPACING_INT_LG, T.SPACING_INT_LG, T.SPACING_INT_LG)
    card_layout.setSpacing(T.SPACING_INT_MD)

    if title is not None:
        header = _build_header(card, title, icon_name)
        card_layout.addWidget(header)

    body = QVBoxLayout()
    body.setSpacing(T.SPACING_INT_MD)
    body.setObjectName(_CARD_BODY_OBJECT_NAME)
    card_layout.addLayout(body, 1)
    return card, body


def wrap_layout(parent: QWidget, layout: QLayout, title: str | None = None,
                icon_name: str | None = None) -> QFrame:
    """把一个现成 QLayout 装进卡片并返回卡片本体。

    用于把 ``build_connection_toolbar``/``build_send_row`` 返回的 layout
    包进卡片，不改动原 builder 签名。
    """

    card, body = build_card(parent, title=title, icon_name=icon_name)
    body.addLayout(layout)
    return card


def card_body(card: QWidget) -> QVBoxLayout:
    """按 objectName 取卡片的主体 layout，避免依赖 header 是否存在。"""

    layout = card.layout()
    if layout is None:
        raise ValueError("card has no layout")
    for index in range(layout.count()):
        item = layout.itemAt(index)
        if item is not None and item.layout() is not None:
            inner = item.layout()
            if inner.objectName() == _CARD_BODY_OBJECT_NAME:
                return inner
    raise ValueError(f"card {card.objectName()} has no body layout")


def _build_header(card: QFrame, title: str, icon_name: str | None) -> QWidget:
    header = QWidget(card)
    header.setObjectName(_CARD_HEADER_OBJECT_NAME)
    row = QHBoxLayout(header)
    row.setContentsMargins(0, 0, 0, 0)
    row.setSpacing(T.SPACING_INT_SM)

    if icon_name is not None:
        icon_label = QLabel(header)
        icon_label.setObjectName("serialStationCardIcon")
        icon_label.setPixmap(button_icon(icon_name).pixmap(16, 16))
        row.addWidget(icon_label)

    title_label = QLabel(title, header)
    title_label.setObjectName(_CARD_TITLE_OBJECT_NAME)
    title_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
    row.addWidget(title_label)
    row.addStretch(1)
    return header
