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

This tool does not call an LLM and does not modify C++ code by itself. It only executes commands listed in the JSON config.
