"""仪表盘控件库面板（拖拽源）。

列出可用控件，每个可拖拽到画布。拖拽时携带控件类型 mime 数据。
"""

from __future__ import annotations

from PyQt6.QtCore import QMimeData, Qt
from PyQt6.QtGui import QDrag, QIcon
from PyQt6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.ui.dashboard.factory import WIDGET_CATALOG
from embeddebug.serial_station.ui.icons import IconManager
from embeddebug.serial_station.ui.theme import palette as P

MIME_TYPE = "application/x-serialstation-widget-type"


class WidgetPaletteButton(QPushButton):
    """可拖拽的控件库按钮。"""

    def __init__(self, widget_type: str, label: str, icon_name: str, parent: QWidget | None = None) -> None:
        super().__init__(label, parent)
        self.setObjectName("serialStationPaletteButton")
        self._widget_type = widget_type
        icon = IconManager().icon(icon_name, color=P.TEXT_SECONDARY)
        if not icon.isNull():
            self.setIcon(icon)
        self.setToolTip(f"Drag to canvas to add {label}")

    def mousePressEvent(self, event: object) -> None:
        if event.button() == Qt.MouseButton.LeftButton:
            self._start_drag(event)
        super().mousePressEvent(event)

    def _start_drag(self, event: object) -> None:
        drag = QDrag(self)
        mime = QMimeData()
        mime.setData(MIME_TYPE, self._widget_type.encode("utf-8"))
        mime.setText(self._widget_type)
        drag.setMimeData(mime)
        # 拖拽时显示控件类型文本。
        pixmap_icon: QIcon = self.icon()
        if not pixmap_icon.isNull():
            drag.setPixmap(pixmap_icon.pixmap(32, 32))
        drag.exec(Qt.DropAction.CopyAction)

    @property
    def widget_type(self) -> str:
        return self._widget_type


class WidgetPalette(QFrame):
    """控件库面板：列出可拖拽的控件。"""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationWidgetPalette")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        title = QLabel("Widgets", self)
        title.setObjectName("serialStationPaletteTitle")
        title.setStyleSheet(f"color: {P.TEXT_PRIMARY}; font-weight: 600; font-size: 13px;")
        layout.addWidget(title)

        for entry in WIDGET_CATALOG:
            btn = WidgetPaletteButton(
                widget_type=entry["type"],
                label=entry["label"],
                icon_name=entry["icon"],
                parent=self,
            )
            layout.addWidget(btn)

        layout.addStretch(1)

    def palette_buttons(self) -> list[WidgetPaletteButton]:
        return self.findChildren(WidgetPaletteButton)
