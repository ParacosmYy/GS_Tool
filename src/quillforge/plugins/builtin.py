"""Built-in plugin used to prove the public extension surface end to end."""

from ..application.commands import Command
from .api import PluginContext, PluginManifest


class DocumentStatsPlugin:
    """Expose document statistics without importing Qt or touching a widget."""

    manifest = PluginManifest(
        plugin_id="quillforge.document-stats",
        name="Document Statistics",
        version="1.0.0",
        permissions=("commands", "active_document", "notifications"),
    )

    def __init__(self) -> None:
        self._context: PluginContext | None = None

    def activate(self, context: PluginContext) -> None:
        self._context = context
        context.register_command(
            Command(
                command_id="tools.document-stats",
                title="Document Statistics",
                execute=self._show_statistics,
                menu_id="tools",
            )
        )

    def deactivate(self) -> None:
        self._context = None

    def _show_statistics(self) -> None:
        if self._context is None:
            return
        document = self._context.get_active_document()
        if document is None:
            self._context.notify("No active document")
            return
        lines = document.text.count("\n") + 1
        characters = len(document.text)
        self._context.notify(f"Document statistics: {lines} lines, {characters} characters")
