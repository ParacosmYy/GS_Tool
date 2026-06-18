"""内置预设工程模板。"""
from __future__ import annotations
from embeddebug.serial_station.project_templates.template import ProjectTemplate

class BuiltInTemplates:
    """内置预设模板清单。"""

    @classmethod
    def all(cls) -> list[ProjectTemplate]:
        return [
            ProjectTemplate(name="UART_AT_Commands", description="UART AT 指令", transport_config={"mode": "uart", "baudrate": 115200}, protocol="raw_data", commands=["AT", "AT+GMR"]),
            ProjectTemplate(name="JustFloat_Waveform", description="JustFloat 波形", transport_config={"mode": "uart", "baudrate": 460800}, protocol="just_float"),
            ProjectTemplate(name="FireWater_PID_Tuning", description="FireWater PID", transport_config={"mode": "uart", "baudrate": 256000}, protocol="fire_water", commands=["set:kp=1.0"]),
            ProjectTemplate(name="TCP_Modbus", description="TCP Modbus", transport_config={"mode": "tcp_client", "host": "127.0.0.1", "port": 502}, protocol="raw_data"),
            ProjectTemplate(name="BLE_Debug", description="BLE 调试", transport_config={"mode": "ble"}, protocol="raw_data", commands=["scan", "connect"]),
        ]

    @classmethod
    def names(cls) -> list[str]:
        return [t.name for t in cls.all()]

    @classmethod
    def count(cls) -> int:
        return 5

    @classmethod
    def get(cls, name: str) -> ProjectTemplate | None:
        for t in cls.all():
            if t.name == name.strip():
                return t
        return None
