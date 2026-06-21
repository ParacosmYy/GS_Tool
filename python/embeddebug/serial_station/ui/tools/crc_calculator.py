"""CRC 计算器面板。

- ``compute_crc``：纯函数 CRC 核（无 Qt 依赖，便于单测覆盖 catalog check value）。
- ``CRC_PRESETS``：4 个常用预设及其 check value（CRC of ``b"123456789"``）。
- ``CrcCalculatorPanel``：UI 包装层，输入控件变更即重算结果。

约束：仅依赖标准库 + PyQt6 + theme；无第三方 CRC 库；颜色全部走 palette 常量。
"""

from __future__ import annotations

from dataclasses import dataclass

from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import (
    QApplication, QCheckBox, QComboBox, QHBoxLayout, QLabel, QLineEdit,
    QPushButton, QRadioButton, QTextEdit, QVBoxLayout, QWidget,
)

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


# ── CRC 算法核（纯函数，无 Qt 依赖） ────────────────────────────────
def _reflect(value: int, width: int) -> int:
    """反射 value 的低 width 位（bit i ↔ bit width-1-i）。"""
    result = 0
    for i in range(width):
        if value & (1 << i):
            result |= 1 << (width - 1 - i)
    return result


def compute_crc(
    data: bytes,
    width: int,
    poly: int,
    init: int,
    ref_in: bool,
    ref_out: bool,
    xor_out: int,
) -> int:
    """对 data 计算 CRC。width ∈ {8,16,32}，否则抛 ValueError。纯函数。

    ref_in / ref_out 控制输入字节 / 输出 CRC 的按位反射；xor_out 应用到最终结果。
    """
    if width not in (8, 16, 32):
        raise ValueError(f"width must be 8/16/32, got {width}")
    mask = (1 << width) - 1
    top_bit = 1 << (width - 1)
    crc = init & mask
    for byte in data:
        b = _reflect(byte, 8) if ref_in else byte
        crc ^= (b << (width - 8)) & mask
        for _ in range(8):
            if crc & top_bit:
                crc = ((crc << 1) ^ poly) & mask
            else:
                crc = (crc << 1) & mask
    if ref_out:
        crc = _reflect(crc, width)
    return (crc ^ xor_out) & mask


# ── 预设 ──────────────────────────────────────────────────────────
@dataclass(frozen=True)
class CrcPreset:
    """CRC 预设：参数 + 自检 check value（CRC of ``b"123456789"``）。"""
    name: str
    width: int
    poly: int
    init: int
    ref_in: bool
    ref_out: bool
    xor_out: int
    check: int


CRC_PRESETS: dict[str, CrcPreset] = {
    "CRC-8/MAXIM": CrcPreset("CRC-8/MAXIM", 8, 0x31, 0x00, True, True, 0x00, 0xA1),
    "CRC-16/MODBUS": CrcPreset("CRC-16/MODBUS", 16, 0x8005, 0xFFFF, True, True, 0x0000, 0x4B37),
    "CRC-16/CCITT-FALSE": CrcPreset("CRC-16/CCITT-FALSE", 16, 0x1021, 0xFFFF, False, False, 0x0000, 0x29B1),
    "CRC-32/ISO-HDLC": CrcPreset("CRC-32/ISO-HDLC", 32, 0x04C11DB7, 0xFFFFFFFF, True, True, 0xFFFFFFFF, 0xCBF43926),
}


