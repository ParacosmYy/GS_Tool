# D76 / UI-49 parent review: recovery notification dynamic localization

## Verdict

`PASS by source review; accepted-with-limits`.

## Scope

Reviewed `src/quillforge/presentation/i18n.py` and the existing recovery
notification call site in `MainWindow`. The change is limited to the existing
presentation localization seam.

## Five-axis review

- **Correctness — PASS:** `en-US` exits before projection and returns the
  exact source message. The Chinese branch requires both the known prefix and
  suffix, then preserves the middle document-name segment verbatim.
- **Readability — PASS:** named prefix/suffix constants make the shape and
  boundary obvious; no nested parser or new template abstraction is added.
- **Architecture — PASS:** recovery services and MainWindow continue to own
  recovery state and notification intent. `localize_message()` remains the
  sole presentation projection owner.
- **Security — PASS:** dynamic names are copied into an f-string as data; no
  evaluation, path access, filesystem operation, or markup interpretation is
  introduced.
- **Performance — PASS:** one bounded prefix/suffix check is linear in the
  notification length and only runs on the existing status-message path.

## Simplification assessment

No safer simplification was identified. Inlining the literals would make the
message-shape contract less legible; a general template registry would add
complexity for one use case.

## Review-role evidence

The Architect role (Lorentz the 3rd / Luna max) returned `NO_CONCLUSION` after
the bounded review window. The independent reviewer (Feynman the 3rd / Luna
max) also returned `NO_CONCLUSION`. No child PASS is claimed.

## Verification and limits

`D76-I18N-RECOVERY-PROBE=PASS` covers `zh-CN`, exact `en-US` identity,
Unicode/punctuation document names, and the absence of the mixed-language
suffix. Full static/package/handoff checks and package identity are recorded
in the D76 handoff. Native runtime language switching, Qt rendering,
accessibility, and release evidence remain unrun.
