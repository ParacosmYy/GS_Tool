"""Qt-free admission for save and save-as path selection."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True, slots=True)
class DocumentSavePickerAdmissionPorts[TabT]:
    """Typed callbacks required to admit one document save path flow."""

    get_active_tab: Callable[[], TabT | None]
    is_busy: Callable[[], bool]
    get_current_path: Callable[[TabT], Path | None]
    choose_save_path: Callable[[Path | None], Path | None]
    start_save: Callable[[TabT, Path], None]


class DocumentSavePickerAdmissionCoordinator[TabT]:
    """Route ordinary Save and Save As into the existing save boundary."""

    def __init__(self, ports: DocumentSavePickerAdmissionPorts[TabT]) -> None:
        self._ports = ports

    def admit(self, *, force_picker: bool = False) -> bool:
        """Admit one save target while preserving Save versus Save As intent."""
        tab = self._ports.get_active_tab()
        if tab is None or self._ports.is_busy():
            return False
        current = self._ports.get_current_path(tab)
        target = current
        if force_picker or target is None:
            target = self._ports.choose_save_path(current)
        if target is None:
            return False
        self._ports.start_save(tab, target)
        return True


__all__ = ["DocumentSavePickerAdmissionCoordinator", "DocumentSavePickerAdmissionPorts"]
