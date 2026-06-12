# Agent GO Loop

`tools/agent-loop` is a small Go command runner for Specs-driven iterations. It repeatedly runs `execute`, `check`, and optional `fix` commands until checks pass or a safety brake stops the loop.

Safety brakes:

- Maximum 20 rounds.
- Maximum 30 minutes.

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

This tool does not call an LLM and does not modify C++ code by itself. It only executes commands listed in the JSON config.
