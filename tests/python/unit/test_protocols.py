from __future__ import annotations

import math
import struct

import pytest

from embeddebug.serial_station.core import SerialDispatcher
from embeddebug.serial_station.protocols import (
    FireWaterProtocol,
    JustFloatProtocol,
    RawDataProtocol,
    create_default_registry,
)


def test_raw_data_preserves_bytes_and_text():
    protocol = RawDataProtocol()

    events = protocol.feed(b"hello\xff")

    assert len(events) == 1
    assert events[0].type == "frame"
    assert events[0].protocol_name == "raw_data"
    assert events[0].raw == b"hello\xff"
    assert events[0].payload["text"] == "hello\ufffd"
    assert events[0].payload["size"] == 6


def test_fire_water_parses_split_header_and_measurement():
    protocol = FireWaterProtocol()

    assert protocol.feed(b"") == []
    assert protocol.feed(b"temp,volt") == []
    header = protocol.feed(b"\n")
    measurement = protocol.feed(b"fw:24.5,3.3\r\n")

    assert header[0].type == "header"
    assert header[0].payload["channelNames"] == ["temp", "volt"]
    assert measurement[0].type == "measurement"
    assert measurement[0].protocol_name == "fire_water"
    assert measurement[0].payload["values"] == [24.5, 3.3]
    assert measurement[0].payload["channelNames"] == ["temp", "volt"]
    assert measurement[0].payload["frameIndex"] == 1
    assert measurement[0].raw == b"fw:24.5,3.3"


def test_fire_water_invalid_numeric_becomes_nan():
    protocol = FireWaterProtocol()

    events = protocol.feed(b"1.0,bad,2.0\n")

    values = events[0].payload["values"]
    assert values[0] == 1.0
    assert math.isnan(values[1])
    assert values[2] == 2.0
    assert events[0].payload["channelNames"] == ["ch1", "ch2", "ch3"]


def test_fire_water_rejects_too_many_channels():
    protocol = FireWaterProtocol(max_channels=2)

    events = protocol.feed(b"1,2,3\n")

    assert events[0].type == "error"
    assert events[0].payload["reason"] == "too_many_channels"


def test_just_float_parses_split_and_multiple_frames():
    protocol = JustFloatProtocol()
    frame_one = struct.pack("<2f", 1.0, -2.5) + JustFloatProtocol.tail
    frame_two = struct.pack("<1f", 3.25) + JustFloatProtocol.tail

    assert protocol.feed(frame_one[:3]) == []
    events = protocol.feed(frame_one[3:] + frame_two)

    assert len(events) == 2
    assert events[0].type == "measurement"
    assert events[0].payload["values"] == pytest.approx([1.0, -2.5])
    assert events[0].payload["channelCount"] == 2
    assert events[0].payload["frameIndex"] == 1
    assert events[1].payload["values"] == pytest.approx([3.25])
    assert events[1].payload["frameIndex"] == 2


def test_just_float_reports_invalid_payload_and_resets_index():
    protocol = JustFloatProtocol()

    events = protocol.feed(b"\x01\x02" + JustFloatProtocol.tail)
    protocol.feed(struct.pack("<1f", 1.0) + JustFloatProtocol.tail)
    protocol.reset()
    after_reset = protocol.feed(struct.pack("<1f", 2.0) + JustFloatProtocol.tail)

    assert events[0].type == "error"
    assert events[0].payload["reason"] == "invalid_payload_length"
    assert after_reset[0].payload["frameIndex"] == 1


def test_default_registry_and_dispatcher():
    registry = create_default_registry()

    assert registry.names() == ["fire_water", "just_float", "raw_data"]

    dispatcher = SerialDispatcher(registry.create("raw_data"))
    raw_event = dispatcher.feed(b"abc")[0]
    dispatcher.set_protocol(registry.create("fire_water"))
    measurement = dispatcher.feed(b"1,2\n")[0]

    assert raw_event.protocol_name == "raw_data"
    assert dispatcher.protocol_name == "fire_water"
    assert measurement.payload["values"] == [1.0, 2.0]
