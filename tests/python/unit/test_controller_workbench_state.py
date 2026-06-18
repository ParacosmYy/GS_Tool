from __future__ import annotations

from embeddebug.serial_station.controllers import controller_workbench_state
from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry


def test_workbench_state_exposes_read_only_entry_and_history_snapshots():
    state = controller_workbench_state.create_workbench_state()
    entry = SerialWorkbenchLogEntry(direction="tx", text="AT", raw=b"AT")

    state.entries.append(entry)
    state.command_history.append("AT")

    assert controller_workbench_state.entries_snapshot(state) == (entry,)
    assert controller_workbench_state.command_history_snapshot(state) == ("AT",)


def test_workbench_state_clear_entries_keeps_command_history():
    state = controller_workbench_state.create_workbench_state()
    state.entries.append(SerialWorkbenchLogEntry(direction="rx", text="OK", raw=b"OK"))
    state.command_history.append("AT")

    controller_workbench_state.clear_entries(state)

    assert controller_workbench_state.entries_snapshot(state) == ()
    assert controller_workbench_state.command_history_snapshot(state) == ("AT",)
