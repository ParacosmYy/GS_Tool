from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.app.main import build_main_window, main


def test_main_window_smoke(qtbot):
    window = build_main_window()
    qtbot.addWidget(window)

    window.show()
    qtbot.waitUntil(lambda: window.isVisible(), timeout=1000)

    assert window.objectName() == "embeddebugPySerialStationWindow"
    assert window.centralWidget().objectName() == "serialStationPyRoot"


def test_smoke_entry_returns_zero():
    assert main(["--smoke"]) == 0
