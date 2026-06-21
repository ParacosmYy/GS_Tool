"""字节频率分析器面板。

- ``compute_frequency`` / ``top_n_bytes`` / ``entropy_bits`` / ``format_stats_text``：
  纯函数核（无 Qt 依赖），便于单测覆盖边界情况（空数据、单字节重复、256 全分布）。
- ``_FrequencyCanvas``：自绘 256 柱频率柱状图，顶四分位条用 ACCENT 高亮。
- ``ByteFrequencyAnalyzer``：UI 包装层，输入 ASCII/Hex → 分析 → 更新柱状图与统计。

约束：仅依赖标准库 + PyQt6 + theme；无 pyqtgraph / numpy；颜色全部走 palette 常量。
"""

from __future__ import annotations

import math

from PyQt6.QtCore import QRectF, QSize, Qt
from PyQt6.QtGui import QColor, QFont, QPainter, QPaintEvent
from PyQt6.QtWidgets import (
    QHBoxLayout, QLabel, QPushButton, QRadioButton, QTextEdit,
    QVBoxLayout, QWidget,
)

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


# ── 频率核（纯函数，无 Qt 依赖） ─────────────────────────────────────
def compute_frequency(data: bytes) -> list[int]:
    """返回长度 256 的频率列表：索引 = 字节值，值 = data 中出现次数。"""

    freq = [0] * 256
    for byte in data:
        freq[byte] += 1
    return freq


def top_n_bytes(freq: list[int], n: int = 10) -> list[tuple[int, int]]:
    """返回前 n 个 (byte_value, count)，按 count 降序；并列时 byte_value 升序。"""

    items = [(i, c) for i, c in enumerate(freq) if c > 0]
    items.sort(key=lambda x: (-x[1], x[0]))
    return items[:n] if n > 0 else []


def entropy_bits(freq: list[int], total: int) -> float:
    """Shannon 熵（bits）。空数据返回 0.0；c > 0 守卫避免 log2(0)。"""

    if total <= 0:
        return 0.0
    entropy = 0.0
    for c in freq:
        if c > 0:
            p = c / total
            entropy -= p * math.log2(p)
    return entropy


def format_stats_text(freq: list[int], total: int) -> str:
    """格式化多行统计：Total / Most common / Least common / Entropy。"""

    if total <= 0:
        return (
            "Total: 0 bytes\n"
            "Most common: -\n"
            "Least common: -\n"
            "Entropy: 0.000 bits"
        )
    top = top_n_bytes(freq, 1)
    most = f"0x{top[0][0]:02X} ({top[0][1]}x)" if top else "-"
    nonzero = sorted(
        ((i, c) for i, c in enumerate(freq) if c > 0),
        key=lambda x: (x[1], x[0]),
    )
    least = f"0x{nonzero[0][0]:02X} ({nonzero[0][1]}x)" if nonzero else "-"
    ent = entropy_bits(freq, total)
    return (
        f"Total: {total} bytes\n"
        f"Most common: {most}\n"
        f"Least common: {least}\n"
        f"Entropy: {ent:.3f} bits"
    )


# ── 面板 QSS（颜色全部走 palette / tokens，无硬编码） ──────────────
_PANEL_QSS = f"""
QWidget#serialStationByteFreqAnalyzer {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationByteFreqAnalyzerInput,
QWidget#serialStationByteFreqAnalyzerResult {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
}}
QTextEdit {{
    background: {P.BG_INPUT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT}; font-family: {T.FONT_FAMILY_MONO};
}}
QTextEdit:focus {{ border: 1px solid {P.ACCENT}; }}
QPushButton {{
    background: {P.ACCENT_SOFT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD}; padding: {T.PADDING_MD};
}}
QPushButton:hover {{ border: 1px solid {P.ACCENT}; color: {P.ACCENT_HOVER}; }}
QPushButton:pressed {{ background: {P.ACCENT_PRESSED}; }}
QLabel {{ color: {P.TEXT_SECONDARY}; background: transparent; border: none; }}
QRadioButton {{ color: {P.TEXT_PRIMARY}; background: transparent; border: none; }}
"""


class _FrequencyCanvas(QWidget):
    """自绘字节频率柱状图：256 个垂直条，顶四分位条用 ACCENT 高亮。

    paintEvent 画 256 个垂直条（条宽 = width / 256），条高 ∝ count / max_count；
    count >= max_count * 0.75 的条用 ACCENT，其余用 ACCENT_SOFT，视觉上突出主导字节。
    空数据时画居中提示文字。配色全部走 palette token，无散落硬编码。
    """

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationByteFreqCanvas")
        self._freq: list[int] = [0] * 256
        self.setMinimumHeight(120)

    def set_frequency(self, freq: list[int]) -> None:
        """更新频率数据并触发重绘。freq 长度必须为 256，否则置零（防御性）。"""

        self._freq = list(freq) if len(freq) == 256 else [0] * 256
        self.update()

    def sizeHint(self) -> QSize:
        return QSize(400, 160)

    def paintEvent(self, event: QPaintEvent) -> None:
        """自绘 256 柱柱状图：max_count 满高，count >= 75% max 用 ACCENT。"""

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, False)

        rect = QRectF(self.rect())
        painter.fillRect(rect, QColor(P.BG_INPUT))

        max_count = max(self._freq) if self._freq else 0
        if max_count <= 0:
            painter.setPen(QColor(P.TEXT_MUTED))
            painter.drawText(
                rect, int(Qt.AlignmentFlag.AlignCenter), self.tr("(no data)")
            )
            return

        threshold = max_count * 3 / 4
        bar_w = rect.width() / 256.0
        for i, count in enumerate(self._freq):
            if count <= 0:
                continue
            bar_h = rect.height() * (count / max_count)
            x = rect.left() + i * bar_w
            y = rect.bottom() - bar_h
            color = P.ACCENT if count >= threshold else P.ACCENT_SOFT
            painter.fillRect(
                QRectF(x, y, max(bar_w - 0.5, 1.0), bar_h), QColor(color)
            )


