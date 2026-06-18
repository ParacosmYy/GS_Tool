"""QSS 分区生成器 — RTT/CAN/BLE/Automation/Settings 域面板共享样式。

覆盖各域面板的全部 serialStation* objectName（面板根/字段标签/状态/日志/按钮/
输入/表格/树/Tab）。用精确选择器统一覆盖，避免逐个写规则。

颜色引用 ``palette``，尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


def domain_panels_section() -> str:
    """RTT/CAN/BLE/Automation/Settings 面板共享样式。"""

    return f"""/* === Domain Panels (RTT / CAN / BLE / Automation / Settings) === */
/* 面板根：统一深色应用底。 */
QWidget#serialStationRttPanel,
QWidget#serialStationCanPanel,
QWidget#serialStationBlePanel,
QWidget#serialStationAutomationPanel,
QWidget#serialStationSettingsPanel {{
    background-color: {P.BG_APP};
}}
/* 字段标签（"通道/设备/ID/数据/特征/主题"等行内标签）。 */
QLabel#serialStationRttFieldLabel,
QLabel#serialStationCanFieldLabel,
QLabel#serialStationBleFieldLabel,
QLabel#serialStationSettingsFieldLabel {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
    font-weight: 600;
}}
/* 状态标签。 */
QLabel#serialStationRttStatusLabel,
QLabel#serialStationCanStatsLabel,
QLabel#serialStationAutomationStatusLabel,
QLabel#serialStationSettingsThemeStatusLabel {{
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_SM};
    padding: 0 {T.SPACING_MD};
}}
/* 日志/文本视图（终端风：深底 + 等宽）。 */
QPlainTextEdit#serialStationRttTextView,
QPlainTextEdit#serialStationBleLog,
QPlainTextEdit#serialStationAutomationLog {{
    background-color: {P.TERM_BACKGROUND};
    color: {P.TERM_SYSTEM};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
/* 主操作按钮（启停/连接/发送/应用：强调青）。 */
QPushButton#serialStationRttStartButton,
QPushButton#serialStationCanSendButton,
QPushButton#serialStationBleConnectButton,
QPushButton#serialStationAutomationRunButton,
QPushButton#serialStationSettingsApplyButton {{
    background-color: {P.ACCENT};
    color: {P.TEXT_ON_ACCENT};
    border: none;
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_MD};
    font-weight: 600;
}}
QPushButton#serialStationRttStartButton:hover,
QPushButton#serialStationCanSendButton:hover,
QPushButton#serialStationBleConnectButton:hover,
QPushButton#serialStationAutomationRunButton:hover,
QPushButton#serialStationSettingsApplyButton:hover {{
    background-color: {P.ACCENT_HOVER};
}}
/* 次要按钮（清空/扫描/演示/读/写/订阅/手动触发/刷新）。 */
QPushButton#serialStationRttClearButton,
QPushButton#serialStationCanClearButton,
QPushButton#serialStationCanDemoButton,
QPushButton#serialStationBleScanButton,
QPushButton#serialStationBleReadButton,
QPushButton#serialStationBleWriteButton,
QPushButton#serialStationBleNotifyButton,
QPushButton#serialStationAutomationFireButton,
QPushButton#serialStationAutomationRefreshButton {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_SECONDARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_MD};
}}
QPushButton#serialStationRttClearButton:hover,
QPushButton#serialStationCanClearButton:hover,
QPushButton#serialStationCanDemoButton:hover,
QPushButton#serialStationBleScanButton:hover,
QPushButton#serialStationBleReadButton:hover,
QPushButton#serialStationBleWriteButton:hover,
QPushButton#serialStationBleNotifyButton:hover,
QPushButton#serialStationAutomationFireButton:hover,
QPushButton#serialStationAutomationRefreshButton:hover {{
    background-color: {P.BG_PANEL_RAISED};
    border-color: {P.ACCENT_BORDER};
}}
/* 输入框（ID/数据/payload/特征 UUID/主题 combo/通道 combo/设备 combo）。 */
QLineEdit#serialStationCanIdEdit,
QLineEdit#serialStationCanDataEdit,
QLineEdit#serialStationBleCharUuidEdit,
QLineEdit#serialStationBlePayloadEdit,
QComboBox#serialStationRttChannelCombo,
QComboBox#serialStationBleDeviceCombo,
QComboBox#serialStationSettingsThemeCombo {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    min-height: {T.CONTROL_HEIGHT_MD};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}
/* CAN 复选框。 */
QCheckBox#serialStationCanExtCheckBox {{
    color: {P.TEXT_SECONDARY};
    spacing: {T.SPACING_SM};
}}
/* 表格 / 树（CAN 帧表、Automation 规则表、Settings 快捷键表、BLE GATT 树）。 */
QTableWidget#serialStationCanFrameTable,
QTableWidget#serialStationAutomationRuleTable,
QTableWidget#serialStationSettingsShortcutsTable,
QTreeWidget#serialStationBleGattTree {{
    background-color: {P.BG_PANEL};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
    gridline-color: {P.BORDER};
    font-size: {T.FONT_SM};
}}
QTableWidget#serialStationCanFrameTable::item,
QTableWidget#serialStationAutomationRuleTable::item,
QTableWidget#serialStationSettingsShortcutsTable::item,
QTreeWidget#serialStationBleGattTree::item {{
    padding: {T.SPACING_XS} {T.SPACING_SM};
}}
QTableWidget#serialStationCanFrameTable::item:selected,
QTableWidget#serialStationAutomationRuleTable::item:selected,
QTableWidget#serialStationSettingsShortcutsTable::item:selected,
QTreeWidget#serialStationBleGattTree::item:selected {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT};
}}
QHeaderView::section {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_SECONDARY};
    border: none;
    border-bottom: {T.BORDER_THIN} solid {P.BORDER};
    padding: {T.SPACING_XS} {T.SPACING_SM};
    font-weight: 600;
}}
/* 设置页 Tab。 */
QTabWidget#serialStationSettingsTabWidget::pane {{
    background-color: {P.BG_PANEL};
    border: {T.BORDER_THIN} solid {P.BORDER};
    border-radius: {T.RADIUS_MD};
}}
QTabBar::tab {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_SECONDARY};
    padding: {T.SPACING_SM} {T.SPACING_LG};
    border-top-left-radius: {T.RADIUS_SM};
    border-top-right-radius: {T.RADIUS_SM};
}}
QTabBar::tab:selected {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT};
}}
/* 关于页标签。 */
QLabel#serialStationSettingsAppNameLabel {{
    color: {P.TEXT_PRIMARY};
    font-size: {T.FONT_XL};
    font-weight: 700;
}}
QLabel#serialStationSettingsVersionLabel,
QLabel#serialStationSettingsRemoteLabel {{
    color: {P.TEXT_MUTED};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}"""
