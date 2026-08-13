"""Pure, explainable policy for external plugin execution authorization."""

from dataclasses import dataclass
from typing import Literal

from ..plugins.catalog import PluginCatalogEntry
from .errors import ApplicationValidationError

PluginExecutionCatalogStatus = Literal["valid", "invalid", "incompatible", "duplicate"]
PluginExecutionTrustState = Literal["trusted", "untrusted"]
PluginExecutionApprovalState = Literal["approved", "stale", "not-approved"]
PluginSignatureState = Literal["valid", "missing", "invalid", "not-verified"]
PluginCodeIdentityState = Literal["matched", "mismatch", "not-verified"]
PluginExecutionContainmentState = Literal[
    "attached",
    "attached-after-start",
    "unsupported",
    "failed",
    "not-requested",
]
PluginExecutionDenyReason = Literal[
    "invalid-evidence",
    "external-execution-disabled",
    "catalog-entry-invalid",
    "untrusted-plugin",
    "approval-missing",
    "approval-stale",
    "signature-not-valid",
    "code-identity-not-verified",
    "plugin-not-enabled",
    "permissions-not-granted",
    "host-containment-unavailable",
    "executor-unavailable",
]
PluginExecutionReason = PluginExecutionDenyReason | Literal["authorized"]


@dataclass(frozen=True, slots=True)
class PluginExecutionEvidence:
    """Independent prerequisites supplied to the execution policy."""

    catalog_status: PluginExecutionCatalogStatus
    trust_state: PluginExecutionTrustState
    approval_state: PluginExecutionApprovalState
    signature_state: PluginSignatureState
    code_identity_state: PluginCodeIdentityState
    enabled: bool
    permissions_granted: bool
    containment_state: PluginExecutionContainmentState
    executor_available: bool


@dataclass(frozen=True, slots=True)
class PluginExecutionDecision:
    """Immutable authorization result with deterministic failure evidence."""

    allowed: bool
    reason: PluginExecutionReason
    failed_requirements: tuple[PluginExecutionDenyReason, ...]
    execution_enabled: bool = False

    def summary(self) -> str:
        """Return a bounded diagnostic suitable for catalog/UI projection."""
        if self.allowed:
            return "External plugin execution authorized"
        requirements = ", ".join(self.failed_requirements) or self.reason
        return f"External plugin execution denied: {self.reason}; requirements: {requirements}"


class PluginExecutionGate:
    """Evaluate external execution evidence without loading or executing code."""

    def __init__(self, *, external_execution_enabled: bool = False) -> None:
        if type(external_execution_enabled) is not bool:
            raise ApplicationValidationError("External execution policy must be a boolean")
        self._external_execution_enabled = external_execution_enabled

    @property
    def external_execution_enabled(self) -> bool:
        """Expose the immutable policy setting for diagnostics and acceptance."""
        return self._external_execution_enabled

    def evaluate(self, evidence: PluginExecutionEvidence) -> PluginExecutionDecision:
        """Return a deterministic decision; never infer missing evidence as success."""
        if not _evidence_is_well_formed(evidence):
            return PluginExecutionDecision(
                allowed=False,
                reason="invalid-evidence",
                failed_requirements=("invalid-evidence",),
                execution_enabled=False,
            )
        failures: list[PluginExecutionDenyReason] = []
        if not self._external_execution_enabled:
            failures.append("external-execution-disabled")
        if evidence.catalog_status != "valid":
            failures.append("catalog-entry-invalid")
        if evidence.trust_state != "trusted":
            failures.append("untrusted-plugin")
        if evidence.approval_state == "stale":
            failures.append("approval-stale")
        elif evidence.approval_state != "approved":
            failures.append("approval-missing")
        if evidence.signature_state != "valid":
            failures.append("signature-not-valid")
        if evidence.code_identity_state != "matched":
            failures.append("code-identity-not-verified")
        if not evidence.enabled:
            failures.append("plugin-not-enabled")
        if not evidence.permissions_granted:
            failures.append("permissions-not-granted")
        if evidence.containment_state != "attached":
            failures.append("host-containment-unavailable")
        if not evidence.executor_available:
            failures.append("executor-unavailable")
        ordered_failures = tuple(_deduplicate(failures))
        if ordered_failures:
            return PluginExecutionDecision(
                allowed=False,
                reason=ordered_failures[0],
                failed_requirements=ordered_failures,
                execution_enabled=False,
            )
        return PluginExecutionDecision(
            allowed=True,
            reason="authorized",
            failed_requirements=(),
            execution_enabled=True,
        )

    def evaluate_catalog_entry(
        self,
        entry: PluginCatalogEntry,
        *,
        containment_state: PluginExecutionContainmentState = "not-requested",
    ) -> PluginExecutionDecision:
        """Evaluate a metadata-only catalog entry with all runtime proof absent."""
        evidence = PluginExecutionEvidence(
            catalog_status=entry.status,
            trust_state=entry.trust_state,
            approval_state=entry.approval_state,
            signature_state="not-verified",
            code_identity_state="not-verified",
            enabled=False,
            permissions_granted=False,
            containment_state=containment_state,
            executor_available=False,
        )
        return self.evaluate(evidence)


def _deduplicate(
    reasons: list[PluginExecutionDenyReason],
) -> tuple[PluginExecutionDenyReason, ...]:
    seen: set[PluginExecutionDenyReason] = set()
    ordered: list[PluginExecutionDenyReason] = []
    for reason in reasons:
        if reason not in seen:
            seen.add(reason)
            ordered.append(reason)
    return tuple(ordered)


def _evidence_is_well_formed(evidence: object) -> bool:
    if not isinstance(evidence, PluginExecutionEvidence):
        return False
    return (
        _is_known_text(evidence.catalog_status, ("valid", "invalid", "incompatible", "duplicate"))
        and _is_known_text(evidence.trust_state, ("trusted", "untrusted"))
        and _is_known_text(evidence.approval_state, ("approved", "stale", "not-approved"))
        and _is_known_text(
            evidence.signature_state, ("valid", "missing", "invalid", "not-verified")
        )
        and _is_known_text(evidence.code_identity_state, ("matched", "mismatch", "not-verified"))
        and type(evidence.enabled) is bool
        and type(evidence.permissions_granted) is bool
        and _is_known_text(
            evidence.containment_state,
            ("attached", "attached-after-start", "unsupported", "failed", "not-requested"),
        )
        and type(evidence.executor_available) is bool
    )


def _is_known_text(value: object, allowed: tuple[str, ...]) -> bool:
    """Validate a literal policy field without hashing untrusted input."""
    return type(value) is str and value in allowed
