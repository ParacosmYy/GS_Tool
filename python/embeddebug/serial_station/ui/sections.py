"""UI section builders for the Serial Station main window."""

from __future__ import annotations

from typing import Protocol

from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QComboBox, QHBoxLayout, QLabel, QLineEdit, QPlainTextEdit, QPushButton, QVBoxLayout, QWidget

from embeddebug.serial_station.controllers import SerialWorkbenchController
from embeddebug.serial_station.ui import command_section
from embeddebug.serial_station.ui import layout_cards
from embeddebug.serial_station.ui import layout_main
from embeddebug.serial_station.ui.connection_toolbar import build_connection_toolbar
from embeddebug.serial_station.ui.log_filter_options import (
    default_log_filter_text,
    log_filter_options,
)
from embeddebug.serial_station.ui.shortcuts import install_shortcuts
from embeddebug.serial_station.ui.top_bar import build_top_bar


class SerialStationSectionsHost(Protocol):
    def tr(self, source_text: str) -> str: ...
    def _set_protocol(self, name: str) -> None: ...
    def _refresh_serial_ports(self) -> None: ...
    def _has_serial_ports(self) -> bool: ...
    def _connect_fake(self) -> None: ...
    def _connect_serial(self) -> None: ...
    def _connect_tcp(self) -> None: ...
    def _connect_udp(self) -> None: ...
    def _disconnect(self) -> None: ...
    def _send_text(self) -> None: ...
    def _select_command_history(self, text: str) -> None: ...
    def _inject_received(self) -> None: ...
    def _render_log_entries(self) -> None: ...
    def _update_log_stats(self) -> None: ...
    def _export_log(self) -> None: ...
    def _replay_log(self) -> None: ...
    def _save_profile(self) -> None: ...
    def _load_profile(self) -> None: ...
    def _clear_log(self) -> None: ...


def build_main_layout(owner: SerialStationSectionsHost, controller: SerialWorkbenchController) -> QWidget:
    root = QWidget(owner)
    root.setObjectName("serialStationPyRoot")

    layout = QVBoxLayout(root)
    layout.setContentsMargins(12, 12, 12, 12)
    layout.setSpacing(10)

    owner._status_label = QLabel(owner.tr("Disconnected"), root)
    owner._status_label.setObjectName("serialStationStatusLabel")
    owner._status_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
    owner._profile_label = QLabel(owner.tr("Profile: unsaved"), root)
    owner._profile_label.setObjectName("serialStationProfileLabel")
    owner._profile_label.setAlignment(Qt.AlignmentFlag.AlignLeft)

    # TopBar 复用上面的状态/Profile 标签（reparent 到右侧药丸区），保持 findChild 契约。
    top_bar = build_top_bar(owner, root)
    layout.addWidget(top_bar)

    # 字面量委托：架构测试要求这两行原样存在。
    toolbar = build_connection_toolbar(owner, controller, root)
    send_row = command_section.build_send_row(owner, root)

    # log_stats_label 在 build_log_row 内创建并挂到布局（Batch 4 改为纵向布局）。
    inject_row = build_inject_row(owner, root)
    log_row = build_log_row(owner, root)
    profile_row = build_profile_row(owner, root)
    footer = build_footer(owner, root)

    splitter = layout_main.assemble_three_zone(
        owner, root, toolbar, send_row, inject_row, log_row, profile_row, footer
    )
    layout.addWidget(splitter, 1)

    # 中区日志卡由 layout_main 创建；log_view 注入到其主体。
    _populate_center_log_card(owner)

    install_shortcuts(owner)
    return root


def _populate_center_log_card(owner: SerialStationSectionsHost) -> None:
    """把 log_view 注入中区日志卡主体（log_stats_label 随 log_row 在右区）。"""

    log_card = getattr(owner, "_center_log_card", None)
    if log_card is None:
        return
    body = layout_cards.card_body(log_card)

    owner._log_view = QPlainTextEdit(log_card)
    owner._log_view.setObjectName("serialStationLogView")
    owner._log_view.setPlaceholderText(owner.tr("No serial log entries"))
    owner._log_view.setReadOnly(True)
    body.addWidget(owner._log_view, 1)
    owner._update_log_stats()


