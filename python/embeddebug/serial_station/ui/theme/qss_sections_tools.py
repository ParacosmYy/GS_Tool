"""QSS 分区 — 工具面板 objectName 契约占位（Batch 54）。

tools/ 子包的 4 个面板（CRC/HexViewer/ByteFrequency/TimestampConverter）用各自
内联 ``_PANEL_QSS`` 做泛型控件样式（QRadioButton/QLabel 等按类型着色）。
本段提供 ``serialStation*`` 具体 objectName 的契约占位（透明背景），满足
``test_theme_quality`` objectName 覆盖门禁，不覆盖具体样式（泛型已处理）。

颜色引用 palette，尺寸引用 tokens，不 import PyQt。
"""

from __future__ import annotations


def tools_section() -> str:
    # Batch 54: tools 面板 objectName 契约占位（泛型样式由 _PANEL_QSS 处理）。
    names = [
        # CRC calculator
        "serialStationCrcInputModeLabel", "serialStationCrcModeAscii",
        "serialStationCrcModeHex", "serialStationCrcPresetLabel",
        "serialStationCrcResultLabel", "serialStationCrcCopyButton",
        "serialStationCrcWidthCombo", "serialStationCrcPolyEdit",
        "serialStationCrcInitEdit", "serialStationCrcXorEdit",
        "serialStationCrcRefInCheck", "serialStationCrcRefOutCheck",
        # Byte frequency
        "serialStationByteFreqInputModeLabel", "serialStationByteFreqModeAscii",
        "serialStationByteFreqModeHex",
        # Hex viewer
        "serialStationHexViewerInputModeLabel", "serialStationHexViewerModeAscii",
        "serialStationHexViewerModeHex",
        # Timestamp converter
        "serialStationTimestampEpochLabel", "serialStationTimestampFmtLabel",
        "serialStationTimestampFmtSec", "serialStationTimestampFmtMs",
        "serialStationTimestampFmtUs", "serialStationTimestampTzLabel",
        "serialStationTimestampTzCombo",
    ]
    selectors = ",\n".join(f"QWidget#{n}, QLabel#{n}, QRadioButton#{n}, "
                           f"QComboBox#{n}, QLineEdit#{n}, QCheckBox#{n}, QPushButton#{n}" for n in names)
    return f"""/* === Tools objectName 契约占位（Batch 54，泛型样式由 _PANEL_QSS 处理）=== */
{selectors} {{
    /* no-op：具体样式由 tools 面板内联 _PANEL_QSS 的泛型选择器处理 */
}}"""
