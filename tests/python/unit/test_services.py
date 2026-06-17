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


def test_measurement_export_service_result_reports_write_failure(tmp_path):
    blocked_parent = tmp_path / "blocked"
    blocked_parent.write_text("not a directory", encoding="utf-8")
    service = SerialMeasurementExportService()

    result = service.export_csv_result(blocked_parent / "measurements.csv", [])

    assert result.failed
    assert result.error_code == "measurement_export_failed"
    assert "blocked" in result.message


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


def test_profile_service_result_reports_save_and_load_failures(tmp_path):
    blocked_parent = tmp_path / "blocked"
    blocked_parent.write_text("not a directory", encoding="utf-8")
    service = SerialProfileService()

    save_result = service.save_result(blocked_parent / "profile.json", {"name": "bad"})
    load_result = service.load_result(tmp_path / "missing.json")

    assert save_result.failed
    assert save_result.error_code == "profile_save_failed"
    assert "blocked" in save_result.message
    assert load_result.failed
    assert load_result.error_code == "profile_load_failed"
    assert "missing.json" in load_result.message


def test_replay_service_result_reports_load_failure(tmp_path):
    service = SerialReplayService()

    result = service.load_events_result(tmp_path / "missing.jsonl")

    assert result.failed
    assert result.error_code == "replay_load_failed"
    assert "missing.jsonl" in result.message
