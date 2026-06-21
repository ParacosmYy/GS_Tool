"""Serial Station 三区分栏主布局（VOFA+ 风格）。

把原单列堆叠重排为三区 QSplitter：
- 左区：连接配置卡（toolbar + 发送/注入命令行）。
- 中区：波形预览 + 日志视图（主信息区，占最大宽度）。
- 右区：日志操作（过滤/搜索/导出/回放）+ Profile（保存/加载）。

设计要点：
- ``sections.build_main_layout`` 先用字面量调用
  ``build_connection_toolbar(owner, controller, root)`` 与
  ``command_section.build_send_row(owner, root)``（满足架构测试），
  再把返回的 layout 交给本模块的 ``assemble_three_zone`` 装配。
- 卡片用 ``layout_cards.wrap_layout`` 包装现有 layout builder，不改 action 契约。
- 所有控件仍以 ``owner``/``root`` 为 parent，``findChild`` 可达。

约束：本模块只装配布局，不访问 controller/transport/protocol/service 的内部状态。
"""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QFrame, QLabel, QLayout, QSplitter, QVBoxLayout, QWidget

from embeddebug.serial_station.ui.layout_cards import build_card, card_body, wrap_layout
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


class LayoutMainHost(Protocol):
    def tr(self, source_text: str) -> str: ...


def assemble_three_zone(
    owner: LayoutMainHost,
    root: QWidget,
    toolbar_layout: QLayout,
    send_row_layout: QLayout,
    inject_row_layout: QLayout,
    log_row_layout: QLayout,
    profile_row_layout: QLayout,
    footer_layout: QLayout,
) -> QSplitter:
    """装配三区分栏布局，返回 QSplitter（供 sections 加入根 layout）。

    调用方（sections）先用字面量 builder 构建各 layout，再传入本函数装配。
    """

    splitter = QSplitter(Qt.Orientation.Horizontal, root)
    splitter.setObjectName("serialStationMainSplitter")
    splitter.setHandleWidth(4)  # Batch 46: 从 2px 加宽到 4px（UI 审计 §2.6，改善拖拽可用性）
    splitter.setChildrenCollapsible(False)

    # 框架：左=连接配置（精简侧栏）、中=主工作区（波形+日志+命令行）、右=日志工具+Profile。
    # 命令行(send/inject)移到中区底部，紧贴日志：发送→看日志是连续视线流，且左栏不再溢出。
    left_zone = _build_left_zone(owner, splitter, toolbar_layout)
    log_card, waveform_card = _build_center_zone(
        owner, splitter, send_row_layout, inject_row_layout
    )
    right_zone = _build_right_zone(owner, splitter, log_row_layout, profile_row_layout, footer_layout)

    # 三区最小宽度：防止窄栏把横向 toolbar 控件挤压到字符截断。
    left_zone.setMinimumWidth(300)
    splitter.widget(1).setMinimumWidth(380)
    right_zone.setMinimumWidth(260)

    # 显式初始分配：左 300 / 中 620 / 右 260（中区主工作区占主导）。
    splitter.setSizes([300, 620, 260])

    # stretch：中区主导（resize 时多余空间优先给中区）。
    splitter.setStretchFactor(0, 1)
    splitter.setStretchFactor(1, 3)
    splitter.setStretchFactor(2, 1)

    # 把中区卡片挂到 owner，供 sections 注入 log_view/log_stats_label。
    owner._center_log_card = log_card
    owner._center_waveform_card = waveform_card
    return splitter


