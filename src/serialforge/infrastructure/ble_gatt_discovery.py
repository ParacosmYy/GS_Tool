"""Optional Bleak discovery adapter kept outside the main runtime import path."""

from __future__ import annotations

import asyncio

from ..domain.errors import TransportDependencyError, TransportDiscoveryError
from ..domain.models import BleGattDevice, BleGattDiscoveryConfig
from ..domain.ports import BleGattDiscoveryPort


class BleakGattDiscovery(BleGattDiscoveryPort):
    """Run one bounded Bleak scan and return immutable advertisement snapshots."""

    def discover(self, config: BleGattDiscoveryConfig) -> tuple[BleGattDevice, ...]:
        try:
            from bleak import BleakScanner
        except ImportError as exc:
            raise TransportDependencyError(
                "BLE 功能需要可选依赖 bleak；请执行 uv sync --locked --extra ble。",
                detail=str(exc),
            ) from exc

        try:
            return asyncio.run(self._discover(BleakScanner, config))
        except (TransportDependencyError, TransportDiscoveryError):
            raise
        except Exception as exc:
            raise TransportDiscoveryError(
                "BLE 扫描失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc

    @staticmethod
    async def _discover(
        scanner_type: object, config: BleGattDiscoveryConfig
    ) -> tuple[BleGattDevice, ...]:
        scanner = scanner_type
        discovered = await scanner.discover(
            timeout=config.scan_timeout,
            return_adv=True,
            service_uuids=list(config.service_uuids) or None,
        )
        snapshots: list[BleGattDevice] = []
        entries = (
            discovered.items()
            if isinstance(discovered, dict)
            else ((None, item) for item in discovered)
        )
        for key, value in entries:
            if isinstance(value, tuple) and len(value) == 2:
                device, advertisement = value
            else:
                device, advertisement = value, None
            device_id = str(getattr(device, "address", key or "")).strip()
            if not device_id:
                continue
            name = str(
                getattr(device, "name", None) or getattr(advertisement, "local_name", None) or ""
            ).strip()
            if config.name_filter and config.name_filter.casefold() not in name.casefold():
                continue
            service_uuids = tuple(
                str(item) for item in (getattr(advertisement, "service_uuids", None) or ())
            )
            if config.service_uuids and not set(config.service_uuids).intersection(
                {item.casefold() for item in service_uuids}
            ):
                continue
            rssi = getattr(advertisement, "rssi", None)
            snapshots.append(
                BleGattDevice(
                    device_id=device_id,
                    name=name,
                    rssi=int(rssi) if isinstance(rssi, (int, float)) else None,
                    service_uuids=service_uuids,
                )
            )
        return tuple(sorted(snapshots, key=lambda item: (item.name.casefold(), item.device_id)))
