"""仪表盘画布（对齐 VOFA+ 拖拽式 GUI）。

可放置波形/LED/滑块/仪表盘/数值显示等控件的网格画布。
支持拖拽放置（从控件库拖入）、移动、删除、布局持久化（JSON）。

Batch 49：``add_widget_at`` / ``set_item_binding`` / ``to_layout_dict`` /
``load_layout`` 全部支持 binding spec 字段，与 binding service 对齐。

约束：本模块只依赖 PyQt6 + controls + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path

from PyQt6.QtCore import QPoint, QRect, pyqtSignal
from PyQt6.QtGui import QColor, QPainter, QPen
from PyQt6.QtWidgets import QFrame, QWidget

from embeddebug.serial_station.ui.theme import palette as P

GRID_SIZE = 20  # 网格吸附步长（像素）

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
        self._show_grid = True
        self.setStyleSheet(f"background-color: {P.BG_APP}; border: 1px dashed {P.BORDER};")
        # Batch 49-4: 画布空态占位（图标 + 标题 + 描述，替代 paintEvent 单行文字提示）。
        # 实现在 _empty_state_overlay 模块（保持本文件 ≤ 300 行）。
        from embeddebug.serial_station.ui.dashboard._empty_state_overlay import (
            build_canvas_empty_state,
        )

        self._empty_state = build_canvas_empty_state(self)
        self._empty_state.show_with_fade()

    def set_show_grid(self, show: bool) -> None:
        self._show_grid = bool(show)
        self.update()

    @property
    def show_grid(self) -> bool:
        return self._show_grid

    @property
    def items(self) -> dict[str, DashboardItem]:
        return dict(self._items)

    def add_widget_at(
        self,
        widget_type: str,
        position: QPoint,
        config: dict | None = None,
        binding: str | None = None,
    ) -> str:
        """在指定位置放置一个控件，返回 item id。

        Batch 49：``binding`` 为可选的 binding spec 字符串，保存到
        ``DashboardItem.config["binding"]`` 并通过 factory 应用到 widget。
        """

        from embeddebug.serial_station.ui.dashboard.factory import (
            create_bound_widget,
        )

        self._counter += 1
        item_id = f"{widget_type}_{self._counter}"
        effective_binding = binding or (config.get("binding") if config else None)
        widget = create_bound_widget(widget_type, effective_binding, self)
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
        merged_config: dict = dict(config or {})
        if effective_binding:
            merged_config["binding"] = effective_binding
        self._items[item_id] = DashboardItem(
            widget_type=widget_type,
            widget=widget,
            geometry=geometry,
            config=merged_config,
        )
        self.item_added.emit(item_id)
        # Batch 49-4: 首个控件放置后淡出空态占位（避免重叠）。
        if len(self._items) == 1:
            self._empty_state.hide_with_fade()
        return item_id

    def remove_item(self, item_id: str) -> bool:
        item = self._items.pop(item_id, None)
        if item is None:
            return False
        item.widget.setParent(None)
        item.widget.deleteLater()
        self.item_removed.emit(item_id)
        # Batch 49-4: 删除最后一个控件后淡入空态占位（可恢复，对齐铁律 5.9）。
        if not self._items:
            self._empty_state.show_with_fade()
        return True

    def set_item_binding(self, item_id: str, binding: str | None) -> bool:
        """更新既有 item 的 binding spec（Batch 49）。"""

        item = self._items.get(item_id)
        if item is None:
            return False
        from embeddebug.serial_station.ui.dashboard.factory import apply_binding

        if not binding:
            item.config.pop("binding", None)
            try:
                item.widget.setProperty("binding_spec", None)
            except Exception:
                pass
            return True
        ok = apply_binding(item.widget, item.widget_type, binding)
        if ok:
            item.config["binding"] = binding
        return ok

    def clear(self) -> None:
        for item_id in list(self._items):
            self.remove_item(item_id)

    def to_layout_dict(self) -> dict:
        """序列化画布布局为 JSON 兼容 dict。

        Batch 49：每个 item 同时输出顶层 ``binding`` 字段，便于工具直接读取。
        """

        return {
            "items": [
                {
                    "id": item_id,
                    "type": item.widget_type,
                    "x": item.geometry.x(),
                    "y": item.geometry.y(),
                    "width": item.geometry.width(),
                    "height": item.geometry.height(),
                    "binding": item.config.get("binding", ""),
                    "config": item.config,
                }
                for item_id, item in self._items.items()
            ]
        }

    def save_layout(self, path: str | Path) -> None:
        Path(path).write_text(
            json.dumps(self.to_layout_dict(), indent=2, ensure_ascii=False),
            encoding="utf-8",
        )

    def load_layout(self, path: str | Path) -> int:
        """从 JSON 文件加载布局。Batch 49：支持顶层 ``binding`` 与 config["binding"]。"""

        data = json.loads(Path(path).read_text(encoding="utf-8"))
        self.clear()
        count = 0
        for entry in data.get("items", []):
            config = dict(entry.get("config", {}))
            config["width"] = entry.get("width", 160)
            config["height"] = entry.get("height", 80)
            top_binding = entry.get("binding")
            if top_binding:
                config["binding"] = top_binding
            self.add_widget_at(
                entry["type"],
                QPoint(entry["x"], entry["y"]),
                config,
            )
            count += 1
        return count

    # ── 拖拽事件 ───────────────────────────────────────────────────
    def resizeEvent(self, event: object) -> None:
        """Batch 49-4: 画布缩放时空态占位跟随居中（覆盖层定位）。"""

        super().resizeEvent(event)
        if self._empty_state is not None:
            self._empty_state.setGeometry(self.rect())

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
        """绘制背景 + 可选网格（Batch 36）。"""

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
        for y in range(GRID_SIZE, h, GRID_SIZE):
            for x in range(GRID_SIZE, w, GRID_SIZE):
                painter.drawPoint(x, y)
        # Batch 49-4: 空画布提示已迁移到 EmptyStateWidget 覆盖层（_empty_state），
        # paintEvent 不再画单行文字，避免双重渲染。
        painter.end()