def _build_left_zone(
    owner: LayoutMainHost,
    splitter: QSplitter,
    toolbar_layout: QLayout,
) -> QWidget:
    """左区：仅连接配置卡（命令行已移到中区，避免左栏纵向溢出）。"""

    zone = QWidget(splitter)
    zone.setObjectName("serialStationLeftZone")
    zone_layout = QVBoxLayout(zone)
    zone_layout.setContentsMargins(0, 0, 0, 0)
    zone_layout.setSpacing(T.SPACING_INT_LG)

    # connection_toolbar 默认返回横向 QHBoxLayout（17 控件一行），
    # 在窄左栏会全部截断；这里把它的控件提取出来纵向重排，并按
    # 语义分组（端口 / 串口参数 / 连接动作 / 端点），保证可读。
    connection_card = _build_connection_card_vertical(owner, zone, toolbar_layout)
    zone_layout.addWidget(connection_card)

    zone_layout.addStretch(1)
    return zone


def _build_connection_card_vertical(
    owner: LayoutMainHost, parent: QWidget, toolbar_layout: QLayout
) -> QFrame:
    """委托 connection_sidebar 把横向 toolbar 纵向重排进连接卡片。"""

    from embeddebug.serial_station.ui.connection_sidebar import build_connection_card

    return build_connection_card(owner, parent, toolbar_layout)


def _build_center_zone(
    owner: LayoutMainHost,
    splitter: QSplitter,
    send_row_layout: QLayout,
    inject_row_layout: QLayout,
) -> tuple[QWidget, QWidget]:
    """中区：主工作区 = 波形(上) + 日志(中,主体) + 命令卡(下)。

    返回 (log_card, waveform_card)，波形卡内已嵌入 waveform_preview。
    日志是主要信息区（用户看收发数据），stretch 3；波形辅助观察，stretch 2；
    命令行(send/inject)贴在日志下方，发送→看日志是连续视线流。
    """

    zone = QWidget(splitter)
    zone.setObjectName("serialStationCenterZone")
    zone_layout = QVBoxLayout(zone)
    zone_layout.setContentsMargins(0, 0, 0, 0)
    zone_layout.setSpacing(T.SPACING_INT_LG)

    # 波形卡（上）：辅助观察，stretch 2，设最小高度防压扁。
    waveform_card, wave_body = build_card(
        zone, title=owner.tr("Waveform"), icon_name="activity"
    )
    owner._waveform_preview = SerialWaveformPreview(waveform_card)
    wave_body.addWidget(owner._waveform_preview, 1)
    waveform_card.setMinimumHeight(160)
    zone_layout.addWidget(waveform_card, 2)

    # 日志卡（中,主体）：主要信息区，stretch 3，设最小高度保证可读。
    log_card, log_body = build_card(zone, title=owner.tr("Log"), icon_name="terminal")
    log_card.setMinimumHeight(200)
    zone_layout.addWidget(log_card, 3)

    # 命令卡（下）：send + inject 紧贴日志底部。
    command_card = wrap_layout(zone, send_row_layout)
    command_body = card_body(command_card)
    command_body.addLayout(inject_row_layout)
    zone_layout.addWidget(command_card)
    return log_card, waveform_card


def _build_right_zone(
    owner: LayoutMainHost,
    splitter: QSplitter,
    log_row_layout: QLayout,
    profile_row_layout: QLayout,
    footer_layout: QLayout,
) -> QWidget:
    """右区：日志操作卡 + Profile 卡 + 底部 Clear。"""

    zone = QWidget(splitter)
    zone.setObjectName("serialStationRightZone")
    zone_layout = QVBoxLayout(zone)
    zone_layout.setContentsMargins(0, 0, 0, 0)
    zone_layout.setSpacing(T.SPACING_INT_LG)

    log_ops_card, log_ops_body = build_card(
        zone, title=owner.tr("Log Tools"), icon_name="filter"
    )
    log_ops_body.addLayout(log_row_layout)
    zone_layout.addWidget(log_ops_card)

    profile_card, profile_body = build_card(
        zone, title=owner.tr("Profile"), icon_name="save"
    )
    profile_body.addLayout(profile_row_layout)
    zone_layout.addWidget(profile_card)

    zone_layout.addLayout(footer_layout)
    return zone
