"""QSS 域面板分区子助手。

每个子助手返回一个 ``/* === ... === */`` 分区字符串（含分区注释）。
``qss_sections_domain.domain_panels_section`` 按顺序拼接这些子助手，组合后产出
与历史等价的域面板共享 QSS。颜色引用 ``palette``、尺寸引用 ``tokens``，不 import PyQt。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T
from embeddebug.serial_station.ui.theme.qss_sections_domain_parts_aux import (  # noqa: F401
    _about_labels,
    _accent_swatches,
    _settings_tabs,
)


def _panel_roots() -> str:
    """面板根：统一深色应用底。"""

    return f"""/* 面板根：统一深色应用底。 */
QWidget#serialStationRttPanel,
QWidget#serialStationCanPanel,
QWidget#serialStationBlePanel,
QWidget#serialStationAutomationPanel,
QWidget#serialStationSettingsPanel,
QWidget#serialStationSvdPanel {{
    background-color: {P.BG_APP};
}}"""


def _field_labels() -> str:
    """字段标签（"通道/设备/ID/数据/特征/主题"等行内标签）。"""

    return f"""/* 字段标签（"通道/设备/ID/数据/特征/主题"等行内标签）。 */
QLabel#serialStationRttFieldLabel,
QLabel#serialStationCanFieldLabel,
QLabel#serialStationBleFieldLabel,
QLabel#serialStationSettingsFieldLabel,
QLabel#serialStationSvdFieldLabel {{
    color: {P.TEXT_SECONDARY};
    font-size: {T.FONT_SM};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
}}"""


def _status_labels() -> str:
    """状态标签。"""

    return f"""/* 状态标签。 */
QLabel#serialStationRttStatusLabel,
QLabel#serialStationCanStatsLabel,
QLabel#serialStationAutomationStatusLabel,
QLabel#serialStationSettingsThemeStatusLabel,
QLabel#serialStationSvdDeviceLabel,
QLabel#serialStationSvdDetailTitle,
QLabel#serialStationSvdDetailValue {{
    color: {P.TEXT_MUTED};
    font-size: {T.FONT_SM};
    padding: 0 {T.SPACING_MD};
}}"""


def _text_views() -> str:
    """日志/文本视图（终端风：深底 + 等宽）。"""

    return f"""/* 日志/文本视图（终端风：深底 + 等宽）。 */
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
}}"""


def _main_buttons() -> str:
    """主操作按钮（启停/连接/发送/应用：强调青）+ 三态。"""

    return f"""/* 主操作按钮（启停/连接/发送/应用：强调青）。 */
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
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
}}
QPushButton#serialStationRttStartButton:hover,
QPushButton#serialStationCanSendButton:hover,
QPushButton#serialStationBleConnectButton:hover,
QPushButton#serialStationAutomationRunButton:hover,
QPushButton#serialStationSettingsApplyButton:hover {{
    background-color: {P.ACCENT_HOVER};
}}
QPushButton#serialStationRttStartButton:pressed,
QPushButton#serialStationCanSendButton:pressed,
QPushButton#serialStationBleConnectButton:pressed,
QPushButton#serialStationAutomationRunButton:pressed,
QPushButton#serialStationSettingsApplyButton:pressed {{
    background-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationRttStartButton:disabled,
QPushButton#serialStationCanSendButton:disabled,
QPushButton#serialStationBleConnectButton:disabled,
QPushButton#serialStationAutomationRunButton:disabled,
QPushButton#serialStationSettingsApplyButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border: none;
}}"""


def _secondary_buttons() -> str:
    """次要按钮（清空/扫描/演示/读/写/订阅/手动触发/刷新/SVD 加载）+ 三态。"""

    return f"""/* 次要按钮（清空/扫描/演示/读/写/订阅/手动触发/刷新/SVD 加载）。 */
