"""UI 控件/选项/字段读写测试：serial config/port/connection 字段 + profile/command/endpoint 控件。"""
from __future__ import annotations
from embeddebug.serial_station.ui.command_history_options import populate_command_history_options
from embeddebug.serial_station.ui.endpoint_profile_controls import apply_endpoint_profile_controls
from embeddebug.serial_station.ui.profile_combo_options import select_profile_combo_value
from embeddebug.serial_station.ui.serial_config_options import (
    apply_serial_config_options, serial_config_options,
)
from embeddebug.serial_station.ui.serial_connection_fields import read_serial_connection_fields
from embeddebug.serial_station.ui.serial_port_options import (
    combo_has_serial_ports, populate_serial_port_options,
)
from embeddebug.serial_station.ui.serial_profile_controls import apply_serial_profile_controls
class _TextEdit:
    def __init__(self) -> None:
        self.value = ""
    def setText(self, text: str) -> None:
        self.value = text
class _ListCombo:
    def __init__(self) -> None:
        self.items: list[str] = []
        self.current = ""
    def addItems(self, values: list[str]) -> None:
        self.items.extend(values)
    def setCurrentText(self, text: str) -> None:
        self.current = text
class _PortCombo:
    def __init__(self) -> None:
        self.enabled = True
        self.items: list[str] = []
        self.current = ""
    def addItem(self, text: str) -> None:
        self.items.append(text)
        if not self.current:
            self.current = text
    def addItems(self, items: list[str]) -> None:
        for item in items:
            self.addItem(item)
    def clear(self) -> None:
        self.items.clear()
        self.current = ""
    def count(self) -> int:
        return len(self.items)
    def currentText(self) -> str:
        return self.current
    def setCurrentText(self, text: str) -> None:
        self.current = text
class _FindCombo:
    def __init__(self, items: list[str] | None = None) -> None:
        self.items: list[str] = list(items) if items else []
        self.current = ""
    def addItem(self, text: str) -> None:
        self.items.append(text)
    def findText(self, text: str) -> int:
        try:
            return self.items.index(text)
        except ValueError:
            return -1
    def setCurrentText(self, text: str) -> None:
        self.current = text
class _HistoryCombo:
    def __init__(self) -> None:
        self.enabled = False
        self.items: list[str] = []
        self.current = ""
        self.signal_blocks: list[bool] = []
    def addItems(self, items: list[str]) -> None:
        self.items.extend(items)
    def blockSignals(self, blocked: bool) -> None:
        self.signal_blocks.append(blocked)
    def clear(self) -> None:
        self.items.clear()
        self.current = ""
    def setCurrentText(self, text: str) -> None:
        self.current = text
    def setEnabled(self, enabled: bool) -> None:
        self.enabled = enabled
class _SerialHost:
    def __init__(self) -> None:
        self._baud_combo = _FindCombo()
        self._data_bits_combo = _FindCombo()
        self._parity_combo = _FindCombo()
        self._stop_bits_combo = _FindCombo()
        self._flow_control_combo = _FindCombo()
class _ConnCombo:
    def __init__(self, value: str) -> None:
        self.value = value
    def currentText(self) -> str:
        return self.value
class _ConnHost:
    def __init__(self) -> None:
        self._port_combo = _ConnCombo("COM7")
        self._baud_combo = _ConnCombo("115200")
        self._data_bits_combo = _ConnCombo("8")
        self._parity_combo = _ConnCombo("None")
        self._stop_bits_combo = _ConnCombo("1")
        self._flow_control_combo = _ConnCombo("Hardware")
class _TrHost:
    def tr(self, text: str) -> str:
        return f"tr:{text}"
def test_serial_config_options_describe_baud_choices_and_default():
    options = serial_config_options("baud")
    assert options.values == ("9600", "19200", "38400", "57600", "115200", "921600")
    assert options.default == "115200"
def test_serial_config_options_describe_frame_choices_and_defaults():
    assert serial_config_options("data_bits").values == ("5", "6", "7", "8")
    assert serial_config_options("data_bits").default == "8"
    assert serial_config_options("parity").values == ("None", "Even", "Odd", "Space", "Mark")
    assert serial_config_options("parity").default == "None"
    assert serial_config_options("stop_bits").values == ("1", "1.5", "2")
    assert serial_config_options("stop_bits").default == "1"
    assert serial_config_options("flow_control").values == ("None", "Hardware", "Software")
    assert serial_config_options("flow_control").default == "None"
