from __future__ import annotations

import inspect

from embeddebug.serial_station.ui import connection_actions, tcp_controls


def test_tcp_connection_action_lives_with_connection_actions():
    assert hasattr(connection_actions, "connect_tcp")
    assert not hasattr(tcp_controls, "connect_tcp_from_controls")

    source = inspect.getsource(tcp_controls)
    assert "connect_tcp_result" not in source
    assert "_status_label.setText" not in source
