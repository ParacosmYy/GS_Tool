"""命令面板（Ctrl+P 模糊搜索）。

VOFA+ / VS Code 风格的全局命令面板：半透明遮罩 + 居中输入框 + 模糊匹配列表。
用户输入关键词，按 Enter 执行选中命令，Esc 关闭。

设计要点：
- 命令项是 ``(标题, 回调)`` 对，由调用方注册。
- 模糊匹配按子序列评分排序（命中连续字符加分），对齐 VS Code 命令面板体验。
- 面板是 QWidget，objectName 统一带 ``serialStationCommandPalette`` 前缀，
  由 QSS 提供半透明遮罩 + 居中卡片样式。
- 不依赖 controller/core/protocol；只消费已注册的命令回调。

约束：本模块只依赖 PyQt6 + 标准库。
"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from PyQt6.QtCore import QCoreApplication, Qt, pyqtSignal
from PyQt6.QtWidgets import (
    QFrame,
    QLabel,
    QLineEdit,
    QListWidget,
    QListWidgetItem,
    QVBoxLayout,
    QWidget,
)

from embeddebug.serial_station.ui.theme import tokens as T


@dataclass(frozen=True)
class CommandItem:
    """命令面板的一条命令。"""

    title: str
    callback: Callable[[], None]
    hint: str = ""


def fuzzy_score(query: str, target: str) -> int:
    """子序列模糊匹配评分：完全匹配最高，连续命中次之，分散命中最低。

    返回 -1 表示不匹配。分数越高越优先。
    """

    if not query:
        return 0
    query_lower = query.lower()
    target_lower = target.lower()
    if query_lower in target_lower:
        # 完全子串匹配：起始位置越靠前分越高。
        base = 100 - target_lower.index(query_lower)
        return base + (10 if target_lower.startswith(query_lower) else 0)
    # 子序列匹配：逐字符扫描，连续命中加分。
    score = 0
    consecutive = 0
    ti = 0
    for _, qc in enumerate(query_lower):
        found = False
        while ti < len(target_lower):
            if target_lower[ti] == qc:
                consecutive += 1
                score += consecutive + (5 if ti == 0 else 0)
                ti += 1
                found = True
                break
            ti += 1
            consecutive = 0
        if not found:
            return -1
    return score


def rank_commands(query: str, commands: tuple[CommandItem, ...]) -> list[CommandItem]:
    """按模糊评分排序命令，过滤不匹配项。"""

    scored: list[tuple[int, CommandItem]] = []
    for cmd in commands:
        s = fuzzy_score(query, cmd.title)
        if s >= 0:
            scored.append((s, cmd))
    scored.sort(key=lambda pair: (-pair[0], pair[1].title))
    return [cmd for _, cmd in scored]


class CommandPalette(QWidget):
    """命令面板：模糊搜索 + 列表 + 执行。"""

    command_executed = pyqtSignal(str)

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("serialStationCommandPalette")
        self._commands: tuple[CommandItem, ...] = ()
        self._build_ui()
        self.hide()

    def _build_ui(self) -> None:
        layout = QVBoxLayout(self)
        layout.setContentsMargins(T.SPACING_INT_2XL, T.SPACING_INT_2XL, T.SPACING_INT_2XL, T.SPACING_INT_2XL)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        card = QFrame(self)
        card.setObjectName("serialStationCommandPaletteCard")
        card_layout = QVBoxLayout(card)
        card_layout.setContentsMargins(0, 0, 0, 0)
        card_layout.setSpacing(0)

        self._edit = QLineEdit(card)
        self._edit.setObjectName("serialStationCommandPaletteEdit")
        self._edit.setPlaceholderText(self.tr("Type a command…"))
        self._edit.textChanged.connect(self._rerank)
        self._edit.returnPressed.connect(self._execute_selected)

        self._list = QListWidget(card)
        self._list.setObjectName("serialStationCommandPaletteList")
        self._list.itemActivated.connect(self._execute_item)
        self._list.setFocusPolicy(Qt.FocusPolicy.NoFocus)

        self._hint = QLabel(self.tr("Enter to run · Esc to close"), card)
        self._hint.setObjectName("serialStationCommandPaletteHint")

        card_layout.addWidget(self._edit)
        card_layout.addWidget(self._list)
        card_layout.addWidget(self._hint)
        layout.addWidget(card)

    def set_commands(self, commands: tuple[CommandItem, ...]) -> None:
        """注册可用命令列表。"""

        self._commands = commands
        self._rerank("")

    def open(self) -> None:
        """打开面板并聚焦输入框。"""

        self._edit.clear()
        self._rerank("")
        self.show()
        self.raise_()
        self._edit.setFocus()
        # show()/setFocus() 后处理挂起事件，确保 hasFocus() 在调用方
        # （含单测）中立即可见，避免事件循环时序导致的焦点断言抖动。
        QCoreApplication.processEvents()

    def close(self) -> None:  # type: ignore[override]
        self.hide()

    # ── 内部 ───────────────────────────────────────────────────────
    def _rerank(self, query: str) -> None:
        ranked = rank_commands(query, self._commands)
        self._list.clear()
        for cmd in ranked:
            item = QListWidgetItem(cmd.title)
            item.setData(Qt.ItemDataRole.UserRole, cmd)
            self._list.addItem(item)
        if self._list.count() > 0:
            self._list.setCurrentRow(0)

    def _execute_selected(self) -> None:
        item = self._list.currentItem()
        self._execute_item(item)

    def _execute_item(self, item: QListWidgetItem | None) -> None:
        if item is None:
            return
        cmd = item.data(Qt.ItemDataRole.UserRole)
        if not isinstance(cmd, CommandItem):
            return
        self.command_executed.emit(cmd.title)
        cmd.callback()
        self.close()

    def keyPressEvent(self, event: object) -> None:
        key = event.key()
        if key == Qt.Key.Key_Escape:
            self.close()
            event.accept()
            return
        if key in (Qt.Key.Key_Up, Qt.Key.Key_Down):
            super().keyPressEvent(event)
            return
        super().keyPressEvent(event)
