"""Small, synchronous application event bus with explicit subscriptions."""

from collections import defaultdict
from collections.abc import Callable
from dataclasses import dataclass
from threading import RLock, get_ident
from typing import TypeVar, cast

from ..domain.models import DocumentState
from .errors import ApplicationStateError

Event = TypeVar("Event")
EventHandler = Callable[[Event], None]


class EventBus:
    """In-process event bus used for host and plugin lifecycle notifications."""

    def __init__(self) -> None:
        self._handlers: defaultdict[type[object], list[Callable[[object], None]]] = defaultdict(
            list
        )
        self._lock = RLock()
        self._owner_thread_id = get_ident()

    def subscribe(
        self, event_type: type[Event], handler: EventHandler[Event]
    ) -> Callable[[], None]:
        """Subscribe and return an idempotent unsubscribe callback."""
        typed_handler = cast(Callable[[object], None], handler)
        with self._lock:
            self._handlers[event_type].append(typed_handler)

        def unsubscribe() -> None:
            with self._lock:
                handlers = self._handlers.get(event_type)
                if handlers is not None and typed_handler in handlers:
                    handlers.remove(typed_handler)

        return unsubscribe

    def publish(self, event: object) -> None:
        """Publish to a snapshot so subscribers may safely unsubscribe."""
        if get_ident() != self._owner_thread_id:
            raise ApplicationStateError(
                "Application events must be published on the owning UI thread"
            )
        with self._lock:
            handlers = tuple(self._handlers.get(type(event), ()))
        for handler in handlers:
            handler(event)


@dataclass(frozen=True, slots=True)
class DocumentOpened:
    """Published after a document has been materialized in the UI."""

    state: DocumentState


@dataclass(frozen=True, slots=True)
class DocumentSaved:
    """Published after a document has been persisted successfully."""

    state: DocumentState


@dataclass(frozen=True, slots=True)
class DocumentClosed:
    """Published after a document tab is closed."""

    state: DocumentState


@dataclass(frozen=True, slots=True)
class PluginFailed:
    """Published when a plugin fails during activation or event handling."""

    plugin_id: str
    phase: str
    error: str
