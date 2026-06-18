"""富游标管理（对齐 VOFA+ 双击添加/拖拽/删除 + X/Y 游标）。

比 B3 的固定双游标更进一步：
- 双击波形区添加 X 游标（垂直线）。
- 拖拽移动游标。
- 右键/菜单删除游标。
- 支持 Y 游标（水平线）。
- 多游标，每个有唯一 label。

约束：本模块只依赖 PyQt6 + pyqtgraph + theme.palette，不访问 controller/transport。
"""

from __future__ import annotations

import pyqtgraph as pg
from PyQt6.QtCore import Qt

from embeddebug.serial_station.ui.theme import palette as P


def make_x_cursor(value: float, index: int) -> pg.InfiniteLine:
    """创建一条 X 游标（垂直线），可拖拽。"""

    line = pg.InfiniteLine(
        pos=value,
        angle=90,
        pen=pg.mkPen(color=P.ACCENT, width=1.4, style=Qt.PenStyle.DashLine),
        movable=True,
        label=f"X{index}: {{value:.3f}}",
        labelOpts={"position": 0.96, "color": P.ACCENT, "fill": P.BG_PANEL},
    )
    line.setObjectName("serialStationWaveformCursorX")
    return line


def make_y_cursor(value: float, index: int) -> pg.InfiniteLine:
    """创建一条 Y 游标（水平线），可拖拽。"""

    line = pg.InfiniteLine(
        pos=value,
        angle=0,
        pen=pg.mkPen(color=P.WARNING, width=1.4, style=Qt.PenStyle.DashLine),
        movable=True,
        label=f"Y{index}: {{value:.3f}}",
        labelOpts={"position": 0.02, "color": P.WARNING, "fill": P.BG_PANEL},
    )
    line.setObjectName("serialStationWaveformCursorY")
    return line


class CursorManager:
    """管理一组可增删的 X/Y 游标。"""

    def __init__(self, plot: pg.PlotWidget) -> None:
        self._plot = plot
        self._x_cursors: list[pg.InfiniteLine] = []
        self._y_cursors: list[pg.InfiniteLine] = []

    @property
    def x_cursors(self) -> tuple[pg.InfiniteLine, ...]:
        return tuple(self._x_cursors)

    @property
    def y_cursors(self) -> tuple[pg.InfiniteLine, ...]:
        return tuple(self._y_cursors)

    def add_x_cursor(self, value: float = 0.0) -> pg.InfiniteLine:
        cursor = make_x_cursor(value, len(self._x_cursors) + 1)
        self._x_cursors.append(cursor)
        self._plot.addItem(cursor)
        return cursor

    def add_y_cursor(self, value: float = 0.0) -> pg.InfiniteLine:
        cursor = make_y_cursor(value, len(self._y_cursors) + 1)
        self._y_cursors.append(cursor)
        self._plot.addItem(cursor)
        return cursor

    def remove_cursor(self, cursor: pg.InfiniteLine) -> bool:
        """删除指定游标，返回是否成功。"""

        for store in (self._x_cursors, self._y_cursors):
            if cursor in store:
                store.remove(cursor)
                self._plot.removeItem(cursor)
                return True
        return False

    def clear(self) -> None:
        for cursor in list(self._x_cursors):
            self._plot.removeItem(cursor)
        for cursor in list(self._y_cursors):
            self._plot.removeItem(cursor)
        self._x_cursors.clear()
        self._y_cursors.clear()

    def cursor_values(self) -> tuple[list[float], list[float]]:
        """返回 (X 游标值列表, Y 游标值列表)。"""

        xs = [float(c.value()) for c in self._x_cursors]
        ys = [float(c.value()) for c in self._y_cursors]
        return xs, ys
