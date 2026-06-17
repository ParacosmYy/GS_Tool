from __future__ import annotations

import csv
import json

from embeddebug.serial_station.protocols import ProtocolEvent
from embeddebug.serial_station.services import (
    SerialLogService,
    SerialMeasurementExportService,
    SerialProfileService,
    SerialReplayService,
)


def test_log_service_appends_jsonl_and_replay_restores_events(tmp_path):
    log_path = tmp_path / "session.jsonl"
    log_service = SerialLogService(log_path)
    replay_service = SerialReplayService()

    event = ProtocolEvent(
        type="measurement",
        protocol_name="fire_water",
        payload={"values": [1.0, 2.0], "frameIndex": 7},
        raw=b"1,2",
    )

    log_service.append(event)
    log_service.append(
        ProtocolEvent(
            type="frame",
            protocol_name="raw_data",
            payload={"text": "ok"},
            raw=b"ok",
        )
    )

    lines = log_path.read_text(encoding="utf-8").splitlines()
    assert len(lines) == 2
    assert json.loads(lines[0])["rawHex"] == "312c32"

    events = replay_service.load_events(log_path)
    assert events[0] == event
    assert events[1].protocol_name == "raw_data"
    assert events[1].raw == b"ok"


def test_measurement_export_service_writes_ordered_csv(tmp_path):
    export_path = tmp_path / "measurements.csv"
    service = SerialMeasurementExportService()
    events = [
        ProtocolEvent(
            type="measurement",
            protocol_name="just_float",
            payload={
                "frameIndex": 1,
                "channelNames": ["temp", "volt"],
                "values": [24.5, 3.3],
            },
            raw=b"",
        ),
        ProtocolEvent(
            type="frame",
            protocol_name="raw_data",
            payload={},
            raw=b"ignored",
        ),
    ]

    service.export_csv(export_path, events)

    rows = list(csv.reader(export_path.read_text(encoding="utf-8").splitlines()))
    assert rows == [
        ["frameIndex", "protocol", "temp", "volt"],
        ["1", "just_float", "24.5", "3.3"],
    ]


def test_profile_service_round_trips_json_profile(tmp_path):
    profile_path = tmp_path / "profile.json"
    service = SerialProfileService()
    profile = {
        "name": "bench-uart",
        "transport": {"port": "COM3", "baud": 115200},
        "protocol": "just_float",
    }

    service.save(profile_path, profile)

    assert service.load(profile_path) == profile
    assert json.loads(profile_path.read_text(encoding="utf-8"))["protocol"] == "just_float"
