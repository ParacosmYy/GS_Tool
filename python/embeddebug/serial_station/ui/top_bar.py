"""Serial Station TopBar — 品牌区 + 状态/Profile 药丸。

对齐 EK-OmniProbe ``TopBar`` shell：左侧品牌 chip（图标 + 名称 + 副标题），
右侧把既有 ``_status_label`` / ``_profile_label`` 摆成状态药丸。

设计要点：
- TopBar 是 ``QFrame#serialStationTopBar``，装在 ``serialStationPyRoot`` 顶部、
  splitter 之上（保持 ``serialStationPyRoot`` 仍是 ``setCentralWidget`` 目标）。
- 状态/Profile 标签**复用** owner 已创建的 ``_status_label`` / ``_profile_label``，
  不新建控件、不改 objectName，保证 ``findChild`` 与 action 模块契约不变。
- 品牌 chip 图标走 ``IconManager``，缺图不抛异常。

约束：本模块只构建 UI 容器，不访问 controller/transport/protocol/service。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QFrame, QHBoxLayout, QLabel, QLayout, QWidget

from embeddebug.serial_station.ui.icons import button_icon
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme import palette as P


class TopBarHost(Protocol):
    """TopBar 宿主需持有的标签引用（SerialStationMainWindow 在 build 前已创建）。"""

    def tr(self, source_text: str) -> str: ...


def build_top_bar(owner: TopBarHost, root: QWidget) -> QFrame:
    """构建 TopBar，返回 ``QFrame#serialStationTopBar``。

    状态/Profile 标签由 owner 预先创建（``_status_label`` / ``_profile_label``），
    本函数把它们 reparent 到 TopBar 右侧并加间距，不改其 objectName 或文本逻辑。
    """

    bar = QFrame(root)
    bar.setObjectName("serialStationTopBar")

    row = QHBoxLayout(bar)
    row.setContentsMargins(T.SPACING_INT_XL, T.SPACING_INT_MD, T.SPACING_INT_XL, T.SPACING_INT_MD)
    row.setSpacing(T.SPACING_INT_LG)

    brand = _build_brand_chip(owner, bar)
    row.addWidget(brand)
    row.addStretch(1)

    # 状态/Profile 药丸：复用 owner 既有标签，保持 findChild 与 action 契约。
    _reparent_status_pills(owner, bar, row)
    return bar


def _build_brand_chip(owner: TopBarHost, parent: QWidget) -> QFrame:
    """品牌 chip：图标 + 名称 + 副标题（对齐 EK-OmniProbe logo chip）。"""

    chip = QFrame(parent)
    chip.setObjectName("serialStationBrandChip")
    chip_layout = QHBoxLayout(chip)
    chip_layout.setContentsMargins(T.SPACING_INT_MD, T.SPACING_INT_SM, T.SPACING_INT_LG, T.SPACING_INT_SM)
    chip_layout.setSpacing(T.SPACING_INT_MD)

    icon_label = QLabel(chip)
    icon_label.setObjectName("serialStationBrandIcon")
    icon = button_icon("cpu", color=P.ACCENT)
    if not icon.isNull():
        icon_label.setPixmap(icon.pixmap(18, 18))
    chip_layout.addWidget(icon_label)

    text_box = _build_brand_text(owner, chip)
    chip_layout.addWidget(text_box)
    return chip


def _build_brand_text(owner: TopBarHost, parent: QWidget) -> QWidget:
    """品牌文字区：名称（主）+ 副标题（次）。"""

    box = QWidget(parent)
    box_layout = QHBoxLayout(box)
    box_layout.setContentsMargins(0, 0, 0, 0)
    box_layout.setSpacing(T.SPACING_INT_MD)

    name = QLabel(owner.tr("EmbedDebug"), box)
    name.setObjectName("serialStationBrandName")
    name.setAlignment(Qt.AlignmentFlag.AlignLeft)

    tagline = QLabel(owner.tr("Serial Station"), box)
    tagline.setObjectName("serialStationBrandTagline")
    tagline.setAlignment(Qt.AlignmentFlag.AlignLeft)

    box_layout.addWidget(name)
    box_layout.addWidget(tagline)
    return box


def _reparent_status_pills(owner: TopBarHost, bar: QFrame, row: QLayout) -> None:
    """把 owner 的状态/Profile 标签 reparent 到 TopBar 右侧。

    标签由 ``sections.build_main_layout`` 预先创建；此处仅改 parent 与间距，
    不改 objectName/文本/对齐逻辑，保证 action 模块与 findChild 不受影响。
    若标签尚未创建（调用顺序异常），静默跳过。
    """

    # Batch 48: Ctrl+P 命令面板快捷键提示（对标 VS Code/MobaXterm 可发现性）。
    try:
        from embeddebug.serial_station.ui.controls.key_hint import KeyboardShortcut
        row.addWidget(KeyboardShortcut("Ctrl+P", bar))
    except Exception:
        pass

    profile_label = getattr(owner, "_profile_label", None)
    status_label = getattr(owner, "_status_label", None)

    if profile_label is not None:
        profile_label.setParent(bar)
        row.addWidget(profile_label)
    if status_label is not None:
        status_label.setParent(bar)
        row.addWidget(status_label)