QPushButton#serialStationRttClearButton,
QPushButton#serialStationCanClearButton,
QPushButton#serialStationCanDemoButton,
QPushButton#serialStationBleScanButton,
QPushButton#serialStationBleReadButton,
QPushButton#serialStationBleWriteButton,
QPushButton#serialStationBleNotifyButton,
QPushButton#serialStationAutomationFireButton,
QPushButton#serialStationAutomationRefreshButton,
QPushButton#serialStationSvdLoadButton,
QPushButton#serialStationSvdDemoButton,
QPushButton#serialStationSettingsResetButton {{
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
QPushButton#serialStationAutomationRefreshButton:hover,
QPushButton#serialStationSvdLoadButton:hover,
QPushButton#serialStationSvdDemoButton:hover,
QPushButton#serialStationSettingsResetButton:hover {{
    background-color: {P.BG_PANEL_RAISED};
    border-color: {P.ACCENT_BORDER};
}}
QPushButton#serialStationRttClearButton:pressed,
QPushButton#serialStationCanClearButton:pressed,
QPushButton#serialStationCanDemoButton:pressed,
QPushButton#serialStationBleScanButton:pressed,
QPushButton#serialStationBleReadButton:pressed,
QPushButton#serialStationBleWriteButton:pressed,
QPushButton#serialStationBleNotifyButton:pressed,
QPushButton#serialStationAutomationFireButton:pressed,
QPushButton#serialStationAutomationRefreshButton:pressed,
QPushButton#serialStationSvdLoadButton:pressed,
QPushButton#serialStationSvdDemoButton:pressed {{
    background-color: {P.BG_SELECTION};
    border-color: {P.ACCENT_PRESSED};
}}
QPushButton#serialStationRttClearButton:disabled,
QPushButton#serialStationCanClearButton:disabled,
QPushButton#serialStationCanDemoButton:disabled,
QPushButton#serialStationBleScanButton:disabled,
QPushButton#serialStationBleReadButton:disabled,
QPushButton#serialStationBleWriteButton:disabled,
QPushButton#serialStationBleNotifyButton:disabled,
QPushButton#serialStationAutomationFireButton:disabled,
QPushButton#serialStationAutomationRefreshButton:disabled,
QPushButton#serialStationSvdLoadButton:disabled,
QPushButton#serialStationSvdDemoButton:disabled {{
    background-color: {P.BG_DISABLED};
    color: {P.TEXT_DISABLED};
    border-color: {P.BORDER};
}}"""


def _inputs() -> str:
    """输入框（ID/数据/payload/特征 UUID/主题 combo/通道 combo/设备 combo）。"""

    return f"""/* 输入框（ID/数据/payload/特征 UUID/主题 combo/通道 combo/设备 combo）。 */
QLineEdit#serialStationCanIdEdit,
QLineEdit#serialStationCanDataEdit,
QLineEdit#serialStationBleCharUuidEdit,
QLineEdit#serialStationBlePayloadEdit,
QComboBox#serialStationRttChannelCombo,
QComboBox#serialStationBleDeviceCombo,
QComboBox#serialStationSettingsThemeCombo,
QComboBox#serialStationSettingsBaudrateCombo,
QSpinBox#serialStationSettingsFontSpin {{
    background-color: {P.BG_INPUT};
    color: {P.TEXT_PRIMARY};
    border: {T.BORDER_THIN} solid {P.BORDER_STRONG};
    border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT};
    min-height: {T.CONTROL_HEIGHT_MD};
    font-family: {T.FONT_FAMILY_MONO};
    font-size: {T.FONT_SM};
}}"""


def _checkboxes() -> str:
    """CAN 复选框。"""

    return f"""/* CAN 复选框。 */
QCheckBox#serialStationCanExtCheckBox,
QCheckBox#serialStationSettingsAnimationCheck {{
    color: {P.TEXT_SECONDARY};
    spacing: {T.SPACING_SM};
}}"""


def _tables_and_trees() -> str:
    """表格 / 树（CAN 帧表、Automation 规则表、Settings 快捷键表、BLE GATT 树、SVD 寄存器树/位域表）。"""

    return f"""/* 表格 / 树（CAN 帧表、Automation 规则表、Settings 快捷键表、BLE GATT 树、SVD 寄存器树/位域表）。 */
QTableWidget#serialStationCanFrameTable,
QTableWidget#serialStationAutomationRuleTable,
QTableWidget#serialStationSettingsShortcutsTable,
QTableWidget#serialStationSvdFieldTable,
QTreeWidget#serialStationBleGattTree,
QTreeWidget#serialStationSvdTree {{
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
QTableWidget#serialStationSvdFieldTable::item,
QTreeWidget#serialStationBleGattTree::item,
QTreeWidget#serialStationSvdTree::item {{
    padding: {T.SPACING_XS} {T.SPACING_SM};
}}
QTableWidget#serialStationCanFrameTable::item:selected,
QTableWidget#serialStationAutomationRuleTable::item:selected,
QTableWidget#serialStationSettingsShortcutsTable::item:selected,
QTableWidget#serialStationSvdFieldTable::item:selected,
QTreeWidget#serialStationBleGattTree::item:selected,
QTreeWidget#serialStationSvdTree::item:selected {{
    background-color: {P.ACCENT_SOFT};
    color: {P.ACCENT};
}}
QHeaderView::section {{
    background-color: {P.BG_PANEL_RAISED};
    color: {P.TEXT_SECONDARY};
    border: none;
    border-bottom: {T.BORDER_THIN} solid {P.BORDER};
    padding: {T.SPACING_XS} {T.SPACING_SM};
    font-weight: {T.FONT_WEIGHT_SEMIBOLD};
}}"""


# _settings_tabs, _accent_swatches, _about_labels 已移至
# qss_sections_domain_parts_aux.py（避免本文件超 300 行限制），
# 通过文件顶部 from ... import 重导出，下游 qss_sections_domain 的
# import 语句无需任何改动。
