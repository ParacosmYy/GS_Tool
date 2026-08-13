# ADR-0001: Windows-first Python/Qt foundation

- Status: accepted for the foundation milestone
- Date: 2026-08-09

## Decision

Use uv-managed Python 3.12, PyQt6/Qt 6 for the desktop shell, QScintilla as the first editor control, and PyInstaller for Windows packaging. Keep QScintilla behind an application-owned editor adapter.

## Reasons

- Python and Qt allow fast iteration while retaining a native desktop UI.
- QScintilla already provides editor-oriented behavior such as syntax styling, folding, markers, and completion.
- uv gives reproducible project environments and a lockfile without requiring a system-wide dependency installation.
- PyInstaller provides a practical Windows EXE path for the first release line.

## Alternatives considered

- **C++/Win32/Scintilla**: stronger low-level control and potentially lower overhead, but a substantially slower first product iteration.
- **C#/.NET/WPF or WinUI**: strong Windows integration, but a different runtime and editor-control integration path.
- **Monaco/Tauri or Electron**: attractive web editor ecosystem, but adds a browser runtime and a different large-file performance profile.

## Consequences

- PyQt/QScintilla licensing must be resolved before distributing a closed-source commercial product.
- Python startup and packaging size must be measured rather than assumed.
- The adapter boundary is mandatory because the first editor control is a replaceable implementation, not the application contract.

