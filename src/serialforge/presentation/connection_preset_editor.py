"""Small metadata editor for user-owned connection presets."""

from __future__ import annotations

from dataclasses import dataclass
from uuid import uuid4

from .connection_presets import (
    MAX_PRESET_KEY_LENGTH,
    MAX_PRESET_TEXT_LENGTH,
    ConnectionPreset,
)
from .dialog_transition import start_dialog_transition, stop_dialog_transition
from .qt import (
    QDialog,
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)
from .theme import apply_theme, theme_key_for_widget


@dataclass(frozen=True, slots=True)
class ConnectionPresetMetadata:
    """User-facing metadata; transport values come from the active form."""

    key: str
    label: str
    description: str


def _new_custom_key() -> str:
    """New custom key."""
    return f"custom-{uuid4().hex}"[:MAX_PRESET_KEY_LENGTH]


def _field_label(text: str) -> QLabel:
    """Create a compact secondary label for the metadata form."""

    label = QLabel(text)
    label.setProperty("role", "muted")
    label.setMinimumWidth(48)
    return label


class ConnectionPresetEditorDialog(QDialog):
    """Collect only a name and note; never expose secrets or endpoint identity."""

    def __init__(
        self,
        parent: QWidget | None = None,
        *,
        existing: ConnectionPreset | None = None,
    ) -> None:
        super().__init__(parent)
        apply_theme(self, theme_key_for_widget(parent))
        self.setWindowTitle("编辑自定义连接配置" if existing else "保存自定义连接配置")
        self.setMinimumWidth(430)
        self.setModal(True)
        self._key = existing.key if existing is not None else _new_custom_key()

        root = QVBoxLayout(self)
        root.setContentsMargins(18, 18, 18, 18)
        root.setSpacing(10)
        intro = QLabel(
            "只保存当前连接页的标准化选项；不会保存端口句柄、BLE 设备、密钥，"
            "也不会自动连接。"
        )
        intro.setProperty("role", "muted")
        intro.setWordWrap(True)
        intro.setAccessibleName("自定义连接配置说明")
        root.addWidget(intro)

        fields = QFrame(self)
        fields.setObjectName("presetMetadataFields")
        fields.setProperty("role", "surface")
        fields.setAccessibleName("自定义连接配置元数据")
        fields.setAccessibleDescription("编辑自定义连接配置的名称和备注；不会保存设备身份或密钥。")
        fields_layout = QVBoxLayout(fields)
        fields_layout.setContentsMargins(10, 10, 10, 10)
        fields_layout.setSpacing(8)

        name_row = QHBoxLayout()
        name_row.addWidget(_field_label("名称"))
        self._label = QLineEdit(existing.label if existing else "")
        self._label.setMaxLength(MAX_PRESET_TEXT_LENGTH)
        self._label.setPlaceholderText("例如：开发板 UART / 本地 RTT")
        self._label.setAccessibleName("自定义连接配置名称")
        name_row.addWidget(self._label, stretch=1)
        fields_layout.addLayout(name_row)

        description_row = QHBoxLayout()
        description_row.addWidget(_field_label("备注"))
        self._description = QLineEdit(existing.description if existing else "")
        self._description.setMaxLength(MAX_PRESET_TEXT_LENGTH)
        self._description.setPlaceholderText("可选；留空会使用默认说明")
        self._description.setAccessibleName("自定义连接配置备注")
        description_row.addWidget(self._description, stretch=1)
        fields_layout.addLayout(description_row)
        root.addWidget(fields)

        self._error = QLabel()
        self._error.setProperty("role", "error")
        self._error.setWordWrap(True)
        self._error.setAccessibleName("自定义连接配置编辑错误")
        root.addWidget(self._error)

        buttons = QHBoxLayout()
        buttons.addStretch()
        cancel = QPushButton("取消")
        cancel.setAccessibleName("取消自定义连接配置编辑")
        cancel.setToolTip("取消自定义连接配置编辑；不会保存本次修改。")
        cancel.setAccessibleDescription("取消自定义连接配置编辑；不会保存本次修改。")
        cancel.clicked.connect(self.reject)
        buttons.addWidget(cancel)
        save = QPushButton("保存")
        save.setObjectName("primaryButton")
        save.setDefault(True)
        save.setAccessibleName("保存自定义连接配置元数据")
        save.setToolTip(
            "保存自定义连接配置元数据；只保存名称和备注，不保存密钥或设备句柄，也不会自动连接。"
        )
        save.setAccessibleDescription(
            "保存自定义连接配置元数据；只保存名称和备注，不保存密钥或设备句柄，也不会自动连接。"
        )
        save.clicked.connect(self._accept_metadata)
        buttons.addWidget(save)
        root.addLayout(buttons)
        self._label.setFocus()

    def showEvent(self, event: object) -> None:
        """Showevent."""
        super().showEvent(event)  # type: ignore[arg-type]
        start_dialog_transition(self)

    def hideEvent(self, event: object) -> None:
        """Hideevent."""
        stop_dialog_transition(self)
        super().hideEvent(event)  # type: ignore[arg-type]

    def metadata(self) -> ConnectionPresetMetadata:
        """Return bounded, normalized metadata for the preset controller."""

        label = self._label.text().strip()
        if not label:
            raise ValueError("名称不能为空")
        description = self._description.text().strip() or f"自定义连接配置 · {label}"
        if len(description) > MAX_PRESET_TEXT_LENGTH:
            raise ValueError(f"备注不能超过 {MAX_PRESET_TEXT_LENGTH} 个字符")
        return ConnectionPresetMetadata(
            key=self._key,
            label=label,
            description=description,
        )

    def _accept_metadata(self) -> None:
        """Accept metadata."""
        try:
            self.metadata()
        except ValueError as exc:
            self._error.setText(str(exc))
            self._error.setAccessibleDescription(str(exc))
            return
        self.accept()


__all__ = ["ConnectionPresetEditorDialog", "ConnectionPresetMetadata"]