class ByteFrequencyAnalyzer(QWidget):
    """字节频率分析器面板：输入 ASCII/Hex → 频率核 → 柱状图 + 统计。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationByteFreqAnalyzer")
        self.setStyleSheet(_PANEL_QSS)
        self._build_ui()

    # ── UI 构建 ────────────────────────────────────────────────────
    def _build_ui(self) -> None:
        root = QVBoxLayout(self)
        root.setContentsMargins(12, 12, 12, 12)
        root.setSpacing(8)

        # ── 输入区 ──
        input_box = QWidget()
        input_box.setObjectName("serialStationByteFreqAnalyzerInput")
        input_lay = QVBoxLayout(input_box)
        input_lay.setContentsMargins(8, 8, 8, 8)
        input_lay.setSpacing(6)

        mode_row = QHBoxLayout()
        mode_row.addWidget(QLabel(self.tr("输入模式:")))
        self._mode_ascii = QRadioButton(self.tr("ASCII"))
        self._mode_hex = QRadioButton(self.tr("Hex"))
        self._mode_ascii.setChecked(True)
        mode_row.addWidget(self._mode_ascii)
        mode_row.addWidget(self._mode_hex)
        mode_row.addStretch()
        input_lay.addLayout(mode_row)

        self._input_edit = QTextEdit()
        self._input_edit.setObjectName("serialStationByteFreqAnalyzerInputEdit")
        self._input_edit.setFont(self._mono_font())
        self._input_edit.setPlaceholderText(
            self.tr("输入 ASCII 文本或 Hex 字节（空格 / 换行 / 逗号分隔）")
        )
        input_lay.addWidget(self._input_edit)
        root.addWidget(input_box)

        # ── 按钮行 ──
        btn_row = QHBoxLayout()
        self._analyze_btn = QPushButton(self.tr("分析"))
        self._analyze_btn.setObjectName("serialStationByteFreqAnalyzerAnalyzeBtn")
        self._analyze_btn.clicked.connect(self._analyze)
        btn_row.addWidget(self._analyze_btn)
        self._clear_btn = QPushButton(self.tr("清空"))
        self._clear_btn.setObjectName("serialStationByteFreqAnalyzerClearBtn")
        self._clear_btn.clicked.connect(self._clear)
        btn_row.addWidget(self._clear_btn)
        btn_row.addStretch()
        root.addLayout(btn_row)

        # ── 柱状图画布 ──
        self._canvas = _FrequencyCanvas()
        root.addWidget(self._canvas, stretch=1)

        # ── 统计区 ──
        result_box = QWidget()
        result_box.setObjectName("serialStationByteFreqAnalyzerResult")
        result_lay = QVBoxLayout(result_box)
        result_lay.setContentsMargins(8, 8, 8, 8)
        self._stats_label = QLabel(format_stats_text([0] * 256, 0))
        self._stats_label.setObjectName("serialStationByteFreqAnalyzerStatsLabel")
        self._stats_label.setFont(self._mono_font())
        self._stats_label.setWordWrap(True)
        result_lay.addWidget(self._stats_label)
        root.addWidget(result_box)

    def _mono_font(self) -> QFont:
        font = QFont()
        font.setFamilies(["JetBrains Mono", "Cascadia Code", "Consolas", "monospace"])
        font.setStyleHint(QFont.StyleHint.Monospace)
        font.setPointSize(10)
        return font

    # ── 分析 / 清空 ────────────────────────────────────────────────
    def _parse_hex_bytes(self, text: str) -> bytes:
        cleaned = "".join(text.replace(",", " ").split())
        if not cleaned or len(cleaned) % 2 != 0:
            return b""
        try:
            return bytes.fromhex(cleaned)
        except ValueError:
            return b""

    def _analyze(self) -> None:
        text = self._input_edit.toPlainText()
        if self._mode_hex.isChecked():
            data = self._parse_hex_bytes(text)
        else:
            data = text.encode("utf-8", errors="replace")
        freq = compute_frequency(data)
        total = len(data)
        self._canvas.set_frequency(freq)
        self._stats_label.setText(format_stats_text(freq, total))

    def _clear(self) -> None:
        self._input_edit.clear()
        self._canvas.set_frequency([0] * 256)
        self._stats_label.setText(format_stats_text([0] * 256, 0))
