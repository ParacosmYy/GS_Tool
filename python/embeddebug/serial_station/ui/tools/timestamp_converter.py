"""时间戳 / 日期转换面板。

- 纯函数核：epoch ↔ datetime / ISO / hex，全部无 Qt 依赖，便于单测。
- ``TZ_PRESETS``：常用时区预设（UTC / 中国 / 美东 / 美西 / 伦敦）。
- ``TimestampConverterPanel``：UI 包装层，输入变更即重算 4 个结果字段。

约束：仅依赖标准库 + PyQt6 + theme；无第三方库；颜色全部走 palette 常量。
"""

from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime, timedelta, timezone

from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import (
    QApplication, QComboBox, QHBoxLayout, QLabel, QLineEdit, QPushButton,
    QRadioButton, QVBoxLayout, QWidget,
)

from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme import tokens as T


# ── 时间戳转换核（纯函数，无 Qt 依赖） ────────────────────────────────
def epoch_to_datetime(epoch: float, tz_aware: bool = False) -> datetime:
    """epoch（秒，UTC）→ datetime。负值抛 ValueError。

    tz_aware=True 返回带 ``timezone.utc`` 的 aware datetime；否则返回 naive UTC。
    """
    if epoch < 0:
        raise ValueError(f"epoch must be non-negative, got {epoch}")
    dt = datetime.fromtimestamp(epoch, tz=timezone.utc)
    return dt if tz_aware else dt.replace(tzinfo=None)


def datetime_to_epoch(dt: datetime) -> float:
    """datetime → epoch 秒（float）。naive 视为 UTC，aware 走原生 timestamp()。"""
    if dt.tzinfo is None:
        return dt.replace(tzinfo=timezone.utc).timestamp()
    return dt.timestamp()


def epoch_to_iso(epoch: float, tz_aware: bool = False) -> str:
    """epoch（秒）→ ISO 8601 字符串。负值抛 ValueError。"""
    return epoch_to_datetime(epoch, tz_aware=tz_aware).isoformat()


def iso_to_epoch(iso_str: str) -> float:
    """ISO 8601 字符串 → epoch 秒。naive 视为 UTC；解析失败抛 ValueError。"""
    try:
        dt = datetime.fromisoformat(iso_str)
    except ValueError as exc:
        raise ValueError(f"invalid ISO string: {iso_str!r}") from exc
    return datetime_to_epoch(dt)


def epoch_to_hex(epoch: float) -> str:
    """epoch → 32-bit hex（嵌入式调试场景，如 RTC 寄存器值 / epoch 回滚分析）。

    负值抛 ValueError；> 0xFFFFFFFF 按 32-bit wrap（对齐 Y2038 风险分析）。
    """
    if epoch < 0:
        raise ValueError(f"epoch must be non-negative, got {epoch}")
    return f"0x{int(epoch) & 0xFFFFFFFF:08x}"


# ── 时区预设 ──────────────────────────────────────────────────────────
@dataclass(frozen=True)
class TzPreset:
    """时区预设：固定 UTC 偏移（秒）+ 展示标签。"""
    name: str
    offset_seconds: int
    label: str


TZ_PRESETS: list[TzPreset] = [
    TzPreset("UTC", 0, "UTC"),
    TzPreset("China", 8 * 3600, "Asia/Shanghai (UTC+8)"),
    TzPreset("US-Pacific", -8 * 3600, "America/Los_Angeles (UTC-8)"),
    TzPreset("US-Eastern", -5 * 3600, "America/New_York (UTC-5)"),
    TzPreset("Europe-London", 0, "Europe/London (UTC+0)"),
]


