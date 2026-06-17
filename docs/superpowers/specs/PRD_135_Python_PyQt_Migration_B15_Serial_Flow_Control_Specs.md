# PRD-135 B15 - Python/PyQt Serial Flow Control

## Scope

- Add serial flow control selection to the Python/PyQt Serial Station lane.
- Carry the selected value through UI, controller, `SerialPortConfig`, and `QtSerialPortTransport`.
- Persist and restore the value in profile JSON as `transport.flowControl`.

## User Story

As a Serial Station user, I can choose no flow control, hardware RTS/CTS, or software XON/XOFF before opening a real serial port so that the Python/PyQt launcher can cover common device connection requirements without falling back to the legacy C++ path.

## Acceptance Criteria

- `serialStationFlowControlCombo` exists in the PyQt UI and defaults to `None`.
- Connecting a selected serial port passes `none`, `hardware`, or `software` to `SerialWorkbenchController.connect_serial`.
- `SerialPortConfig` stores `flow_control`.
- `QtSerialPortTransport.configure()` maps flow control to `QSerialPort.FlowControl`.
- Saved profiles include `flowControl`; loaded profiles restore the combo selection.
- Existing fake transport, replay, waveform, and packaging smoke paths remain green.

## Non-Goals

- No D4 hardware validation in this batch.
- No RTS/DTR line toggling panel.
- No live modem-signal monitor.
