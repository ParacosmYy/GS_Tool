# D297 parent review — frozen Qt runtime preflight

## Scope

Reviewed `src/quillforge/app.py`, the normal `app.main()` branch ordering,
`__main__` startup exception handling, existing frozen dependency diagnostics,
the static contract audit, and the rebuilt one-file archive.

## Findings

- PASS — the preflight is reached only after plugin-host and all diagnostic
  branches have returned.
- PASS — it executes before the normal `QApplication` import and construction.
- PASS — source mode is a no-op and the source startup/file-open diagnostics
  remain green.
- PASS — the check reuses the existing platform-plugin and runtime-dependency
  inventories rather than introducing a second list of binaries.
- PASS — missing paths are deduplicated and reported as actionable bundle paths
  through the existing `__main__` localized startup fallback.
- PASS — the current archive contains every required Qt DLL, QScintilla, and
  `qwindows.dll`; PE and root/dist/manifest identity match.
- PASS — no composition, user-data, plugin-execution, theme, or document policy
  changed.

## Simplification assessment

PASS. A single frozen-only call at the application entry boundary is the
smallest complete change. Moving the check into composition would import Qt
too early; duplicating the dependency list would create drift; changing the
diagnostic runner would incorrectly couple diagnostics to normal startup.

## Review limits

The independent Luna/max review window returned `NO_CONCLUSION` after three
bounded waits and was closed. No independent PASS is claimed. Native EXE/Qt
startup, clean-machine behavior, and real loader failures remain unrun.

## Applicability

Python 3.12/PyQt6/PyInstaller desktop code only. Public embedded-vendor source
applicability is N/A.
