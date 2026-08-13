"""Read-only capability page for the future embedded tool station."""

from __future__ import annotations

from collections.abc import Iterable
from dataclasses import dataclass

from ..application.extension_capabilities import (
    ExtensionCapability,
    ExtensionCapabilityGroup,
    ExtensionCapabilityState,
    extension_capabilities,
)
from ..application.extension_station import extension_station_summary
from .embedded_station_overview import (
    EmbeddedStationOverview,
    build_embedded_station_overview,
)
from .extension_capability_detail import ExtensionCapabilityDetail
from .property_refresh import refresh_dynamic_property
from .qt import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QScrollArea,
    QSizePolicy,
    Qt,
    QVBoxLayout,
    Signal,
)
from .responsive_capability_grid import ResponsiveCapabilityGrid

_GROUPS: tuple[tuple[ExtensionCapabilityGroup, str, str], ...] = (
    (
        ExtensionCapabilityGroup.OTA_TRANSFER,
        "OTA 传输框架",
        "协议槽位独立演进；当前只展示 contract-only 边界。",
    ),
    (
        ExtensionCapabilityGroup.OTA_SECURITY,
        "OTA 安全升级框架",
        "认证加密只是安全链的一环；当前保留策略与验证端口，不执行升级。",
    ),
    (
        ExtensionCapabilityGroup.DEBUG_OUTPUT,
        "RTT / J-Link 打印框架",
        "只保留 attach-only 原始日志方向，不扩大为探针控制器。",
    ),
)
_STATE_LABELS = {
    ExtensionCapabilityState.CONTRACT_ONLY: "契约预留",
    ExtensionCapabilityState.ATTACH_ONLY: "仅附着",
}


class _CapabilityCardButton(QPushButton):
    """One full-card, keyboard-operable selector with no backend behavior."""

    focus_received = Signal()

    def focusInEvent(self, event: object) -> None:
        """Focusinevent."""
        super().focusInEvent(event)  # type: ignore[arg-type]
        self.focus_received.emit()


@dataclass(frozen=True, slots=True)
class ExtensionPanelWidgets:
    """Typed composition result for the read-only extension station page."""

    layout: QVBoxLayout
    station_overview: EmbeddedStationOverview


def build_extension_panel() -> ExtensionPanelWidgets:
    """Build an accessible page without opening any extension handle."""

    layout = QVBoxLayout()
    intro = QLabel(
        "这里是嵌入式调试工具站的扩展边界。当前页面只展示可审计的能力槽位，"
        "不会连接设备、读取密钥、启动 vendor 工具或执行 OTA；选择能力卡可查看只读详情。"
    )
    intro.setObjectName("extensionIntro")
    intro.setProperty("role", "subtle")
    intro.setWordWrap(True)
    intro.setAccessibleName("扩展工具站说明")
    intro.setAccessibleDescription(intro.text())
    layout.addWidget(intro)

    capabilities = extension_capabilities()
    station_overview = build_embedded_station_overview(extension_station_summary(capabilities))
    layout.addWidget(station_overview)
    capability_detail = ExtensionCapabilityDetail(capabilities[0])
    capability_detail.setVisible(False)
    layout.addWidget(capability_detail)
    selection_controls: list[_CapabilityCardButton] = []
    for group, title, description in _GROUPS:
        section = QLabel(title)
        section.setProperty("role", "section")
        section.setProperty("scope", "station")
        section.setAccessibleName(title)
        section.setAccessibleDescription(description)

        hint = QLabel(description)
        hint.setProperty("role", "subtle")
        hint.setWordWrap(True)

        cards: list[_CapabilityCardButton] = []
        for capability in _group_capabilities(capabilities, group):
            card = _build_capability_card(capability)
            selection_controls.append(card)
            cards.append(card)
        layout.addWidget(_build_capability_section(section, hint, cards))

    def ensure_card_visible(card: _CapabilityCardButton) -> None:
        """Keep the focused capability card inside the existing scroll owner."""

        layout.activate()
        parent = card.parentWidget()
        while parent is not None:
            if isinstance(parent, QScrollArea):
                content = parent.widget()
                if content is not None:
                    content.adjustSize()
                    content_layout = content.layout()
                    if content_layout is not None:
                        content_layout.activate()
                parent.ensureWidgetVisible(card, 12, 12)
                viewport = parent.viewport()
                top = card.mapTo(viewport, card.rect().topLeft()).y()
                bottom = card.mapTo(viewport, card.rect().bottomRight()).y()
                value = parent.verticalScrollBar().value()
                if top < 12:
                    value += top - 12
                elif bottom > viewport.height() - 12:
                    value += bottom - (viewport.height() - 12)
                if value != parent.verticalScrollBar().value():
                    parent.verticalScrollBar().setValue(value)
                return
            parent = parent.parentWidget()

    def select_capability(
        index: int,
        _checked: bool = False,
        *,
        reveal_detail: bool = True,
    ) -> None:
        """Keep card selection and the on-demand detail projection synchronized."""

        for item_index, card in enumerate(selection_controls):
            selected = item_index == index
            card.setChecked(selected)
            refresh_dynamic_property(card, "selected", selected)
        capability_detail.set_capability(capabilities[index])
        if reveal_detail:
            capability_detail.setVisible(True)
            ensure_card_visible(selection_controls[index])

    for index, card in enumerate(selection_controls):
        card.clicked.connect(
            lambda _checked=False, selected_index=index: select_capability(selected_index)
        )
        card.focus_received.connect(
            lambda selected_index=index: select_capability(selected_index)
        )
    # Do not repeat the first card in the initial viewport. The first keyboard
    # focus or click reveals the same detail surface on demand.

    footer = QLabel(
        "激活任何真实 OTA 或调试后端前，必须补齐目标型号、公开一手资料、授权、"
        "失败恢复与硬件验收证据。"
    )
    footer.setObjectName("extensionFooter")
    footer.setProperty("role", "status")
    footer.setWordWrap(True)
    footer.setAccessibleName("扩展激活前提")
    footer.setAccessibleDescription(footer.text())
    layout.addWidget(footer)
    layout.addStretch(1)
    return ExtensionPanelWidgets(layout=layout, station_overview=station_overview)


