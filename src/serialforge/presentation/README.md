# Presentation boundary

`serialforge.presentation` is the only UI package. It owns PySide6 widgets,
theme tokens, accessibility text, reduced-motion behavior, and immutable view
projections. It may call application ports through the existing view-model
boundary, but it must not import OTA implementations, debug backends, vendor
SDKs, sockets, `pyserial`, or cryptographic keys.

New OTA, RTT, or J-Link capabilities must first expose typed application/domain
DTOs and explicit capability state. The UI may then render those DTOs without
opening a device handle or deciding protocol/security policy itself.

`embedded_extension_panel.py` is the read-only capability page for that rule. It
renders `application.extension_capabilities` DTOs and labels XMODEM/YMODEM/TFTP
and AES-GCM/CCM as contract-only, while RTT/J-Link remains attach-only. It owns
no action, timer, device handle, key material, vendor dependency, or upgrade
policy.

`embedded_station_overview.py` owns the read-only station overview and its
resource-free route glyph. `ExtensionPanelWidgets` is the typed composition
bundle for the extension page; `controllers/workspace.py` wires the overview
reference, while `controllers/lifecycle.py` fans out the existing shared
`MotionController` frame. The glyph has no second clock and always falls back
to a static route when motion is reduced, paused, hidden, minimized, or closing.

`workspace_scroll_hint.py` owns the presentation-only scroll discoverability
label in the shared route strip. It observes only the active settings page's
native vertical scrollbar, exposes a bounded top/middle/bottom/complete cue and
updates AccessibleDescription/tooltip text. It owns no page content, navigation,
business state, timer, or second scroll policy.

`workspace_context_surface.py` owns the read-only current-page cue in the same
route strip. `WorkspaceContextLabel` consumes only the existing bounded Tab
index, projects one of four workspace labels, and updates its state,
AccessibleDescription, and tooltip together. It is NoFocus and mouse-transparent;
it owns no navigation, ViewModel, transport, timer, OTA/debug dependency, or
second state source.

`workspace_bindings.py` owns the frozen `WorkspaceShellBindings` composition
bundle. The workspace builder is its only construction owner; bootstrap and
feature/lifecycle controllers consume the typed widget references through
`workspace_bindings_for()` instead of depending on a collection of implicit
`MainWindow._workspace_*` fields. The bundle contains no application state or
business callback.

`connection_bindings.py` owns the frozen `UartControlBindings` composition
bundle. `controllers/connection_builder.py` is the only construction owner;
connection, preset, protocol-timing, lifecycle, composition, and terminal
runtime consumers use `uart_bindings_for()` instead of reaching across the
window for UART widget fields. The bundle contains only Qt references and no
business state, callbacks, timers, transport handles, keys, or option catalog.
During incremental migration the builder may retain local compatibility fields;
new consumers must not add another dynamic window facade.

The same module also owns the frozen `NetworkControlBindings` bundle for the
shared TCP Client/Server, UDP, and RTT endpoint panel. The builder is the only
construction owner; connection/runtime, preset, composition, lifecycle,
commands, and terminal runtime consume `network_bindings_for()`. The bundle
contains widget references only. Server peer state, default-value flags,
transport configuration, and network policy remain in their existing owners.

The module also owns the frozen `BleControlBindings` bundle for the BLE GATT
panel. `controllers/connection_builder.py` is the only construction and signal
wiring owner; BLE selection, action, connection/runtime, preset, composition,
commands, lifecycle, and terminal runtime consume `ble_bindings_for()`. The
bundle contains only BLE widget references. Notification pending/ref/timer
state, device snapshots, backend handles, and write policy remain in their
existing owners; new consumers must not add a dynamic `_ble_*` widget facade.

`terminal_bindings.py` owns the frozen `TerminalControlBindings` bundle for
live observation, terminal preview, recording, send controls, quick commands,
and send history. `controllers/bootstrap.py` completes the composition-root
assembly after the terminal and send builders finish; terminal runtime,
connection, commands, send-context, lifecycle, composition, and focus owners
consume `terminal_bindings_for()`. The bundle contains Qt references only;
preview/history snapshots, timers, session/recording state, MotionController,
callbacks, and ViewModel facts remain in their existing owners.

`command_bindings.py` owns the frozen `CommandBatchControlBindings` bundle for
the command-management combo, batch action buttons, progress status, results
table, and empty-state card. `controllers/bootstrap.py` assembles it after the
workspace builder completes the command page; command, connection, selection,
composition, and lifecycle owners consume `command_batch_bindings_for()`.
Batch catalog/snapshot, execution policy, ViewModel state, timers, callbacks,
and transport handles remain outside the bundle. The terminal builder may keep
temporary dynamic fields while constructing widgets, but new cross-controller
code must use the typed accessor.

`connection_bindings.py` also owns the frozen `ConnectionShellBindings` bundle
for the shared transport/preset shell: control band, transport mode surface,
transport and preset selectors, preset context, save/delete actions, status rail,
and connect action. `controllers/connection_builder.py` assembles it before
refreshing the preset combo; connection/runtime, preset, command, composition,
derived/protocol, replay, source-scope, and lifecycle consumers use
`connection_shell_bindings_for()`. Preset catalog/store, session/ViewModel
state, callbacks, timers, transport handles, and policy remain outside the
bundle. Builder-local dynamic fields are construction-only compatibility fields;
new cross-controller code must use the typed accessor.

`chrome_bindings.py` owns the frozen `HeaderChromeBindings` bundle for the
global header's status cluster, motion controls, theme picker, palette swatch,
brand mark, and signal field. `controllers/workspace.py` assembles it before
the first theme projection; lifecycle and composition consume
`header_chrome_bindings_for()`. Preferences, `MotionController`, theme policy,
the application root, error bar, footer, and business state remain outside the
bundle. Header builder fields are construction-only compatibility references;
new cross-controller code must use the typed accessor.

`widgets.py` owns the single approximately-120Hz-target presentation cadence.
The timer is PreciseTimer-based with an 8ms periodic scheduler slot and
elapsed-time phase; the rounded scheduler target is not a guarantee of
compositor refresh or an exact display FPS. The
lifecycle controller filters hidden pages before fan-out and applies only a
deterministic decorative phase offset. `controllers/terminal.py` owns bounded
station layout spacing/row heights; it does not move layout policy into the
application or domain layers.

`pipeline_surface.py` contains the presentation-only `PipelineSurfaceLabel`.
It preserves the existing `pipelineSummary` text/accessibility/QSS contract and
draws a theme-aware flow rail from the shell's shared motion frame. It owns no
timer, controller, business state, transport dependency, or external asset.

`file_dialog_surface.py` contains the presentation-only `QFileDialog` theme
bridge. It keeps Qt's file-system model, path navigation, filters, keyboard
navigation, cancellation, acceptance, and overwrite semantics, while forcing
the themeable Qt surface and applying bounded theme QSS. It owns no path state,
file I/O, ViewModel, transport, or security policy; all open/save call sites
must use its explicit wrapper instead of the static unthemed convenience API.