# ── 面板 QSS（颜色全部走 palette / tokens，无硬编码） ──────────────
_PANEL_QSS = f"""
QWidget#serialStationTimestampConverter {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_LG};
}}
QWidget#serialStationTimestampConverterInput,
QWidget#serialStationTimestampConverterConfig,
QWidget#serialStationTimestampConverterResult {{
    background: {P.BG_PANEL}; border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
}}
QLineEdit, QComboBox {{
    background: {P.BG_INPUT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD};
    padding: {T.PADDING_INPUT}; font-family: {T.FONT_FAMILY_MONO};
}}
QLineEdit:focus, QComboBox:focus {{ border: 1px solid {P.ACCENT}; }}
QLineEdit#serialStationTimestampConverterResultEdit {{ color: {P.ACCENT}; font-weight: 600; }}
QPushButton {{
    background: {P.ACCENT_SOFT}; color: {P.TEXT_PRIMARY};
    border: 1px solid {P.BORDER}; border-radius: {T.RADIUS_MD}; padding: {T.PADDING_MD};
}}
QPushButton:hover {{ border: 1px solid {P.ACCENT}; color: {P.ACCENT_HOVER}; }}
QPushButton:pressed {{ background: {P.ACCENT_PRESSED}; }}
QLabel {{ color: {P.TEXT_SECONDARY}; background: transparent; border: none; }}
QRadioButton {{ color: {P.TEXT_PRIMARY}; background: transparent; border: none; }}
"""


