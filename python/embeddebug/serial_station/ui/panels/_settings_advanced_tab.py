"""设置页「高级」Tab —— 字体大小 / 动画开关 / 默认波特率 / 恢复默认。

从 ``settings_panel`` 抽出以守 300 行运行时文件门禁（Batch 23）。控件全部
objectName 化（``serialStationSettings*`` 前缀），用户可见文字用 ``tr()``，
即时持久化到 ``SettingsManager``。

约束：只依赖 PyQt6 + services.settings_service，不碰 controller/transport/
protocol。与 ``_accent_row`` 同一 docstring 边界。
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from PyQt6.QtWidgets import (
    QCheckBox,
    QComboBox,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.services.settings_service import (
    SettingsManager,
)

if TYPE_CHECKING:
    from embeddebug.serial_station.ui.panels.settings_panel import SettingsPanel


# 默认波特率候选（覆盖常用 UART 场景；与 connection_toolbar 默认列表对齐）。
_BAUDRATES: tuple[int, ...] = (
    9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600,
)


def build_advanced_tab(panel: SettingsPanel, parent: QWidget) -> QWidget:
    """构建「高级」Tab：字体大小 / 动画 / 默认波特率 / 恢复默认按钮。

    控件存到 ``panel._font_spin`` / ``panel._animation_check`` /
    ``panel._baudrate_combo`` / ``panel._reset_button``，供 on_enter 同步当前值。
    """

    tab = QWidget(parent)
    layout = QVBoxLayout(tab)
    layout.setContentsMargins(16, 16, 16, 16)
    layout.setSpacing(12)

    settings = SettingsManager.instance().get()

    # ── 字体大小 ──────────────────────────────────────────────────
    font_row = QHBoxLayout()
    font_label = QLabel(panel._widget.tr("字体大小"), tab)
    font_label.setObjectName("serialStationSettingsFieldLabel")
    font_spin = QSpinBox(tab)
    font_spin.setObjectName("serialStationSettingsFontSpin")
    font_spin.setRange(8, 24)
    font_spin.setSingleStep(1)
    font_spin.setValue(settings.font_point)
    font_spin.valueChanged.connect(_on_font_changed)
    panel._font_spin = font_spin
    font_row.addWidget(font_label)
    font_row.addWidget(font_spin, 1)
    layout.addLayout(font_row)

    # ── 动画开关 ──────────────────────────────────────────────────
    anim_row = QHBoxLayout()
    anim_label = QLabel(panel._widget.tr("启用动画"), tab)
    anim_label.setObjectName("serialStationSettingsFieldLabel")
    anim_check = QCheckBox(tab)
    anim_check.setObjectName("serialStationSettingsAnimationCheck")
    anim_check.setChecked(settings.animation_enabled)
    anim_check.toggled.connect(_on_animation_toggled)
    panel._animation_check = anim_check
    anim_row.addWidget(anim_label)
    anim_row.addWidget(anim_check)
    anim_row.addStretch(1)
    layout.addLayout(anim_row)

    # ── 默认波特率 ────────────────────────────────────────────────
    baud_row = QHBoxLayout()
    baud_label = QLabel(panel._widget.tr("默认波特率"), tab)
    baud_label.setObjectName("serialStationSettingsFieldLabel")
    baud_combo = QComboBox(tab)
    baud_combo.setObjectName("serialStationSettingsBaudrateCombo")
    for rate in _BAUDRATES:
        baud_combo.addItem(str(rate), rate)
    # 同步当前持久化值（不在列表中则追加）。
    _select_baudrate(baud_combo, settings.default_baudrate)
    baud_combo.currentIndexChanged.connect(_on_baudrate_changed)
    panel._baudrate_combo = baud_combo
    baud_row.addWidget(baud_label)
    baud_row.addWidget(baud_combo, 1)
    layout.addLayout(baud_row)

    # ── 恢复默认按钮 ──────────────────────────────────────────────
    reset_btn = QPushButton(panel._widget.tr("恢复默认"), tab)
    reset_btn.setObjectName("serialStationSettingsResetButton")
    reset_btn.clicked.connect(_on_reset_clicked)
    panel._reset_button = reset_btn
    layout.addWidget(reset_btn)

    hint = QLabel(
        panel._widget.tr(
            "字体、动画、默认波特率等偏好会即时保存，重启后生效。"
        ),
        tab,
    )
    hint.setObjectName("serialStationSettingsFieldLabel")
    hint.setWordWrap(True)
    layout.addWidget(hint)
    layout.addStretch(1)
    return tab


def sync_advanced_tab(panel: SettingsPanel) -> None:
    """``on_enter`` 时把 SettingsManager 当前值同步到控件（避免外部修改后失同步）。"""

    settings = SettingsManager.instance().get()
    if panel._font_spin is not None:
        panel._font_spin.blockSignals(True)
        panel._font_spin.setValue(settings.font_point)
        panel._font_spin.blockSignals(False)
    if panel._animation_check is not None:
        panel._animation_check.blockSignals(True)
        panel._animation_check.setChecked(settings.animation_enabled)
        panel._animation_check.blockSignals(False)
    if panel._baudrate_combo is not None:
        panel._baudrate_combo.blockSignals(True)
        _select_baudrate(panel._baudrate_combo, settings.default_baudrate)
        panel._baudrate_combo.blockSignals(False)


# ── 内部回调 ────────────────────────────────────────────────────────
def _on_font_changed(value: int) -> None:
    SettingsManager.instance().update(font_point=int(value))


def _on_animation_toggled(checked: bool) -> None:
    SettingsManager.instance().update(animation_enabled=bool(checked))


def _on_baudrate_changed(_index: int) -> None:
    combo = _find_active_combo()
    if combo is None:
        return
    rate = combo.currentData()
    if isinstance(rate, int):
        SettingsManager.instance().update(default_baudrate=rate)


def _on_reset_clicked() -> None:
    """恢复全部默认值（通过 SettingsManager.reset），并刷新控件显示。"""

    SettingsManager.instance().reset()
    # 触发全局控件同步：通过查询当前 panel 的 _widget 重新 enter。
    # 简化做法：直接拿单例面板（若已注册）调 sync_advanced_tab；否则依赖
    # 下次 on_enter 自动同步。
    panel = _find_active_panel()
    if panel is not None:
        sync_advanced_tab(panel)


def _select_baudrate(combo: QComboBox, rate: int) -> None:
    """选中 combo 中 data==rate 的项；不存在则在末尾追加。"""

    idx = combo.findData(rate)
    if idx >= 0:
        combo.setCurrentIndex(idx)
    else:
        combo.addItem(str(rate), rate)
        combo.setCurrentIndex(combo.count() - 1)


def _find_active_combo() -> QComboBox | None:
    """从当前 QApplication 焦点控件树找 baudrate combo（信号回调用）。

    信号绑定没传 combo 引用以避免循环引用；运行时通过全局查找更稳。
    """

    from PyQt6.QtWidgets import QApplication

    app = QApplication.instance()
    if app is None:
        return None
    for w in app.topLevelWidgets():
        combo = w.findChild(QComboBox, "serialStationSettingsBaudrateCombo")
        if combo is not None:
            return combo
    return None


def _find_active_panel() -> SettingsPanel | None:
    """从顶层窗口反查 SettingsPanel 实例（reset 按钮回调后刷新控件用）。"""

    from PyQt6.QtWidgets import QApplication

    app = QApplication.instance()
    if app is None:
        return None
    for w in app.topLevelWidgets():
        anchor = w.findChild(QWidget, "serialStationSettingsPanel")
        if anchor is not None:
            # SettingsPanel 把自身存到 _widget 属性；anchor 就是它。
            panel_attr = getattr(anchor, "_settings_panel_ref", None)
            if panel_attr is not None:
                return panel_attr
    return None
