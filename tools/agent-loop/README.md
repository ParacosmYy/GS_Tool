# Agent GO Loop

`tools/agent-loop` is a small Go command runner for Specs-driven iterations. It repeatedly runs `execute`, `check`, and optional `fix` commands until checks pass or a safety brake stops the loop.

Safety brakes:

- Maximum 20 rounds.
- Maximum 30 minutes.
- When the round brake trips after repeated check failures, the error includes the last failing check command and output so the next step can route into LOOP Debug or Simplify.

Example:

```powershell
Push-Location .\tools\agent-loop
go run . -config .\sample.embeddebug.json -dry-run
go run . -config .\sample.embeddebug.json
Pop-Location
```

Environment notes:

- Run `tools\bootstrap_env.bat` once before using the loop on a fresh machine.
- The sample config uses `tools\doctor.ps1` and `EmbedDebug.bat` because those entry points load the project-local Qt/MinGW paths.
- Direct `cmake --build build` commands from an arbitrary shell can fail to start Qt tools such as `moc.exe` if `QT_PREFIX\bin` and `QT_PREFIX\share\qt6\bin` are not on `PATH`.
- If `go.exe` is missing, `tools\doctor.ps1` reports a warning. Install Go or run the listed commands manually until Go is available.
- The sample config uses `tools\verify_embeddebug_launch.ps1` instead of calling `EmbedDebug.bat` directly. The probe starts the app, waits for a new `EmbedDebug.exe` process, reports `launch_result=PASSED`, and then closes only the process it started.

Manual fallback when Go is not available:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

This tool does not call an LLM and does not modify C++ code by itself. It only executes commands listed in the JSON config.

Recommended closeout after a GO run:

1. If the loop passes, record the executed config and resulting state in the Specs closeout section.
2. If `execute` fails, run Doctor first and route build errors into Debug.
3. If repeated `check` failures hit the round brake, use the reported last check command/output as the Debug trace repro.
4. If repeated fixes do not converge, open Simplify before expanding the implementation.
