"""BLE GATT discovery, capability, and notification controller.

The controller owns BLE-specific presentation state and delegates transport
actions to the view-model port; it never reaches into infrastructure handles.
"""

from __future__ import annotations

from ...domain.errors import ConfigurationError
from ...domain.models import (
    BleGattCharacteristic,
    BleGattCharacteristicRef,
    BleGattWriteMode,
    SessionState,
)
from ..ble_selection import selected_ble_write_mode, supported_ble_write_modes
from ..connection_bindings import ble_bindings_for
from ..qt import Qt
from .connection_runtime import build_ble_discovery_config


def scan_ble(window) -> None:
    try:
        window._view_model.scan_ble(build_ble_discovery_config(window))
    except ConfigurationError as exc:
        window._show_local_error(f"BLE 扫描配置无效 · {exc}")
    window._refresh_connection_controls(window._view_model.state)


def read_ble_characteristic(window) -> None:
    ble = ble_bindings_for(window)
    if ble is None:
        window._show_local_error("BLE 读取失败 · BLE 控件尚未初始化。")
        return
    selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    if not isinstance(selected, BleGattCharacteristic):
        window._show_local_error("请先选择 BLE GATT 特征。")
        return
    window._view_model.read_ble_characteristic(selected.ref)


def toggle_ble_notifications(window, enabled: bool) -> None:
    ble = ble_bindings_for(window)
    if ble is None:
        return
    selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    if not isinstance(selected, BleGattCharacteristic):
        return
    window._request_ble_notifications(selected.ref, enabled)


def request_ble_notifications(
    window,
    characteristic: BleGattCharacteristicRef,
    enabled: bool,
) -> None:
    if window._view_model.state is not SessionState.OPEN:
        window._show_local_error("当前 BLE 会话未处于可订阅状态。")
        window._refresh_connection_controls(window._view_model.state)
        return
    window._ble_notification_pending = (characteristic, enabled)
    window._ble_notification_ref = characteristic if enabled else None
    window._ble_notification_timer.start()
    previous_error = window._view_model.error_info
    window._view_model.set_ble_notifications(characteristic, enabled)
    if window._ble_notification_pending is not None:
        current_error = window._view_model.error_info
        if current_error is not None and current_error is not previous_error:
            window._rollback_ble_notification()
    window._refresh_connection_controls(window._view_model.state)


def on_ble_notification_timeout(window) -> None:
    if window._closing:
        window._ble_notification_pending = None
        window._ble_notification_ref = None
        return
    if window._ble_notification_pending is None:
        return
    window._rollback_ble_notification()
    window._show_local_error("BLE 通知订阅确认超时 · 已恢复控件状态，请重试。")


def on_ble_device_changed(window, _index: int = -1) -> None:
    window._update_connection_context()
    window._refresh_connection_controls(window._view_model.state)


def on_ble_characteristic_changed(window, _index: int = -1) -> None:
    ble = ble_bindings_for(window)
    if ble is None:
        return
    selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    selected_ref = selected.ref if isinstance(selected, BleGattCharacteristic) else None
    previous_ref = window._ble_notification_ref
    preserve_subscription = previous_ref is not None and previous_ref == selected_ref
    if (
        previous_ref is not None
        and previous_ref != selected_ref
        and window._view_model.state is SessionState.OPEN
    ):
        window._request_ble_notifications(previous_ref, False)
    if not preserve_subscription:
        window._ble_notification_ref = None
    if isinstance(selected, BleGattCharacteristic):
        ble.characteristic_properties.setText(
            f"{selected.ref.key} · {', '.join(selected.properties) or '无属性'}"
            + (
                f" · without-response 上限 {selected.max_write_without_response_size} B"
                if selected.max_write_without_response_size
                else ""
            )
        )
        normalize_ble_write_mode(window, selected)
    else:
        ble.characteristic_properties.setText("未选择特征")
    ble.characteristic_properties.setAccessibleDescription(
        ble.characteristic_properties.text()
    )
    ble.characteristic_properties.setToolTip(
        ble.characteristic_properties.accessibleDescription()
    )
    ble.notify_check.blockSignals(True)
    ble.notify_check.setChecked(preserve_subscription)
    ble.notify_check.blockSignals(False)
    window._refresh_connection_controls(window._view_model.state)


def on_ble_write_mode_changed(window, _index: int = -1) -> None:
    ble = ble_bindings_for(window)
    if ble is None:
        return
    selected = ble.characteristic_combo.currentData(Qt.ItemDataRole.UserRole)
    if isinstance(selected, BleGattCharacteristic):
        normalize_ble_write_mode(window, selected)
    window._refresh_connection_controls(window._view_model.state)


def normalize_ble_write_mode(window, characteristic: BleGattCharacteristic) -> None:
    """Keep the visible write mode usable when a GATT characteristic changes."""

    supported = supported_ble_write_modes(characteristic)
    if not supported or selected_ble_write_mode(window) in supported:
        return
    target = supported[0]
    ble = ble_bindings_for(window)
    if ble is None:
        return
    ble.write_mode.blockSignals(True)
    try:
        for index in range(ble.write_mode.count()):
            value = ble.write_mode.itemData(index, Qt.ItemDataRole.UserRole)
            try:
                normalized = (
                    value if isinstance(value, BleGattWriteMode) else BleGattWriteMode(value)
                )
            except (TypeError, ValueError):
                continue
            if normalized is target:
                ble.write_mode.setCurrentIndex(index)
                break
    finally:
        ble.write_mode.blockSignals(False)
