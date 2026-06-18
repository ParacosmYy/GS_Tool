"""驱动适配器注册表。"""

from __future__ import annotations

from embeddebug.serial_station.driver_adapters.info import (
    DriverInfo,
    DriverStatus,
    check_availability,
)


class DriverRegistry:
    """按稳定 name 索引的驱动信息注册表。"""

    def __init__(self) -> None:
        self._drivers: dict[str, DriverInfo] = {}

    def register(self, info: DriverInfo) -> None:
        if not info.name:
            raise ValueError("driver info name is required")
        if info.name in self._drivers:
            raise ValueError(f"duplicate driver name: {info.name}")
        self._drivers[info.name] = info

    def register_many(self, infos: list[DriverInfo]) -> None:
        for info in infos:
            self.register(info)

    def list_drivers(self) -> list[DriverInfo]:
        return list(self._drivers.values())

    def get(self, name: str) -> DriverInfo | None:
        return self._drivers.get(name)

    def check_all(self) -> dict[str, DriverStatus]:
        return {name: check_availability(info) for name, info in self._drivers.items()}

    @property
    def names(self) -> tuple[str, ...]:
        return tuple(self._drivers)
