"""仪表盘画布（对齐 VOFA+ 拖拽式 GUI）。

可放置波形/LED/滑块/仪表盘/数值显示等控件的网格画布。
支持拖拽放置（从控件库拖入）、移动、删除、布局持久化（JSON）。

设计要点：
- 画布是 QWidget，接受 drop，按网格吸附放置控件。
- 每个放置的控件是一个 ``DashboardItem``（包装实例 + 位置 + 元数据）。
- 布局可序列化为 JSON，启动恢复。
- 双击控件全屏，再双击恢复。

约束：本模块只依赖 PyQt6 + controls + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path

from PyQt6.QtCore import QPoint, QRect, Qt, pyqtSignal
from PyQt6.QtGui import QColor, QPainter, QPen
from PyQt6.QtWidgets import QFrame, QWidget

from embeddebug.serial_station.ui.theme import palette as P

GRID_SIZE = 20  # 网格吸附步长（像素）

# 支持的控件类型（与 dashboard_factory 对齐）。
SUPPORTED_WIDGET_TYPES: tuple[str, ...] = (
    "led", "slider", "button", "gauge", "value_display",
)


@dataclass
class DashboardItem:
    """画布上一个已放置的控件实例。"""

    widget_type: str
    widget: QWidget
    geometry: QRect
    config: dict = field(default_factory=dict)


def snap_to_grid(value: int, grid: int = GRID_SIZE) -> int:
    """把坐标吸附到网格。"""

    return round(value / grid) * grid


class DashboardCanvas(QFrame):
    """仪表盘画布：接受拖拽放置控件。"""

    item_added = pyqtSignal(str)
    item_removed = pyqtSignal(str)

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationDashboardCanvas")
        self.setAcceptDrops(True)
        self._items: dict[str, DashboardItem] = {}
        self._counter = 0
        # Batch 36: 网格可见性（默认显示，辅助对齐；可经 set_show_grid 切换）。
        self._show_grid = True
        self.setStyleSheet(f"background-color: {P.BG_APP}; border: 1px dashed {P.BORDER};")

    def set_show_grid(self, show: bool) -> None:
        """切换网格可见性（Batch 36）。"""

        self._show_grid = bool(show)
        self.update()

    @property
    def show_grid(self) -> bool:
        """当前网格是否可见。"""

        return self._show_grid

    @property
    def items(self) -> dict[str, DashboardItem]:
        return dict(self._items)

    def add_widget_at(self, widget_type: str, position: QPoint, config: dict | None = None) -> str:
        """在指定位置放置一个控件，返回 item id。"""

        from embeddebug.serial_station.ui.dashboard.factory import create_widget

        self._counter += 1
        item_id = f"{widget_type}_{self._counter}"
        widget = create_widget(widget_type, self)
        widget.setObjectName(f"serialStationDashboardItem_{item_id}")
        snapped_x = snap_to_grid(position.x())
        snapped_y = snap_to_grid(position.y())
        width = config.get("width", 160) if config else 160
        height = config.get("height", 80) if config else 80
        geometry = QRect(snapped_x, snapped_y, width, height)
        widget.setGeometry(geometry)
        widget.show()
        try:
            from embeddebug.serial_station.ui.animations.bounce_path import BouncePathAnimation
            BouncePathAnimation.drop_in(widget).start()
        except Exception:
            pass
        self._items[item_id] = DashboardItem(
            widget_type=widget_type,
            widget=widget,
            geometry=geometry,
            config=config or {},
        )
        self.item_added.emit(item_id)
        return item_id

    def remove_item(self, item_id: str) -> bool:
        item = self._items.pop(item_id, None)
        if item is None:
            return False
        item.widget.setParent(None)
        item.widget.deleteLater()
        self.item_removed.emit(item_id)
        return True

    def clear(self) -> None:
        for item_id in list(self._items):
            self.remove_item(item_id)

    def to_layout_dict(self) -> dict:
        """序列化画布布局为 JSON 兼容 dict。"""

        return {
            "items": [
                {
                    "id": item_id,
                    "type": item.widget_type,
                    "x": item.geometry.x(),
                    "y": item.geometry.y(),
                    "width": item.geometry.width(),
                    "height": item.geometry.height(),
                    "config": item.config,
                }
                for item_id, item in self._items.items()
            ]
        }

    def save_layout(self, path: str | Path) -> None:
        """保存布局到 JSON 文件。"""

        Path(path).write_text(
            json.dumps(self.to_layout_dict(), indent=2, ensure_ascii=False),
            encoding="utf-8",
        )

    def load_layout(self, path: str | Path) -> int:
        """从 JSON 文件加载布局，返回恢复的控件数。"""

        data = json.loads(Path(path).read_text(encoding="utf-8"))
        self.clear()
        count = 0
        for entry in data.get("items", []):
            config = dict(entry.get("config", {}))
            config["width"] = entry.get("width", 160)
            config["height"] = entry.get("height", 80)
            self.add_widget_at(
                entry["type"],
                QPoint(entry["x"], entry["y"]),
                config,
            )
            count += 1
        return count

    # ── 拖拽事件 ───────────────────────────────────────────────────
    def dragEnterEvent(self, event: object) -> None:
        if event.mimeData().hasFormat("application/x-serialstation-widget-type"):
            event.acceptProposedAction()
        else:
            super().dragEnterEvent(event)

    def dropEvent(self, event: object) -> None:
        mime = event.mimeData()
        widget_type = bytes(mime.data("application/x-serialstation-widget-type")).decode("utf-8")
        if widget_type in SUPPORTED_WIDGET_TYPES:
            pos = event.position().toPoint() if hasattr(event, "position") else event.pos()
            self.add_widget_at(widget_type, pos)
            event.acceptProposedAction()
        else:
            super().dropEvent(event)

    # ── 绘制 ────────────────────────────────────────────────────────
    def paintEvent(self, event: object) -> None:
        """绘制背景 + 可选网格（Batch 36）。

        网格用 BORDER 色低 alpha 画 GRID_SIZE 步长的交叉点（小圆点），
        辅助控件对齐（与 snap_to_grid 一致）。_show_grid=False 时不画。
        """

        super().paintEvent(event)
        if not self._show_grid:
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        grid_color = QColor(P.BORDER)
        grid_color.setAlpha(90)
        painter.setPen(QPen(grid_color, 1))
        w = self.width()
        h = self.height()
        # 画网格交叉点（1px 点），步长 GRID_SIZE。
        for y in range(GRID_SIZE, h, GRID_SIZE):
            for x in range(GRID_SIZE, w, GRID_SIZE):
                painter.drawPoint(x, y)
        # Batch 47: 空画布提示（UI 审计 #2 — 画布为空时给拖拽指引）。
        if not self._items:
            hint_color = QColor(P.TEXT_MUTED)
            hint_color.setAlpha(120)
            painter.setPen(hint_color)
            font = painter.font()
            font.setPointSize(12)
            painter.setFont(font)
            painter.drawText(
                self.rect(),
                int(Qt.AlignmentFlag.AlignCenter),
                self.tr("从右侧控件库拖入 LED / 滑块 / 仪表到画布"),
            )
