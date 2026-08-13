"""Application-owned limits for editor operations that can consume UI time."""

from dataclasses import dataclass

from .errors import ApplicationValidationError


@dataclass(frozen=True, slots=True)
class EditorOperationPolicy:
    """Bound editor work without coupling product policy to QScintilla."""

    max_replace_matches: int = 10_000
    replace_all_slice_budget_ms: int = 8
    replace_all_items_per_slice: int = 256
    recovery_capture_slice_budget_ms: int = 8
    recovery_capture_chunks_per_slice: int = 512
    recovery_capture_characters_per_chunk: int = 16_384
    recovery_capture_queue_chunks: int = 64
    recovery_capture_queue_bytes: int = 1_048_576

    def __post_init__(self) -> None:
        """Reject unsafe policy values at the composition boundary."""
        if type(self.max_replace_matches) is not int or self.max_replace_matches < 1:
            raise ApplicationValidationError("max_replace_matches must be a positive integer")
        if (
            type(self.replace_all_slice_budget_ms) is not int
            or self.replace_all_slice_budget_ms < 1
        ):
            raise ApplicationValidationError(
                "replace_all_slice_budget_ms must be a positive integer"
            )
        if (
            type(self.replace_all_items_per_slice) is not int
            or self.replace_all_items_per_slice < 1
        ):
            raise ApplicationValidationError(
                "replace_all_items_per_slice must be a positive integer"
            )
        if (
            type(self.recovery_capture_slice_budget_ms) is not int
            or self.recovery_capture_slice_budget_ms < 1
        ):
            raise ApplicationValidationError(
                "recovery_capture_slice_budget_ms must be a positive integer"
            )
        if (
            type(self.recovery_capture_chunks_per_slice) is not int
            or self.recovery_capture_chunks_per_slice < 1
        ):
            raise ApplicationValidationError(
                "recovery_capture_chunks_per_slice must be a positive integer"
            )
        if (
            type(self.recovery_capture_characters_per_chunk) is not int
            or self.recovery_capture_characters_per_chunk < 1
        ):
            raise ApplicationValidationError(
                "recovery_capture_characters_per_chunk must be a positive integer"
            )
        if (
            type(self.recovery_capture_queue_chunks) is not int
            or self.recovery_capture_queue_chunks < 1
        ):
            raise ApplicationValidationError(
                "recovery_capture_queue_chunks must be a positive integer"
            )
        if (
            type(self.recovery_capture_queue_bytes) is not int
            or self.recovery_capture_queue_bytes < 1
        ):
            raise ApplicationValidationError(
                "recovery_capture_queue_bytes must be a positive integer"
            )


DEFAULT_EDITOR_OPERATION_POLICY = EditorOperationPolicy()
