"""Hex dump 查看器面板。

- ``to_ascii_repr`` / ``format_hex_line`` / ``format_hex_dump`` / ``parse_hex_input``：
  纯函数格式化核（无 Qt 依赖，便于单测覆盖）。
- ``HexViewerPanel``：UI 包装层，输入控件 + 只读格式化视图 + 字节计数。

约束：仅依赖标准库 + PyQt6 + theme；无第三方 hexdump 库；颜色全部走 palette 常量。
"""

from __future__ import annotations

from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import (
    QHBoxLayout, QLabel, QPlainTextEdit, QPushButton, QRadioButton,
    QTextEdit, QVBoxLayout, QWidget,
)

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


# ── 常量 ───────────────────────────────────────────────────────────
BYTES_PER_LINE = 16
_HEX_DIGITS = "0123456789abcdefABCDEF"


# ── 纯函数格式化核（无 Qt 依赖） ────────────────────────────────────
def to_ascii_repr(byte: int) -> str:
    """返回可打印字符或 '.'（0x20-0x7e 范围外的非可见字符以点占位）。"""
    if 0x20 <= byte <= 0x7E:
        return chr(byte)
    return "."


def format_hex_line(offset: int, data: bytes, bytes_per_line: int = BYTES_PER_LINE) -> str:
    """格式化单行：'offset  g1_hex  g2_hex  ascii'。

    结构：8 位 hex 偏移 + 2 空格 + 字节组（前半 + 2 空格 + 后半）+ 2 空格 + ASCII。
    字节列宽度恒定（不足 bytes_per_line 时用空格补齐），保证列对齐。
    ASCII 列：前半 + 1 空格 + 后半（仅当后半非空时插入分隔）。
    """
    half = bytes_per_line // 2
    g1_data = data[:half]
    g2_data = data[half:bytes_per_line]

    g1_hex = " ".join(f"{b:02x}" for b in g1_data)
    g2_hex = " ".join(f"{b:02x}" for b in g2_data)

    # 每半组等宽："XX XX ... XX" = 2*half + (half-1) 空格 = 3*half - 1 字符
    g1_width = max(half * 3 - 1, 0)
    g2_width = max(half * 3 - 1, 0)

    g1_ascii = "".join(to_ascii_repr(b) for b in g1_data)
    g2_ascii = "".join(to_ascii_repr(b) for b in g2_data)
    ascii_part = f"{g1_ascii} {g2_ascii}" if g2_ascii else g1_ascii

    return f"{offset:08x}  {g1_hex:<{g1_width}}  {g2_hex:<{g2_width}}  {ascii_part}"


def format_hex_dump(data: bytes, base_offset: int = 0, bytes_per_line: int = BYTES_PER_LINE) -> str:
    """格式化完整 dump：按 bytes_per_line 切片，多行以 '\\n' 连接。空数据返回 ""。"""
    if not data:
        return ""
    lines: list[str] = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i + bytes_per_line]
        lines.append(format_hex_line(base_offset + i, chunk, bytes_per_line))
    return "\n".join(lines)


def parse_hex_input(text: str) -> bytes:
    """解析用户 Hex 输入（容忍空格 / 换行 / '0x' 前缀 / 逗号）。

    遇到非 hex 字符抛 ValueError；hex 位数奇数抛 ValueError。
    """
    cleaned = text.replace(",", " ").replace("\n", " ").replace("\t", " ").replace("\r", " ")
    tokens = cleaned.split()
    if not tokens:
        return b""

    parts: list[str] = []
    for tok in tokens:
        low = tok.lower()
        if low.startswith("0x"):
            tok = tok[2:]
        if not tok:
            raise ValueError("empty hex token after stripping 0x prefix")
        if any(c not in _HEX_DIGITS for c in tok):
            raise ValueError(f"invalid hex token: {tok!r}")
        parts.append(tok)

    hex_str = "".join(parts)
    if len(hex_str) % 2 != 0:
        raise ValueError(f"odd number of hex digits: {len(hex_str)}")
    return bytes.fromhex(hex_str)


# ── 面板 QSS（颜色全部走 palette / tokens，无硬编码） ──────────────
_PANEL_QSS = f"""
QWidget#serialStationHexViewer {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationHexViewerInput,
QWidget#serialStationHexViewerView,
QWidget#serialStationHexViewerStatus {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
}}
QTextEdit, QPlainTextEdit {{
    background: {P.BG_INPUT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT}; font-family: {T.FONT_FAMILY_MONO};
}}
QTextEdit:focus, QPlainTextEdit:focus {{ border: 1px solid {P.ACCENT}; }}
QPushButton {{
    background: {P.ACCENT_SOFT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD}; padding: {T.PADDING_MD};
}}
QPushButton:hover {{ border: 1px solid {P.ACCENT}; color: {P.ACCENT_HOVER}; }}
QPushButton:pressed {{ background: {P.ACCENT_PRESSED}; }}
QLabel {{ color: {P.TEXT_SECONDARY}; background: transparent; border: none; }}
QRadioButton {{ color: {P.TEXT_PRIMARY}; background: transparent; border: none; }}
"""


