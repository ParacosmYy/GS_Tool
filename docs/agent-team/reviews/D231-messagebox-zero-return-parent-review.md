# D231 parent review — MessageBox zero-return fallback

## Decision

`PASS` with limits.

## Findings

- `MessageBoxW` is treated as successful only when its documented nonzero
  result is returned.
- A zero result now reaches the existing stderr fallback.
- Nonzero results preserve the existing no-duplicate-output path.
- Message text, title/icon arguments, Qt boundary, normal `main()` return, and
  final startup exit code are unchanged.
- The change is one local result check and adds no dependency or owner.

## Evidence

- `D231-MESSAGEBOX-ZERO-RETURN-FALLBACK-PROBE=PASS`
- `D231-MESSAGEBOX-NONZERO-RETURN-PRESERVE-PROBE=PASS`
- `D231-COMPILEALL=PASS`
- `D231-RUFF=PASS`
- `D231-FORMAT=PASS`
- `D231-PRESENTATION-AUDIT=PASS`

## Simplification assessment

`D231-SIMPLIFICATION-ASSESSMENT=PASS`: the explicit result check is the
smallest readable change that distinguishes Win32 success from failure.

## Limits

No EXE, QApplication, native MessageBox rendering, clean-machine, or
cross-machine launch was performed. No native runtime success is claimed.
