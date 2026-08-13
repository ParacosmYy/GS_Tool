"""Application command registry exposed to UI and plugins."""

from collections.abc import Callable
from dataclasses import dataclass
from typing import Literal

from .errors import ApplicationValidationError

MenuId = Literal["file", "edit", "tools", "help"]
SUPPORTED_MENU_IDS: tuple[MenuId, ...] = ("file", "edit", "tools", "help")


@dataclass(frozen=True, slots=True)
class Command:
    """A user-invokable application command."""

    command_id: str
    title: str
    execute: Callable[[], None]
    shortcut: str | None = None
    menu_id: str = "tools"


class CommandRegistry:
    """Registry with stable command IDs and explicit collision handling."""

    def __init__(self) -> None:
        self._commands: dict[str, Command] = {}

    def register(self, command: Command) -> None:
        if command.command_id in self._commands:
            raise ApplicationValidationError(f"Command already registered: {command.command_id}")
        if command.menu_id not in SUPPORTED_MENU_IDS:
            raise ApplicationValidationError(f"Unsupported command menu: {command.menu_id}")
        self._commands[command.command_id] = command

    def get(self, command_id: str) -> Command | None:
        return self._commands.get(command_id)

    def unregister(self, command_id: str) -> None:
        """Remove a command owned by a deactivated extension."""
        self._commands.pop(command_id, None)

    def all(self) -> tuple[Command, ...]:
        return tuple(self._commands.values())
