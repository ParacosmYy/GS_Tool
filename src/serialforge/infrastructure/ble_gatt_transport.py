"""Optional Bleak GATT adapter with all backend objects confined to one asyncio worker."""

from __future__ import annotations

from typing import NoReturn

from ..domain.errors import (
    BleGattAuthorizationError,
    BleGattCharacteristicUnsupportedError,
    BleGattDeviceNotFoundError,
    BleGattMtuError,
    BleGattNotificationError,
    TransportCloseError,
    TransportDependencyError,
    TransportOpenError,
    TransportReadError,
    TransportWriteError,
)
from ..domain.models import (
    MAX_BLE_WRITE_WITH_RESPONSE_BYTES,
    BleGattCharacteristic,
    BleGattCharacteristicRef,
    BleGattService,
    BleGattTransportConfig,
    BleGattWrite,
    BleGattWriteMode,
    Endpoint,
    TransportKind,
)
from ..domain.ports import (
    BleGattDisconnectHandler,
    BleGattNotificationHandler,
    BleGattTransportPort,
)


class BleakGattTransport(BleGattTransportPort):
    """A lazy Bleak client; no Bleak import occurs until BLE is actually opened."""

    def __init__(self, config: BleGattTransportConfig) -> None:
        self._config = config
        self._client: object | None = None
        self._characteristics: dict[str, object] = {}
        self._capabilities: dict[str, BleGattCharacteristic] = {}
        self._refs_by_handle: dict[int, BleGattCharacteristicRef] = {}
        self._notifications: set[str] = set()
        self._mtu_size: int | None = None
        self._notification_handler: BleGattNotificationHandler | None = None
        self._disconnected_handler: BleGattDisconnectHandler | None = None
        self._closing = False

    @property
    def endpoint(self) -> Endpoint:
        return self._config.endpoint

    @property
    def kind(self) -> TransportKind:
        return TransportKind.BLE_GATT

    @property
    def mtu_size(self) -> int | None:
        return self._mtu_size

    def set_handlers(
        self,
        notification: BleGattNotificationHandler,
        disconnected: BleGattDisconnectHandler,
    ) -> None:
        self._notification_handler = notification
        self._disconnected_handler = disconnected

    async def open(self) -> tuple[BleGattService, ...]:
        try:
            from bleak import BleakClient
        except ImportError as exc:
            raise TransportDependencyError(
                "BLE 功能需要可选依赖 bleak；请执行 uv sync --locked --extra ble。",
                detail=str(exc),
            ) from exc

        kwargs: dict[str, object] = {
            "timeout": self._config.connect_timeout,
            "pair": self._config.pair,
        }
        if self._config.service_uuids:
            kwargs["services"] = set(self._config.service_uuids)
        if self._config.use_cached_services is not None:
            kwargs["winrt"] = {"use_cached_services": self._config.use_cached_services}
        self._closing = False
        try:
            self._client = BleakClient(
                self._config.device_id,
                disconnected_callback=self._on_disconnected,
                **kwargs,
            )
            await self._client.connect()  # type: ignore[union-attr]
            if not self._client.is_connected:  # type: ignore[union-attr]
                raise BleGattDeviceNotFoundError(self._config.device_id)
            self._mtu_size = self._read_mtu()
            services = self._build_services()
            if not services:
                raise TransportOpenError("BLE 设备未发现任何 GATT service。")
            return services
        except Exception as exc:
            await self._disconnect_quietly()
            self._raise_open_error(exc)

    async def read(self, characteristic: BleGattCharacteristicRef) -> bytes:
        client = self._require_client()
        capability, handle = self._require_characteristic(characteristic)
        if not capability.supports("read"):
            raise BleGattCharacteristicUnsupportedError(f"特征 {characteristic.key} 不支持 read。")
        try:
            return bytes(await client.read_gatt_char(handle))
        except Exception as exc:
            self._raise_read_error(exc, characteristic)

    async def write(self, command: BleGattWrite) -> None:
        if not isinstance(command, BleGattWrite):
            raise TypeError("BLE transport.write 只接受 BleGattWrite。")
        client = self._require_client()
        capability, handle = self._require_characteristic(command.characteristic)
        if command.mode is BleGattWriteMode.WITH_RESPONSE:
            if not capability.supports("write"):
                raise BleGattCharacteristicUnsupportedError(
                    f"特征 {command.characteristic.key} 不支持 write-with-response。"
                )
            if len(command.payload) > MAX_BLE_WRITE_WITH_RESPONSE_BYTES:
                raise BleGattMtuError("BLE with-response payload 超过当前未分片上限。")
        elif not capability.supports("write-without-response"):
            raise BleGattCharacteristicUnsupportedError(
                f"特征 {command.characteristic.key} 不支持 write-without-response。"
            )
        maximum = capability.max_write_without_response_size
        if (
            command.mode is BleGattWriteMode.WITHOUT_RESPONSE
            and maximum is not None
            and len(command.payload) > maximum
        ):
            raise BleGattMtuError(
                f"BLE payload {len(command.payload)} B 超过该特征当前 "
                f"without-response 上限 {maximum} B。"
            )
        try:
            await client.write_gatt_char(
                handle,
                command.payload,
                response=command.mode is BleGattWriteMode.WITH_RESPONSE,
            )
        except Exception as exc:
            self._raise_write_error(exc, command.characteristic)

    async def set_notify(self, characteristic: BleGattCharacteristicRef, enabled: bool) -> None:
        client = self._require_client()
        capability, handle = self._require_characteristic(characteristic)
        if not capability.supports("notify") and not capability.supports("indicate"):
            raise BleGattCharacteristicUnsupportedError(
                f"特征 {characteristic.key} 不支持 notification/indication。"
            )
        try:
            if enabled:
                await client.start_notify(handle, self._on_notification)
                self._notifications.add(characteristic.key)
            else:
                await client.stop_notify(handle)
                self._notifications.discard(characteristic.key)
        except Exception as exc:
            raise BleGattNotificationError(
                f"BLE 特征 {characteristic.key} 通知订阅操作失败。",
                detail=f"{type(exc).__name__}: {exc}",
            ) from exc

    async def close(self) -> None:
        self._closing = True
        first_error: Exception | None = None
        client = self._client
        if client is None:
            return
        for key in tuple(self._notifications):
            capability = self._capabilities.get(key)
            handle = self._characteristics.get(key)
            if capability is None or handle is None:
                continue
            try:
                await client.stop_notify(handle)
            except Exception as exc:
                first_error = first_error or exc
        self._notifications.clear()
        try:
            await client.disconnect()
        except Exception as exc:
            first_error = first_error or exc
        finally:
            self._client = None
            self._characteristics.clear()
            self._capabilities.clear()
            self._refs_by_handle.clear()
        if first_error is not None:
            raise TransportCloseError(
                "关闭 BLE GATT 连接时发生错误。",
                detail=f"{type(first_error).__name__}: {first_error}",
            ) from first_error

    def _build_services(self) -> tuple[BleGattService, ...]:
        client = self._client
        services = getattr(client, "services", None)
        if services is None:
            raise TransportOpenError("BLE GATT 服务发现尚未完成。")
        result: list[BleGattService] = []
        for service in services:
            characteristics: list[BleGattCharacteristic] = []
            for item in service.characteristics:
                ref = BleGattCharacteristicRef(
                    service_uuid=str(service.uuid),
                    characteristic_uuid=str(item.uuid),
                    handle=int(item.handle),
                )
                properties = tuple(str(value).lower() for value in item.properties)
                maximum = self._read_without_response_size(item)
                capability = BleGattCharacteristic(
                    ref=ref,
                    properties=properties,
                    max_write_without_response_size=maximum,
                )
                characteristics.append(capability)
                self._characteristics[ref.key] = item
                self._capabilities[ref.key] = capability
                self._refs_by_handle[ref.handle] = ref
            result.append(
                BleGattService(
                    uuid=str(service.uuid),
                    characteristics=tuple(characteristics),
                )
            )
        return tuple(result)

    def _require_characteristic(
        self,
        characteristic: BleGattCharacteristicRef,
    ) -> tuple[BleGattCharacteristic, object]:
        capability = self._capabilities.get(characteristic.key)
        handle = self._characteristics.get(characteristic.key)
        if capability is None or handle is None:
            raise BleGattCharacteristicUnsupportedError(
                f"BLE 特征 {characteristic.key} 不在当前服务快照中。"
            )
        return capability, handle

    def _require_client(self) -> object:
        client = self._client
        if client is None or not client.is_connected:  # type: ignore[union-attr]
            raise TransportOpenError("BLE GATT 设备尚未连接。")
        return client

    def _on_notification(self, sender: object, data: bytearray) -> None:
        if self._closing:
            return
        handle = getattr(sender, "handle", None)
        ref = self._refs_by_handle.get(int(handle)) if handle is not None else None
        if ref is None or self._notification_handler is None:
            return
        self._notification_handler(ref, bytes(data))

    def _on_disconnected(self, _client: object) -> None:
        if not self._closing and self._disconnected_handler is not None:
            self._disconnected_handler()

    def _read_mtu(self) -> int | None:
        value = getattr(self._client, "mtu_size", None)
        return int(value) if isinstance(value, int) and value > 0 else None

    @staticmethod
    def _read_without_response_size(item: object) -> int | None:
        try:
            value = item.max_write_without_response_size
        except Exception:
            return None
        return int(value) if isinstance(value, int) and value > 0 else None

    async def _disconnect_quietly(self) -> None:
        if self._client is None:
            return
        try:
            await self._client.disconnect()  # type: ignore[union-attr]
        except Exception:
            pass
        self._client = None

    def _raise_open_error(self, exc: Exception) -> NoReturn:
        if isinstance(exc, (TransportOpenError, BleGattDeviceNotFoundError)):
            raise exc
        message = str(exc).casefold()
        if any(value in message for value in ("not found", "no device", "could not find")):
            raise BleGattDeviceNotFoundError(self._config.device_id, detail=str(exc)) from exc
        if any(value in message for value in ("access", "authorize", "pair", "authentication")):
            raise BleGattAuthorizationError(detail=str(exc)) from exc
        raise TransportOpenError(
            "无法连接 BLE GATT 设备。",
            detail=f"{type(exc).__name__}: {exc}",
        ) from exc

    def _raise_read_error(
        self, exc: Exception, characteristic: BleGattCharacteristicRef
    ) -> NoReturn:
        message = str(exc).casefold()
        if any(value in message for value in ("access", "authorize", "pair", "authentication")):
            raise BleGattAuthorizationError(detail=str(exc)) from exc
        raise TransportReadError(
            f"读取 BLE 特征 {characteristic.key} 失败。",
            detail=f"{type(exc).__name__}: {exc}",
        ) from exc

    def _raise_write_error(
        self, exc: Exception, characteristic: BleGattCharacteristicRef
    ) -> NoReturn:
        message = str(exc).casefold()
        if any(value in message for value in ("access", "authorize", "pair", "authentication")):
            raise BleGattAuthorizationError(detail=str(exc)) from exc
        raise TransportWriteError(
            f"写入 BLE 特征 {characteristic.key} 失败。",
            detail=f"{type(exc).__name__}: {exc}",
        ) from exc


class BleakGattTransportFactory:
    """Create lazy Bleak adapters without importing the optional package."""

    def create(self, config: BleGattTransportConfig) -> BleakGattTransport:
        return BleakGattTransport(config)