def build_inject_row(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(8)
    owner._inject_edit = QLineEdit(root)
    owner._inject_edit.setObjectName("serialStationInjectEdit")
    owner._inject_edit.setPlaceholderText(owner.tr("Fake received text"))
    owner._inject_edit.returnPressed.connect(owner._inject_received)
    owner._inject_button = QPushButton(owner.tr("Inject RX"), root)
    owner._inject_button.setObjectName("serialStationInjectButton")
    owner._inject_button.setToolTip(owner.tr("Inject fake received bytes"))
    owner._inject_button.clicked.connect(owner._inject_received)
    row.addWidget(owner._inject_edit, 1)
    row.addWidget(owner._inject_button)
    return row


def build_log_row(owner: SerialStationSectionsHost, root: QWidget) -> QVBoxLayout:
    """构建日志工具区（Batch 4 改为纵向分组布局）。

    旧版把 6 控件塞进单个 QHBoxLayout，在右区 260px 窄列必然横向截断。
    Batch 4 改为纵向 QVBoxLayout 分 3 行：
    - 第 1 行：filter combo + search edit（filter 弹性较小，search 占主导）
    - 第 2 行：session log path（占满宽度）
    - 第 3 行：stats label + Save/Replay 按钮（按钮弹性收缩）

    返回 QVBoxLayout（与调用方 ``addLayout`` 兼容任意 QLayout）。
    """

    col = QVBoxLayout()
    col.setSpacing(6)
    col.setContentsMargins(0, 0, 0, 0)

    # 第 1 行：过滤 + 搜索。
    row1 = QHBoxLayout()
    row1.setSpacing(6)
    owner._log_filter_combo = QComboBox(root)
    owner._log_filter_combo.setObjectName("serialStationLogFilterCombo")
    owner._log_filter_combo.setToolTip(owner.tr("Filter visible log entries"))
    owner._log_filter_combo.addItems([owner.tr(text) for text in log_filter_options()])
    owner._log_filter_combo.setCurrentText(owner.tr(default_log_filter_text()))
    owner._log_filter_combo.currentTextChanged.connect(owner._render_log_entries)
    owner._log_search_edit = QLineEdit(root)
    owner._log_search_edit.setObjectName("serialStationLogSearchEdit")
    owner._log_search_edit.setPlaceholderText(owner.tr("Search log text"))
    owner._log_search_edit.setToolTip(owner.tr("Filter visible log entries by text"))
    owner._log_search_edit.textChanged.connect(owner._render_log_entries)
    row1.addWidget(owner._log_filter_combo, 1)
    row1.addWidget(owner._log_search_edit, 2)
    col.addLayout(row1)

    # 第 2 行：session log 路径（占满宽度）。
    owner._log_path_edit = QLineEdit(root)
    owner._log_path_edit.setObjectName("serialStationLogPathEdit")
    owner._log_path_edit.setPlaceholderText(owner.tr("Session log JSONL path"))
    col.addWidget(owner._log_path_edit)

    # 第 3 行：统计 + Save/Replay 按钮。
    row3 = QHBoxLayout()
    row3.setSpacing(6)
    owner._log_stats_label = QLabel(owner.tr("0 entries"), root)
    owner._log_stats_label.setObjectName("serialStationLogStatsLabel")
    owner._log_stats_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
    owner._export_log_button = QPushButton(owner.tr("Save Log"), root)
    owner._export_log_button.setObjectName("serialStationExportLogButton")
    owner._export_log_button.setToolTip(owner.tr("Save current session log"))
    owner._export_log_button.clicked.connect(owner._export_log)
    owner._replay_log_button = QPushButton(owner.tr("Replay Log"), root)
    owner._replay_log_button.setObjectName("serialStationReplayLogButton")
    owner._replay_log_button.setToolTip(owner.tr("Replay saved session log"))
    owner._replay_log_button.clicked.connect(owner._replay_log)
    row3.addWidget(owner._log_stats_label, 1)
    row3.addWidget(owner._export_log_button)
    row3.addWidget(owner._replay_log_button)
    col.addLayout(row3)
    return col


def build_profile_row(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.setSpacing(8)
    owner._profile_path_edit = QLineEdit(root)
    owner._profile_path_edit.setObjectName("serialStationProfilePathEdit")
    owner._profile_path_edit.setPlaceholderText(owner.tr("Profile JSON path"))
    owner._profile_name_edit = QLineEdit(root)
    owner._profile_name_edit.setObjectName("serialStationProfileNameEdit")
    owner._profile_name_edit.setPlaceholderText(owner.tr("Profile name"))
    owner._save_profile_button = QPushButton(owner.tr("Save Profile"), root)
    owner._save_profile_button.setObjectName("serialStationSaveProfileButton")
    owner._save_profile_button.setToolTip(owner.tr("Save current profile"))
    owner._save_profile_button.clicked.connect(owner._save_profile)
    owner._load_profile_button = QPushButton(owner.tr("Load Profile"), root)
    owner._load_profile_button.setObjectName("serialStationLoadProfileButton")
    owner._load_profile_button.setToolTip(owner.tr("Load saved profile"))
    owner._load_profile_button.clicked.connect(owner._load_profile)
    row.addWidget(owner._profile_path_edit, 1)
    row.addWidget(owner._profile_name_edit)
    row.addWidget(owner._save_profile_button)
    row.addWidget(owner._load_profile_button)
    return row


def build_footer(owner: SerialStationSectionsHost, root: QWidget) -> QHBoxLayout:
    row = QHBoxLayout()
    row.addStretch(1)
    owner._clear_button = QPushButton(owner.tr("Clear"), root)
    owner._clear_button.setObjectName("serialStationClearButton")
    owner._clear_button.setToolTip(owner.tr("Clear the serial log (Ctrl+L)"))
    owner._clear_button.clicked.connect(owner._clear_log)
    row.addWidget(owner._clear_button)
    return row
