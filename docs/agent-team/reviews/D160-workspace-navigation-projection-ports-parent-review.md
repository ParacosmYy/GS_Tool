# D160 / ARCH-147 parent review

- Reviewer: parent agent
- Result: `PASS`
- Scope: workspace-navigation projection Ports contract and MainWindow wiring

## Findings

- `WorkspaceNavigationProjectionPorts` is frozen/slotted, named, Qt-free, and
  limited to the nine existing navigation projection callbacks.
- Opened results preserve invalidate -> activate -> search root -> surface
  guard -> directory -> notification -> session save -> restore order.
- Missing surfaces stop after the guard; directory results require both a
  current root and a live surface.
- MainWindow retains WorkspaceService, containment, file activation, TaskRunner,
  surface, session, startup, close, and policy ownership.

## Simplification assessment

`PASS`: the named immutable contract removes positional coupling without
adding a state machine, event bus, registry, duplicate lifecycle, or policy
layer.

## Limits

This is source, inline, package, and static evidence only. Native event timing,
file/folder activation, filesystem behavior, runtime startup, clean-machine,
cross-machine, and release-owner evidence remain unrun or open.
