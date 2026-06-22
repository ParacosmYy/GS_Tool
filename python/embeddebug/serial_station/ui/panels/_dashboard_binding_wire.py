"""仪表盘绑定装配 helper（Batch 49）。

把 BindingService 与 SerialWorkbenchController 的事件、canvas 的 binding spec
和 DashboardPanel 的状态串起来。本身不持有 QWidget —— 只通过 canvas.items /
 panel 的属性反射访问 widget，便于测试 mock。

职责：
- ``subscribe_controller_events``: 把 controller 的 measurement/log/error 事件
  路由到 BindingService。
- ``make_widget_update_handler``: 按 widget_type 创建 handler（用于 service
  把入站事件转发到具体 QWidget 的 setter；slider/button 返回 None，因为它们
  是出站命令控件）。
- ``register_item_binding``: 把 canvas 既有 item 的 binding spec 登记到 service。
- ``open_binding_config_for_item``: 弹出配置对话框 + 应用结果到 canvas / service。

约束：只依赖 PyQt6 + canvas API + binding_service，不直接访问 transport/protocol。
"""

from __future__ import annotations

from collections.abc import Callable

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.panels._dashboard_binding_dialog import (
    open_binding_dialog,
)


def make_widget_update_handler(
    widget: QWidget, widget_type: str
) -> Callable[[object], None] | None:
    """按 widget_type 创建 update handler；slider/button 返回 None（出站控件）。"""

    if widget_type in ("value_display", "gauge"):

        def _value_handler(value: object) -> None:
            try:
                widget.set_value(float(value))  # type: ignore[attr-defined]
            except Exception:
                pass

        return _value_handler

    if widget_type == "led":

        def _led_handler(payload: object) -> None:
            try:
                from embeddebug.serial_station.ui.controls.led import LedState

                level_map = {
                    "info": LedState.BLUE,
                    "warning": LedState.YELLOW,
                    "error": LedState.RED,
                }
                level = payload[0] if isinstance(payload, tuple) else "info"
                widget.set_state(level_map.get(level, LedState.OFF))  # type: ignore[attr-defined]
            except Exception:
                pass

        return _led_handler

    return None


def subscribe_controller_events(serial_controller, binding_service) -> None:
    """订阅 controller 的 measurement/log/error 事件，路由到 binding_service。

    任何订阅失败静默（不阻塞面板构建；controller 不存在 / 已订阅等场景）。
    """

    try:
        serial_controller.on_measurement_batch(
            lambda batch: _dispatch_measurement_batch(binding_service, batch)
        )
    except Exception:
        pass
    try:
        serial_controller.on_log_entry(
            lambda entry: binding_service.route_log("info", entry.text)
        )
    except Exception:
        pass
    try:
        serial_controller.on_error(
            lambda message: binding_service.route_log("error", message)
        )
    except Exception:
        pass


def _dispatch_measurement_batch(binding_service, batch) -> None:
    """把 ChannelBatch 的最新一行按通道分发到 route_measurement。"""

    try:
        values = batch.values
        if values is None or not hasattr(values, "shape") or values.ndim != 2:
            return
        last_row = values[-1]
        for channel, value in enumerate(last_row):
            try:
                binding_service.route_measurement(channel, float(value))
            except Exception:
                pass
    except Exception:
        pass


def register_item_binding(canvas, item_id: str, binding_service) -> bool:
    """把 canvas 上既有 item 的 binding 登记到 service（build 时恢复布局用）。

    返回是否成功（item 不存在 / 无 binding / 已是当前 spec 时返回 False）。
    """

    try:
        item = canvas.items.get(item_id)
    except Exception:
        return False
    if item is None:
        return False
    spec = item.config.get("binding")
    if not spec:
        return False
    handler = make_widget_update_handler(item.widget, item.widget_type)
    current = binding_service.get_binding(item_id)
    if current == spec:
        return False  # 已是当前绑定，跳过。
    try:
        binding_service.bind(item_id, spec, handler)
    except Exception:
        return False
    return True


def open_binding_config_for_item(panel, item_id: str) -> None:
    """弹出绑定配置对话框 + 应用结果到 canvas / binding service。

    由 DashboardPanel._configure_widget_binding 调用。
    """

    tabs = getattr(panel, "_tabs", None)
    widget_root = getattr(panel, "_widget", None)
    binding_service = getattr(panel, "_binding_service", None)
    if tabs is None or widget_root is None:
        return
    canvas = tabs.current_canvas()
    if canvas is None:
        return
    item = canvas.items.get(item_id)
    if item is None:
        return
    current_spec = item.config.get("binding")

    def _on_apply(new_spec: str) -> None:
        # 1) 更新 canvas（widget property + config 持久化）。
        canvas.set_item_binding(item_id, new_spec)
        # 2) 更新 binding service（注册 handler 用于路由）。
        if binding_service is not None:
            new_item = canvas.items.get(item_id)
            if new_item is not None:
                handler = make_widget_update_handler(
                    new_item.widget, new_item.widget_type
                )
                binding_service.bind(item_id, new_spec, handler)
        # 3) 状态栏反馈 + autosave。
        _notify_status(panel, widget_root, new_spec)
        _trigger_autosave(panel)

    open_binding_dialog(
        widget=item.widget,
        widget_type=item.widget_type,
        current_binding=current_spec,
        parent=widget_root,
        on_apply=_on_apply,
    )


def _notify_status(panel, widget_root, spec: str) -> None:
    """更新状态栏文字（失败静默）。"""

    try:
        status = getattr(panel, "_status", None)
        if status is not None:
            status.setText(widget_root.tr("已绑定：{spec}").format(spec=spec))
    except Exception:
        pass


def _trigger_autosave(panel) -> None:
    """binding 变更后触发 autosave（失败静默）。"""

    try:
        autosave = getattr(panel, "_autosave_layout", None)
        if callable(autosave):
            autosave()
    except Exception:
        pass