class TimestampConverterPanel(QWidget):
    """时间戳转换面板：实时显示 epoch 在不同表示下的 4 个结果。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationTimestampConverter")
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
        input_box.setObjectName("serialStationTimestampConverterInput")
        input_lay = QVBoxLayout(input_box)
        input_lay.setContentsMargins(8, 8, 8, 8)
        input_lay.setSpacing(6)

        epoch_row = QHBoxLayout()
        _epoch_label = QLabel(self.tr("Epoch:"))
        _epoch_label.setObjectName("serialStationTimestampEpochLabel")
        epoch_row.addWidget(_epoch_label)
        self._epoch_edit = QLineEdit()
        self._epoch_edit.setObjectName("serialStationTimestampConverterEpochEdit")
        self._epoch_edit.setFont(self._mono_font())
        self._epoch_edit.setPlaceholderText(self.tr("输入 Unix epoch（秒 / 毫秒 / 微秒）"))
        epoch_row.addWidget(self._epoch_edit, stretch=1)
        self._now_button = QPushButton(self.tr("Now"))
        self._now_button.setObjectName("serialStationTimestampConverterNowButton")
        self._now_button.clicked.connect(self._fill_now)
        epoch_row.addWidget(self._now_button)
        input_lay.addLayout(epoch_row)

        fmt_row = QHBoxLayout()
        _fmt_label = QLabel(self.tr("格式:"))
        _fmt_label.setObjectName("serialStationTimestampFmtLabel")
        fmt_row.addWidget(_fmt_label)
        self._fmt_sec = QRadioButton(self.tr("秒"))
        self._fmt_sec.setObjectName("serialStationTimestampFmtSec")
        self._fmt_ms = QRadioButton(self.tr("毫秒"))
        self._fmt_ms.setObjectName("serialStationTimestampFmtMs")
        self._fmt_us = QRadioButton(self.tr("微秒"))
        self._fmt_us.setObjectName("serialStationTimestampFmtUs")
        self._fmt_sec.setChecked(True)
        fmt_row.addWidget(self._fmt_sec)
        fmt_row.addWidget(self._fmt_ms)
        fmt_row.addWidget(self._fmt_us)
        fmt_row.addStretch()
        input_lay.addLayout(fmt_row)
        root.addWidget(input_box)

        # ── 时区区 ──
        self._build_tz_row(root)

        # ── 结果区 ──
        root.addWidget(self._build_result_box())

        # ── 信号接入（构建完成后再接，避免构建中误触发） ──
        self._epoch_edit.textChanged.connect(self._recompute)
        self._fmt_sec.toggled.connect(self._recompute)
        self._fmt_ms.toggled.connect(self._recompute)
        self._fmt_us.toggled.connect(self._recompute)
        self._tz_combo.currentIndexChanged.connect(self._recompute)

    def _build_tz_row(self, root: QVBoxLayout) -> None:
        cfg = QWidget()
        cfg.setObjectName("serialStationTimestampConverterConfig")
        lay = QHBoxLayout(cfg)
        lay.setContentsMargins(8, 8, 8, 8)
        lay.setSpacing(6)
        _tz_label = QLabel(self.tr("时区:"))
        _tz_label.setObjectName("serialStationTimestampTzLabel")
        lay.addWidget(_tz_label)
        self._tz_combo = QComboBox()
        self._tz_combo.setObjectName("serialStationTimestampTzCombo")
        for preset in TZ_PRESETS:
            self._tz_combo.addItem(self.tr(preset.label), preset)
        lay.addWidget(self._tz_combo)
        lay.addStretch()
        root.addWidget(cfg)

    def _build_result_box(self) -> QWidget:
        result_box = QWidget()
        result_box.setObjectName("serialStationTimestampConverterResult")
        lay = QVBoxLayout(result_box)
        lay.setContentsMargins(8, 8, 8, 8)
        lay.setSpacing(6)
        self._result_edits: dict[str, QLineEdit] = {}
        for key, label in (
            ("datetime", self.tr("Datetime:")),
            ("iso", self.tr("ISO:")),
            ("hex", self.tr("Hex:")),
            ("weekday", self.tr("Weekday:")),
        ):
            row = QHBoxLayout()
            _lbl = QLabel(label)
            _lbl.setObjectName(f"serialStationTimestampResult_{key}_Label")
            row.addWidget(_lbl)
            edit = QLineEdit()
            edit.setObjectName(f"serialStationTimestampResult_{key}_Edit")
            edit.setReadOnly(True)
            edit.setFont(self._mono_font())
            row.addWidget(edit, stretch=1)
            copy_btn = QPushButton(self.tr("复制"))
            copy_btn.setObjectName(f"serialStationTimestampResult_{key}_CopyButton")
            copy_btn.clicked.connect(lambda checked=False, e=edit: self._copy_field(e))
            row.addWidget(copy_btn)
            lay.addLayout(row)
            self._result_edits[key] = edit
        return result_box

    def _mono_font(self) -> QFont:
        """构建等宽字体（epoch 输入与结果显示用）。"""
        font = QFont()
        font.setFamilies(["JetBrains Mono", "Cascadia Code", "Consolas", "monospace"])
        font.setStyleHint(QFont.StyleHint.Monospace)
        font.setPointSize(T.FONT_POINT_DESC)
        return font

    # ── 重算 / Now / Copy ────────────────────────────────────────
    def _selected_offset_seconds(self) -> int:
        preset = self._tz_combo.currentData()
        return preset.offset_seconds if preset is not None else 0

    def _fill_now(self) -> None:
        """填入当前 epoch（按所选格式单位）。"""
        now_epoch = datetime.now(timezone.utc).timestamp()
        if self._fmt_ms.isChecked():
            text = f"{now_epoch * 1000:.0f}"
        elif self._fmt_us.isChecked():
            text = f"{now_epoch * 1_000_000:.0f}"
        else:
            text = f"{now_epoch:.0f}"
        self._epoch_edit.setText(text)

    def _recompute(self) -> None:
        """读 epoch + 格式 + 时区，调纯函数核，更新 4 个结果字段。"""
        text = self._epoch_edit.text().strip()
        if not text:
            for edit in self._result_edits.values():
                edit.setText("")
            return
        try:
            raw = float(text)
        except ValueError:
            for edit in self._result_edits.values():
                edit.setText(self.tr("无效输入"))
            return
        # 格式归一化为秒
        if self._fmt_ms.isChecked():
            epoch = raw / 1000.0
        elif self._fmt_us.isChecked():
            epoch = raw / 1_000_000.0
        else:
            epoch = raw
        offset = self._selected_offset_seconds()
        tz = timezone(timedelta(seconds=offset))
        try:
            dt_utc = epoch_to_datetime(epoch, tz_aware=True).astimezone(tz)
            iso = dt_utc.isoformat()
            hex_str = epoch_to_hex(epoch)
            weekday = dt_utc.strftime("%Y-%m-%d %A")
        except ValueError:
            for edit in self._result_edits.values():
                edit.setText(self.tr("无效 epoch"))
            return
        self._result_edits["datetime"].setText(dt_utc.strftime("%Y-%m-%d %H:%M:%S"))
        self._result_edits["iso"].setText(iso)
        self._result_edits["hex"].setText(hex_str)
        self._result_edits["weekday"].setText(weekday)

    def _copy_field(self, edit: QLineEdit) -> None:
        """复制指定结果字段到系统剪贴板。"""
        clipboard = QApplication.clipboard()
        if clipboard is not None:
            clipboard.setText(edit.text())