def _group_capabilities(
    capabilities: Iterable[ExtensionCapability],
    group: ExtensionCapabilityGroup,
) -> tuple[ExtensionCapability, ...]:
    """Keep grouping bounded and independent from widget construction."""

    return tuple(capability for capability in capabilities if capability.group is group)


def _build_capability_section(
    title: QLabel,
    hint: QLabel,
    cards: Iterable[_CapabilityCardButton],
) -> QFrame:
    """Give each capability group one visual rhythm and layout owner."""

    section = QFrame()
    section.setObjectName("extensionCapabilitySection")
    section.setProperty("role", "surface")
    section.setFocusPolicy(Qt.FocusPolicy.NoFocus)
    section.setSizePolicy(
        QSizePolicy.Policy.Expanding,
        QSizePolicy.Policy.Preferred,
    )
    section_layout = QVBoxLayout(section)
    section_layout.setContentsMargins(0, 10, 0, 10)
    section_layout.setSpacing(7)
    section_layout.addWidget(title)
    section_layout.addWidget(hint)
    section_layout.addWidget(ResponsiveCapabilityGrid(cards))
    return section


def _build_capability_card(capability: ExtensionCapability) -> _CapabilityCardButton:
    """Build capability card."""
    description = f"{capability.summary} {capability.boundary}"
    card = _CapabilityCardButton()
    card.setObjectName("extensionCapabilityCard")
    card.setProperty("role", "surface")
    card.setProperty("state", capability.state.value)
    card.setProperty("selected", False)
    card.setCheckable(True)
    card.setAutoDefault(False)
    card.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
    card.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
    card.setMinimumHeight(122)
    card.setAccessibleName(capability.title)
    card.setAccessibleDescription(
        f"{description} 按 Tab 聚焦，按 Enter 或空格选择并查看详情。"
    )
    card.setToolTip(f"选择 {capability.title} 查看只读契约详情")

    layout = QVBoxLayout(card)
    layout.setContentsMargins(11, 10, 11, 10)
    layout.setSpacing(6)
    title_row = QHBoxLayout()
    title_row.setContentsMargins(0, 0, 0, 0)
    title = QLabel(capability.title)
    title.setObjectName("extensionCapabilityTitle")
    title_row.addWidget(title, stretch=1)
    state = QLabel(_STATE_LABELS[capability.state])
    state.setObjectName("extensionCapabilityState")
    state.setProperty("role", "status")
    state.setProperty("state", capability.state.value)
    state.setAccessibleName(f"{capability.title} 状态")
    state.setAccessibleDescription(state.text())
    title_row.addWidget(state)
    layout.addLayout(title_row)

    summary = QLabel(capability.summary)
    summary.setProperty("role", "subtle")
    summary.setWordWrap(True)
    layout.addWidget(summary)
    boundary = QLabel(capability.boundary)
    boundary.setProperty("role", "subtle")
    boundary.setWordWrap(True)
    layout.addWidget(boundary)
    for label in (title, state, summary, boundary):
        label.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents, True)
    return card


__all__ = ["ExtensionPanelWidgets", "build_extension_panel"]
