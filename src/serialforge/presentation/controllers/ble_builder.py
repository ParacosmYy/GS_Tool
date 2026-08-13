"""BLE GATT connection surface composition with readable field groups."""

from __future__ import annotations

from ...domain.models import BleGattWriteMode
from ..action_surface import ActionRailButton, BusyActionButton
from ..connection_bindings import BleControlBindings
from ..form_fields import build_field_row, build_labeled_field
from ..qt import QCheckBox, QComboBox, QLabel, QLineEdit, QSizePolicy, QVBoxLayout, QWidget
from .composition import timeout_combo


def _section_label(text: str) -> QLabel:
    """Create a stable sub-section marker without absorbing spare height."""

    label = QLabel(text)
    label.setProperty("role", "section")
    label.setSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
    label.setAccessibleName(text)
    return label


def _bounded_combo(combo: QComboBox, *, minimum: int, maximum: int) -> None:
    """Bound long BLE labels while allowing each semantic field to expand."""

    combo.setSizeAdjustPolicy(QComboBox.SizeAdjustPolicy.AdjustToMinimumContentsLengthWithIcon)
    combo.setMinimumContentsLength(8)
    combo.setMinimumWidth(minimum)
    combo.setMaximumWidth(maximum)


def build_ble_panel(window) -> BleControlBindings:
    """Build the BLE form without changing its runtime or binding contract."""

    panel = QWidget(window)
    panel.setProperty("role", "surface")
    layout = QVBoxLayout(panel)
    layout.setContentsMargins(0, 0, 0, 0)
    layout.setSpacing(10)

    layout.addWidget(_section_label("扫描与筛选"))
    scan_timeout = timeout_combo(5.0, 60.0, minimum=0.001)
    scan_timeout.setAccessibleName("BLE 扫描超时")
    scan_timeout.setToolTip("选择 BLE 扫描持续时间，扫描期间不会自动连接设备。")
    scan_timeout.setAccessibleDescription("选择 BLE 扫描持续时间，单位为秒；扫描不会自动连接设备。")
    scan_button = BusyActionButton("扫描 BLE")
    scan_button.setAccessibleName("扫描 BLE 设备")
    scan_button.setToolTip("按当前名称和服务过滤条件扫描附近 BLE 设备。")
    scan_button.clicked.connect(window._scan_ble)
    layout.addLayout(
        build_field_row(
            (
                build_labeled_field("扫描超时", scan_timeout),
                build_labeled_field("扫描动作", scan_button),
            )
        )
    )

    name_filter = QLineEdit()
    name_filter.setAccessibleName("BLE 名称过滤")
    name_filter.setPlaceholderText("可选：名称包含，例如 Sensor")
    name_filter.setToolTip("可选：只显示名称包含该文本的 BLE 设备。")
    name_filter.setAccessibleDescription("可选地按设备名称包含关系过滤 BLE 扫描结果。")
    service_filter = QLineEdit()
    service_filter.setAccessibleName("BLE Service UUID 过滤")
    service_filter.setPlaceholderText("可选：多个 UUID 用逗号或换行分隔")
    service_filter.setToolTip("可选：按 Service UUID 过滤；多个 UUID 使用逗号或换行分隔。")
    service_filter.setAccessibleDescription(
        "可选地按 BLE Service UUID 过滤扫描结果，多个 UUID 使用逗号或换行分隔。"
    )
    layout.addLayout(
        build_field_row(
            (
                build_labeled_field("名称过滤", name_filter),
                build_labeled_field("Service UUID 过滤", service_filter),
            )
        )
    )

    layout.addWidget(_section_label("设备发现与连接"))
    device_combo = QComboBox()
    device_combo.setAccessibleName("BLE 设备")
    device_combo.setPlaceholderText("先扫描并选择 BLE 设备")
    device_combo.setToolTip("选择要连接的 BLE 设备；扫描结果不会自动替换当前选择。")
    device_combo.setAccessibleDescription(
        "从扫描结果中选择要连接的 BLE 设备；设备不会自动连接或自动替换。"
    )
    _bounded_combo(device_combo, minimum=160, maximum=520)
    device_combo.currentIndexChanged.connect(window._on_ble_device_changed)
    layout.addWidget(build_labeled_field("设备", device_combo))

    connect_timeout = timeout_combo(30.0, 60.0, minimum=0.001)
    connect_timeout.setAccessibleName("BLE 连接超时")
    connect_timeout.setToolTip("选择 BLE 设备连接等待的最长时间。")
    connect_timeout.setAccessibleDescription("选择 BLE 设备连接等待的最长时间，单位为秒。")
    pair = QCheckBox("连接时配对")
    pair.setAccessibleName("BLE 连接时配对")
    pair.setToolTip("连接 BLE 设备前请求系统执行配对。")
    pair.setAccessibleDescription("连接 BLE 设备时请求系统执行配对；设备不支持时由系统处理。")
    cached_services = QComboBox()
    cached_services.setAccessibleName("BLE 服务缓存策略")
    cached_services.addItem("默认", None)
    cached_services.addItem("使用缓存", True)
    cached_services.addItem("不使用缓存", False)
    cached_services.setEditable(False)
    cached_services.setToolTip("选择 BLE GATT 服务发现是否使用系统缓存。")
    cached_services.setAccessibleDescription("选择 BLE GATT 服务发现的缓存策略。")
    layout.addLayout(
        build_field_row(
            (
                build_labeled_field("连接超时", connect_timeout),
                build_labeled_field("连接选项", pair),
                build_labeled_field("服务缓存", cached_services),
            )
        )
    )

    layout.addWidget(_section_label("GATT 特征与收发"))
    characteristic_combo = QComboBox()
    characteristic_combo.setAccessibleName("BLE GATT 特征")
    characteristic_combo.setPlaceholderText("连接后选择特征")
    characteristic_combo.setToolTip("连接并发现服务后，选择用于读写或订阅的 GATT 特征。")
    characteristic_combo.setAccessibleDescription(
        "连接并发现服务后，从 GATT 特征列表中选择读写或订阅目标。"
    )
    _bounded_combo(characteristic_combo, minimum=160, maximum=520)
    characteristic_combo.currentIndexChanged.connect(window._on_ble_characteristic_changed)
    layout.addWidget(build_labeled_field("特征", characteristic_combo))

    characteristic_properties = QLabel("未连接")
    characteristic_properties.setProperty("role", "muted")
    characteristic_properties.setWordWrap(True)
    layout.addWidget(build_labeled_field("特征属性", characteristic_properties))

    read_button = ActionRailButton("读取特征")
    read_button.setAccessibleName("读取 BLE 特征")
    read_button.setToolTip("读取当前选中 GATT 特征的一次性数据。")
    read_button.clicked.connect(window._read_ble_characteristic)
    notify_check = QCheckBox("订阅通知/指示")
    notify_check.setAccessibleName("订阅 BLE 通知或指示")
    notify_check.setToolTip("切换当前 GATT 特征的通知或指示订阅。")
    notify_check.setAccessibleDescription("切换当前 GATT 特征的通知或指示订阅状态。")
    notify_check.toggled.connect(window._toggle_ble_notifications)
    write_mode = QComboBox()
    write_mode.setAccessibleName("BLE 写入模式")
    write_mode.setEditable(False)
    write_mode.addItem("写入 · 等待响应", BleGattWriteMode.WITH_RESPONSE)
    write_mode.addItem("写入 · 不等待响应", BleGattWriteMode.WITHOUT_RESPONSE)
    _bounded_combo(write_mode, minimum=150, maximum=300)
    write_mode.setToolTip("选择 GATT 写入是否等待设备响应。")
    write_mode.setAccessibleDescription("选择 BLE GATT 写入模式：等待响应，或不等待响应。")
    write_mode.currentIndexChanged.connect(window._on_ble_write_mode_changed)
    layout.addLayout(
        build_field_row(
            (
                build_labeled_field("读取动作", read_button),
                build_labeled_field("通知/指示", notify_check),
                build_labeled_field("写入模式", write_mode),
            )
        )
    )

    hint = QLabel(
        "BLE 只做单设备 Central：手动扫描、显式连接；不会自动重连，也不会自动替换设备。"
    )
    hint.setProperty("role", "muted")
    hint.setWordWrap(True)
    hint.setSizePolicy(QSizePolicy.Policy.Ignored, QSizePolicy.Policy.Fixed)
    hint.setAccessibleName("BLE 连接边界")
    hint.setAccessibleDescription(hint.text())
    layout.addWidget(hint)

    title = _section_label("BLE GATT 设备")
    window._ble_panel = panel
    window._ble_title = title
    window._ble_scan_timeout = scan_timeout
    window._ble_scan_button = scan_button
    window._ble_name_filter = name_filter
    window._ble_service_filter = service_filter
    window._ble_device_combo = device_combo
    window._ble_connect_timeout = connect_timeout
    window._ble_pair = pair
    window._ble_cached_services = cached_services
    window._ble_characteristic_combo = characteristic_combo
    window._ble_characteristic_properties = characteristic_properties
    window._ble_read_button = read_button
    window._ble_notify_check = notify_check
    window._ble_write_mode = write_mode
    return BleControlBindings(
        panel=panel,
        title=title,
        hint=hint,
        scan_timeout=scan_timeout,
        scan_button=scan_button,
        name_filter=name_filter,
        service_filter=service_filter,
        device_combo=device_combo,
        connect_timeout=connect_timeout,
        pair=pair,
        cached_services=cached_services,
        characteristic_combo=characteristic_combo,
        characteristic_properties=characteristic_properties,
        read_button=read_button,
        notify_check=notify_check,
        write_mode=write_mode,
    )


__all__ = ["build_ble_panel"]