class HexViewerPanel(QWidget):
    """Hex dump 查看器面板：输入 ASCII/Hex → 格式化为只读 offset/bytes/ASCII dump。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationHexViewer")
        self.setStyleSheet(_PANEL_QSS)
        self._build_ui()

    # ── UI 构建 ────────────────────────────────────────────────────
    def _build_ui(self) -> None:
        root = QVBoxLayout(self)
        root.setContentsMargins(12, 12, 12, 12)
        root.setSpacing(8)

        # ── 输入区 ──
        input_box = QWidget()
        input_box.setObjectName("serialStationHexViewerInput")
        input_lay = QVBoxLayout(input_box)
        input_lay.setContentsMargins(8, 8, 8, 8)
        input_lay.setSpacing(6)

        mode_row = QHBoxLayout()
        _mode_label = QLabel(self.tr("输入模式:"))
        _mode_label.setObjectName("serialStationHexViewerInputModeLabel")
        mode_row.addWidget(_mode_label)
        self._mode_ascii = QRadioButton(self.tr("ASCII"))
        self._mode_ascii.setObjectName("serialStationHexViewerModeAscii")
        self._mode_hex = QRadioButton(self.tr("Hex"))
        self._mode_hex.setObjectName("serialStationHexViewerModeHex")
        self._mode_ascii.setChecked(True)
        mode_row.addWidget(self._mode_ascii)
        mode_row.addWidget(self._mode_hex)
        mode_row.addStretch()
        self._format_btn = QPushButton(self.tr("格式化"))
        self._format_btn.setObjectName("serialStationHexViewerFormatBtn")
        self._format_btn.clicked.connect(self._format)
        mode_row.addWidget(self._format_btn)
        input_lay.addLayout(mode_row)

        self._input_edit = QTextEdit()
        self._input_edit.setObjectName("serialStationHexViewerInputEdit")
        self._input_edit.setFont(self._mono_font())
        self._input_edit.setPlaceholderText(
            self.tr("输入 ASCII 文本或 Hex 字节（空格 / 换行 / 逗号分隔）")
        )
        input_lay.addWidget(self._input_edit)
        root.addWidget(input_box)

        # ── 主视图（只读 dump） ──
        view_box = QWidget()
        view_box.setObjectName("serialStationHexViewerView")
        view_lay = QVBoxLayout(view_box)
        view_lay.setContentsMargins(8, 8, 8, 8)
        view_lay.setSpacing(6)
        self._view = QPlainTextEdit()
        self._view.setObjectName("serialStationHexViewerDump")
        self._view.setReadOnly(True)
        self._view.setFont(self._mono_font())
        self._view.setPlaceholderText(self.tr("格式化后的 Hex dump 显示在此"))
        view_lay.addWidget(self._view)
        root.addWidget(view_box, stretch=1)

        # ── 状态栏（字节计数） ──
        status_box = QWidget()
        status_box.setObjectName("serialStationHexViewerStatus")
        status_lay = QHBoxLayout(status_box)
        status_lay.setContentsMargins(8, 8, 8, 8)
        self._status_label = QLabel(self.tr("字节数: 0"))
        self._status_label.setObjectName("serialStationHexViewerCount")
        status_lay.addWidget(self._status_label)
        status_lay.addStretch()
        root.addWidget(status_box)

    def _mono_font(self) -> QFont:
        """构建等宽字体（Hex 输入与 dump 显示用）。"""
        font = QFont()
        font.setFamilies(["JetBrains Mono", "Cascadia Code", "Consolas", "monospace"])
        font.setStyleHint(QFont.StyleHint.Monospace)
        font.setPointSize(T.FONT_POINT_DESC)
        return font

    # ── 行为 ──────────────────────────────────────────────────────
    def _format(self) -> None:
        """读 input + 模式，调格式化核，更新主视图与状态栏。"""
        text = self._input_edit.toPlainText()
        if self._mode_hex.isChecked():
            try:
                data = parse_hex_input(text)
            except ValueError:
                self._status_label.setText(self.tr("字节数: 0（Hex 输入无效）"))
                self._view.setPlainText("")
                return
        else:
            data = text.encode("utf-8", errors="replace")
        self._render(data)

    def _render(self, data: bytes) -> None:
        """渲染 dump 到主视图并更新计数。"""
        self._view.setPlainText(format_hex_dump(data))
        self._status_label.setText(self.tr("字节数: {}").format(len(data)))

    def set_data(self, data: bytes) -> None:
        """程序化设置输入数据并刷新主视图（供测试与外部调用方使用）。

        以 Hex 模式呈现：输入框填入空格分隔的 hex 字符串，主视图显示完整 dump。
        """
        hex_text = " ".join(f"{b:02x}" for b in data)
        self._mode_hex.setChecked(True)
        self._input_edit.setPlainText(hex_text)
        self._render(data)
