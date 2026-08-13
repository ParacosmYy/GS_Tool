"""Semantic icon identifiers shared without a Qt dependency."""

from enum import StrEnum


class IconKey(StrEnum):
    """Semantic shell icons independent of a platform's native icon set."""

    DOCUMENT = "document"
    DOCUMENT_NEW = "document-new"
    FOLDER = "folder"
    FOLDER_OPEN = "folder-open"
    SAVE = "save"
    SEARCH = "search"
    REPLACE = "replace"
    COMMAND = "command"
    ARROW_UP = "arrow-up"
    ARROW_DOWN = "arrow-down"
    CLOSE = "close"
    MODIFIED = "modified"
    WARNING = "warning"


__all__ = ["IconKey"]
