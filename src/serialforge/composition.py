"""The only composition root for SerialForge dependencies."""

from __future__ import annotations

from dataclasses import dataclass

from .application.commands import CommandBatchService
from .application.components import (
    ComponentEventBridge,
    ComponentPipelineWorker,
    ComponentProfileStore,
)
from .application.datasets import DatasetConfigStore, DatasetEventBridge, DatasetPipelineWorker
from .application.events import BoundedEventBus
from .application.gatt_sessions import BleGattSessionManager
from .application.paths import ApplicationPaths, configure_logging
from .application.protocols import ProtocolEventBridge, ProtocolPipelineWorker
from .application.replay import ReplayPipelineWorker
from .application.services import (
    BoundedRawRecorder,
    CommandHistoryService,
    EndpointIdentityService,
)
from .application.session_router import RoutingSessionManager
from .application.sessions import SessionManager
from .application.tcp_server_sessions import TcpServerSessionManager
from .domain.ports import (
    BleGattDiscoveryPort,
    CommandBatchPort,
    CommandHistoryPort,
    ComponentPipelinePort,
    DatasetConfigStorePort,
    DatasetPipelinePort,
    EndpointDiscoveryPort,
    EndpointIdentityPort,
    EventPort,
    ProtocolPipelinePort,
    RawRecorderPort,
    ReplayPipelinePort,
    SessionPort,
)
from .infrastructure.ble_gatt_discovery import BleakGattDiscovery
from .infrastructure.ble_gatt_transport import BleakGattTransportFactory
from .infrastructure.serial_transport import SerialPortDiscovery
from .infrastructure.tcp_server_transport import TcpServerTransportFactory
from .infrastructure.transport_factory import RoutingTransportFactory


@dataclass(frozen=True, slots=True)
class ApplicationContext:
    """Application services and ports shared by the presentation layer."""

    session: SessionPort
    events: EventPort
    discovery: EndpointDiscoveryPort
    recorder: RawRecorderPort
    history: CommandHistoryPort
    identities: EndpointIdentityPort
    ble_discovery: BleGattDiscoveryPort
    protocol: ProtocolPipelinePort
    components: ComponentPipelinePort
    component_profiles: ComponentProfileStore
    dataset: DatasetPipelinePort
    dataset_configs: DatasetConfigStorePort
    replay: ReplayPipelinePort
    commands: CommandBatchPort
    paths: ApplicationPaths


def create_application() -> ApplicationContext:
    """Build the backend without importing Qt or opening a device."""

    paths = ApplicationPaths.from_environment()
    paths.ensure_directories()
    configure_logging(paths)

    events = BoundedEventBus(capacity=256)
    recorder = BoundedRawRecorder(event_sink=events)
    client_session = SessionManager(
        transport_factory=RoutingTransportFactory(),
        event_sink=events,
        raw_recorder=recorder,
    )
    server_session = TcpServerSessionManager(
        transport_factory=TcpServerTransportFactory(),
        event_sink=events,
        raw_recorder=recorder,
    )
    ble_session = BleGattSessionManager(
        transport_factory=BleakGattTransportFactory(),
        event_sink=events,
        raw_recorder=recorder,
    )
    session = RoutingSessionManager(client_session, server_session, ble_session)
    discovery = SerialPortDiscovery()
    ble_discovery = BleakGattDiscovery()
    protocol = ProtocolPipelineWorker(event_sink=events)
    components = ComponentPipelineWorker(event_sink=events)
    dataset = DatasetPipelineWorker(event_sink=events)
    replay = ReplayPipelineWorker(protocol=protocol, event_sink=events)
    events.add_observer(ProtocolEventBridge(protocol).observe)
    events.add_observer(ComponentEventBridge(components).observe)
    events.add_observer(DatasetEventBridge(dataset).observe)
    history = CommandHistoryService()
    commands = CommandBatchService(session=session, history=history)
    identities = EndpointIdentityService()
    return ApplicationContext(
        session=session,
        events=events,
        discovery=discovery,
        recorder=recorder,
        history=history,
        identities=identities,
        ble_discovery=ble_discovery,
        protocol=protocol,
        components=components,
        component_profiles=ComponentProfileStore(),
        dataset=dataset,
        dataset_configs=DatasetConfigStore(),
        replay=replay,
        commands=commands,
        paths=paths,
    )


def create_main_window(context: ApplicationContext):
    """Build the Qt presentation objects from already-created application ports."""

    from .presentation.connection_preset_store import QSettingsConnectionPresetCatalogStore
    from .presentation.main_window import MainWindow
    from .presentation.preferences import QSettingsPreferenceStore
    from .presentation.viewmodels import SessionViewModel

    view_model = SessionViewModel(
        session=context.session,
        events=context.events,
        discovery=context.discovery,
        recorder=context.recorder,
        history=context.history,
        identities=context.identities,
        ble_discovery=context.ble_discovery,
        protocol=context.protocol,
        components=context.components,
        component_profiles=context.component_profiles,
        dataset=context.dataset,
        dataset_configs=context.dataset_configs,
        replay=context.replay,
        commands=context.commands,
        default_record_path=context.paths.default_record_path,
    )
    preset_store = QSettingsConnectionPresetCatalogStore()
    preset_catalog = preset_store.load()
    return MainWindow(view_model, QSettingsPreferenceStore(), preset_catalog, preset_store)
