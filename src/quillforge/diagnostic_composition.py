"""Qt-free composition helpers for packaged diagnostics."""

from .application.workspace_search import WorkspaceSearchService
from .infrastructure.workspace_search_provider import FileWorkspaceSearchProvider


def build_workspace_search_service() -> WorkspaceSearchService:
    """Assemble the local search service without importing the Qt runtime."""
    return WorkspaceSearchService(FileWorkspaceSearchProvider())
