"""Compose the built-in command-rail action specifications."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from .icon_contract import IconKey
from .toolbar_contract import ToolbarActionSpec


@dataclass(frozen=True, slots=True)
class CoreToolbarPorts:
    """Application callbacks used to compose the stable core toolbar actions."""

    new_document: Callable[[], None]
    open_document: Callable[[], None]
    save_document: Callable[[], None]
    show_find: Callable[[], None]
    show_replace: Callable[[], None]
    show_command_palette: Callable[[], None]
    choose_workspace: Callable[[], None] | None = None


class CoreToolbarCoordinator:
    """Build presentation-only toolbar specs without owning Qt projection."""

    def __init__(self, ports: CoreToolbarPorts) -> None:
        self._ports = ports

    def actions(self) -> tuple[ToolbarActionSpec, ...]:
        """Return the stable core action order, including workspace when available."""
        ports = self._ports
        actions = (
            ToolbarActionSpec(
                "toolbar.new",
                ports.new_document,
                icon_key=IconKey.DOCUMENT_NEW,
            ),
            ToolbarActionSpec(
                "toolbar.open",
                ports.open_document,
                icon_key=IconKey.FOLDER_OPEN,
            ),
            ToolbarActionSpec(
                "toolbar.save",
                ports.save_document,
                icon_key=IconKey.SAVE,
                role="primary",
            ),
            ToolbarActionSpec(
                "toolbar.find",
                ports.show_find,
                icon_key=IconKey.SEARCH,
                separator_before=True,
            ),
            ToolbarActionSpec(
                "toolbar.replace",
                ports.show_replace,
                icon_key=IconKey.REPLACE,
                role="quiet",
            ),
            ToolbarActionSpec(
                "toolbar.command_palette",
                ports.show_command_palette,
                icon_key=IconKey.COMMAND,
                role="quiet",
            ),
        )
        if ports.choose_workspace is None:
            return actions
        return actions + (
            ToolbarActionSpec(
                "toolbar.workspace",
                ports.choose_workspace,
                icon_key=IconKey.FOLDER,
                role="context",
            ),
        )


__all__ = ["CoreToolbarCoordinator", "CoreToolbarPorts"]