# ── 面板 QSS（颜色全部走 palette / tokens，无硬编码） ──────────────
_PANEL_QSS = f"""
QWidget#serialStationCrcCalculator {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationCrcCalculatorInput,
QWidget#serialStationCrcCalculatorConfig,
QWidget#serialStationCrcCalculatorResult {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
}}
QTextEdit, QLineEdit, QComboBox {{
    background: {P.BG_INPUT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT}; font-family: {T.FONT_FAMILY_MONO};
}}
QTextEdit:focus, QLineEdit:focus, QComboBox:focus {{ border: 1px solid {P.ACCENT}; }}
QLineEdit#serialStationCrcResultEdit {{ color: {P.ACCENT}; font-weight: 600; }}
QPushButton {{
    background: {P.ACCENT_SOFT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD}; padding: {T.PADDING_MD};
}}
QPushButton:hover {{ border: 1px solid {P.ACCENT}; color: {P.ACCENT_HOVER}; }}
QPushButton:pressed {{ background: {P.ACCENT_PRESSED}; }}
QLabel {{ color: {P.TEXT_SECONDARY}; background: transparent; border: none; }}
QRadioButton, QCheckBox {{ color: {P.TEXT_PRIMARY}; background: transparent; border: none; }}
"""


class CrcCalculatorPanel(QWidget):
    """CRC 计算器面板：实时显示当前配置下输入数据的 CRC。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationCrcCalculator")
        self.setStyleSheet(_PANEL_QSS)
        self._build_ui()
        self._recompute()

    # ── UI 构建 ────────────────────────────────────────────────────
    def _build_ui(self) -> None:
        root = QVBoxLayout(self)
        root.setContentsMargins(12, 12, 12, 12)
        root.setSpacing(8)

        # ── 输入区 ──
        input_box = QWidget()
        input_box.setObjectName("serialStationCrcCalculatorInput")
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
        self._input_edit.setObjectName("serialStationCrcCalculatorInputEdit")
        self._input_edit.setFont(self._mono_font())
        self._input_edit.setPlaceholderText(
            self.tr("输入 ASCII 文本或 Hex 字节（空格 / 换行 / 逗号分隔）")
        )
        input_lay.addWidget(self._input_edit)
        root.addWidget(input_box)

        # ── 配置区 ──
        self._build_config_row(root)

        # ── 预设按钮 ──
        preset_row = QHBoxLayout()
        preset_row.addWidget(QLabel(self.tr("预设:")))
        self._preset_buttons: dict[str, QPushButton] = {}
        for name, preset in CRC_PRESETS.items():
            btn = QPushButton(self.tr(name))
            btn.clicked.connect(lambda checked=False, p=preset: self._apply_preset(p))
            preset_row.addWidget(btn)
            self._preset_buttons[name] = btn
        preset_row.addStretch()
        root.addLayout(preset_row)

        # ── 结果区 ──
        result_box = QWidget()
        result_box.setObjectName("serialStationCrcCalculatorResult")
        result_lay = QHBoxLayout(result_box)
        result_lay.setContentsMargins(8, 8, 8, 8)
        result_lay.addWidget(QLabel(self.tr("结果:")))
        self._result_edit = QLineEdit()
        self._result_edit.setObjectName("serialStationCrcResultEdit")
        self._result_edit.setReadOnly(True)
        self._result_edit.setFont(self._mono_font())
        result_lay.addWidget(self._result_edit, stretch=1)
        copy_btn = QPushButton(self.tr("复制"))
        copy_btn.clicked.connect(self._copy_result)
        result_lay.addWidget(copy_btn)
        root.addWidget(result_box)

        # ── 信号接入（构建完成后再接，避免构建中误触发） ──
        self._mode_ascii.toggled.connect(self._recompute)
        self._input_edit.textChanged.connect(self._recompute)
        self._width_combo.currentIndexChanged.connect(self._recompute)
        self._poly_edit.textChanged.connect(self._recompute)
        self._init_edit.textChanged.connect(self._recompute)
        self._xor_edit.textChanged.connect(self._recompute)
        self._ref_in_check.toggled.connect(self._recompute)
        self._ref_out_check.toggled.connect(self._recompute)

    def _build_config_row(self, root: QVBoxLayout) -> None:
        """配置行：width / poly / init / ref_in / ref_out / xor_out。"""
        cfg = QWidget()
        cfg.setObjectName("serialStationCrcCalculatorConfig")
        lay = QHBoxLayout(cfg)
        lay.setContentsMargins(8, 8, 8, 8)
        lay.setSpacing(6)

        lay.addWidget(QLabel(self.tr("位宽:")))
        self._width_combo = QComboBox()
        self._width_combo.addItems(["8", "16", "32"])
        lay.addWidget(self._width_combo)

        lay.addWidget(QLabel(self.tr("多项式:")))
        self._poly_edit = QLineEdit("0x31")
        lay.addWidget(self._poly_edit)

        lay.addWidget(QLabel(self.tr("初始值:")))
        self._init_edit = QLineEdit("0x00")
        lay.addWidget(self._init_edit)

        self._ref_in_check = QCheckBox(self.tr("输入反转"))
        self._ref_in_check.setChecked(True)
        self._ref_out_check = QCheckBox(self.tr("输出反转"))
        self._ref_out_check.setChecked(True)
        lay.addWidget(self._ref_in_check)
        lay.addWidget(self._ref_out_check)

        lay.addWidget(QLabel(self.tr("异或输出:")))
        self._xor_edit = QLineEdit("0x00")
        lay.addWidget(self._xor_edit)
        root.addWidget(cfg)

    def _mono_font(self) -> QFont:
        """构建等宽字体（Hex 输入与结果显示用）。"""
        font = QFont()
        font.setFamilies(["JetBrains Mono", "Cascadia Code", "Consolas", "monospace"])
        font.setStyleHint(QFont.StyleHint.Monospace)
        font.setPointSize(10)
        return font

    # ── 预设 / 重算 / 复制 ────────────────────────────────────────
    def _apply_preset(self, preset: CrcPreset) -> None:
        """根据预设填充配置控件（控件信号会自动触发 _recompute）。"""
        self._width_combo.setCurrentText(str(preset.width))
        self._poly_edit.setText(f"0x{preset.poly:X}")
        self._init_edit.setText(f"0x{preset.init:X}")
        self._xor_edit.setText(f"0x{preset.xor_out:X}")
        self._ref_in_check.setChecked(preset.ref_in)
        self._ref_out_check.setChecked(preset.ref_out)

    def _parse_hex_bytes(self, text: str) -> bytes:
        """解析 Hex 输入（容忍空格 / 换行 / 逗号），失败返回 b""。"""
        cleaned = "".join(text.replace(",", " ").split())
        if not cleaned or len(cleaned) % 2 != 0:
            return b""
        try:
            return bytes.fromhex(cleaned)
        except ValueError:
            return b""

    def _parse_int(self, text: str) -> int:
        """解析整数（支持 0x 前缀或无前缀按 hex），失败返回 0。"""
        s = text.strip()
        if not s:
            return 0
        try:
            return int(s, 16)
        except ValueError:
            return 0

    def _recompute(self) -> None:
        """读 input + config，调 compute_crc，更新结果字段。"""
        text = self._input_edit.toPlainText()
        if self._mode_hex.isChecked():
            data = self._parse_hex_bytes(text)
        else:
            data = text.encode("utf-8", errors="replace")
        try:
            width = int(self._width_combo.currentText())
            crc = compute_crc(
                data,
                width,
                self._parse_int(self._poly_edit.text()),
                self._parse_int(self._init_edit.text()),
                self._ref_in_check.isChecked(),
                self._ref_out_check.isChecked(),
                self._parse_int(self._xor_edit.text()),
            )
        except ValueError:
            self._result_edit.setText(self.tr("参数错误"))
            return
        nibbles = (width + 3) // 4
        self._result_edit.setText(f"0x{crc:0{nibbles}X}")

    def _copy_result(self) -> None:
        """复制当前结果到系统剪贴板。"""
        clipboard = QApplication.clipboard()
        if clipboard is not None:
            clipboard.setText(self._result_edit.text())