def test_apply_serial_config_options_writes_values_and_default_to_combo():
    combo = _ListCombo()
    apply_serial_config_options(combo, "baud")
    assert combo.items == ["9600", "19200", "38400", "57600", "115200", "921600"]
    assert combo.current == "115200"
def test_populate_serial_port_options_writes_translated_empty_option():
    combo = _PortCombo()
    populate_serial_port_options(_TrHost(), combo, ports=[], current="")
    assert combo.items == ["tr:No serial ports"]
    assert not combo_has_serial_ports(_TrHost(), combo)
def test_populate_serial_port_options_keeps_current_port_when_available():
    combo = _PortCombo()
    populate_serial_port_options(_TrHost(), combo, ports=["COM1", "COM2"], current="COM2")
    assert combo.items == ["COM1", "COM2"]
    assert combo.currentText() == "COM2"
    assert combo_has_serial_ports(_TrHost(), combo)
def test_read_serial_connection_fields_converts_combo_text_values():
    fields = read_serial_connection_fields(_ConnHost())
    assert fields.port_name == "COM7"
    assert fields.baud_rate == 115200
    assert fields.data_bits == 8
    assert fields.parity == "none"
    assert fields.stop_bits == "1"
    assert fields.flow_control == "hardware"
def test_select_profile_combo_value_selects_existing_option_without_duplicate():
    combo = _FindCombo(["raw_data", "just_float"])
    select_profile_combo_value(combo, "just_float")
    assert combo.items == ["raw_data", "just_float"]
    assert combo.current == "just_float"
def test_select_profile_combo_value_adds_missing_option_before_selecting():
    combo = _FindCombo(["COM1"])
    select_profile_combo_value(combo, "COM9")
    assert combo.items == ["COM1", "COM9"]
    assert combo.current == "COM9"
def test_apply_endpoint_profile_controls_sets_host_and_port_for_matching_mode():
    host_edit = _TextEdit()
    port_edit = _TextEdit()
    apply_endpoint_profile_controls(
        host_edit, port_edit,
        transport={"mode": "tcp"}, port_name="127.0.0.1:19000", expected_mode="tcp")
    assert host_edit.value == "127.0.0.1"
    assert port_edit.value == "19000"
def test_apply_endpoint_profile_controls_ignores_non_matching_or_invalid_endpoint():
    host_edit = _TextEdit()
    port_edit = _TextEdit()
    apply_endpoint_profile_controls(
        host_edit, port_edit,
        transport={"mode": "udp"}, port_name="127.0.0.1:19000", expected_mode="tcp")
    apply_endpoint_profile_controls(
        host_edit, port_edit,
        transport={"mode": "tcp"}, port_name="COM1", expected_mode="tcp")
    assert host_edit.value == ""
    assert port_edit.value == ""
def test_populate_command_history_options_selects_latest_and_unblocks_signals():
    combo = _HistoryCombo()
    populate_command_history_options(combo, ["AT", "ATI"])
    assert combo.items == ["AT", "ATI"]
    assert combo.current == "ATI"
    assert combo.enabled
    assert combo.signal_blocks == [True, False]
def test_populate_command_history_options_disables_empty_history():
    combo = _HistoryCombo()
    combo.enabled = True
    combo.current = "AT"
    combo.items = ["AT"]
    populate_command_history_options(combo, [])
    assert combo.items == []
    assert combo.current == ""
    assert not combo.enabled
    assert combo.signal_blocks == [True, False]
def test_apply_serial_profile_controls_writes_serial_transport_values():
    host = _SerialHost()
    apply_serial_profile_controls(
        host,
        {"baudRate": 115200, "dataBits": 8, "parity": "none", "stopBits": 1, "flowControl": "none"})
    assert host._baud_combo.current == "115200"
    assert host._data_bits_combo.current == "8"
    assert host._parity_combo.current == "None"
    assert host._stop_bits_combo.current == "1"
    assert host._flow_control_combo.current == "None"
def test_apply_serial_profile_controls_ignores_missing_values():
    host = _SerialHost()
    apply_serial_profile_controls(host, {})
    assert host._baud_combo.current == ""
    assert host._data_bits_combo.current == ""
    assert host._parity_combo.current == ""
    assert host._stop_bits_combo.current == ""
    assert host._flow_control_combo.current == ""
