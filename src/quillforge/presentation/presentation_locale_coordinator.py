"""Qt-free orchestration for the presentation locale refresh sequence."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..domain.models import Locale


@dataclass(frozen=True, slots=True)
class PresentationLocalePorts:
    """Concrete presentation callbacks used by the locale refresh boundary."""

    get_locale: Callable[[], Locale]
    set_window_title: Callable[[Locale], None]
    retranslate_commands: Callable[[Locale], None]
    set_command_palette_locale: Callable[[Locale], None]
    set_editor_shell_locale: Callable[[Locale], None]
    refresh_tab_icons: Callable[[], None]
    refresh_tab_titles: Callable[[], None]
    set_file_dialog_locale: Callable[[Locale], None]
    set_message_locale: Callable[[Locale], None]
    set_recovery_prompt_locale: Callable[[Locale], None]
    set_status_locale: Callable[[Locale], None]
    set_workspace_search_locale: Callable[[Locale], None]
    set_plugin_locale: Callable[[Locale], None]
    set_workspace_locale: Callable[[Locale], None] | None
    refresh_workspace_icons: Callable[[], None] | None


class PresentationLocaleCoordinator:
    """Apply one locale snapshot to all existing presentation surfaces."""

    def __init__(self, ports: PresentationLocalePorts) -> None:
        self._ports = ports

    def retranslate(self) -> None:
        """Preserve the shell's established locale refresh order."""
        locale = self._ports.get_locale()
        self._ports.set_window_title(locale)
        self._ports.retranslate_commands(locale)
        self._ports.set_command_palette_locale(locale)
        if self._ports.set_workspace_locale is not None:
            self._ports.set_workspace_locale(locale)
        if self._ports.refresh_workspace_icons is not None:
            self._ports.refresh_workspace_icons()
        self._ports.set_editor_shell_locale(locale)
        self._ports.refresh_tab_titles()
        self._ports.refresh_tab_icons()
        self._ports.set_file_dialog_locale(locale)
        self._ports.set_message_locale(locale)
        self._ports.set_recovery_prompt_locale(locale)
        self._ports.set_status_locale(locale)
        self._ports.set_workspace_search_locale(locale)
        self._ports.set_plugin_locale(locale)


__all__ = ["PresentationLocaleCoordinator", "PresentationLocalePorts"]
