from __future__ import annotations

from embeddebug.serial_station.ui.endpoint_control_text import endpoint_control_text


def test_endpoint_control_text_describes_tcp_controls():
    text = endpoint_control_text("tcp")

    assert text.host_placeholder == "TCP host"
    assert text.host_tooltip == "TCP host name or address"
    assert text.port_placeholder == "TCP port"
    assert text.port_tooltip == "TCP port number"
    assert text.connect_label == "Connect TCP"
    assert text.connect_tooltip == "Open a TCP client connection"


def test_endpoint_control_text_describes_udp_controls():
    text = endpoint_control_text("udp")

    assert text.host_placeholder == "UDP host"
    assert text.host_tooltip == "UDP remote host name or address"
    assert text.port_placeholder == "UDP port"
    assert text.port_tooltip == "UDP remote port number"
    assert text.connect_label == "Connect UDP"
    assert text.connect_tooltip == "Open a UDP datagram connection"
