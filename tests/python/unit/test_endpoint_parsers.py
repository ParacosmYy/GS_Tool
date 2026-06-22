"""TCP/UDP endpoint parser 边界扩展单元测试。

tcp_client._parse_endpoint 与 udp_datagram._parse_endpoint 是纯函数，负责把
``host:port`` 字符串解析成 (host, port) 元组。此前 test_tcp_client_transport /
test_udp_datagram_transport 仅通过 transport.open 间接覆盖 invalid endpoint
路径，未直接断言 parser 的所有分支。

覆盖：
- 合法 endpoint（IPv4 + 端口）。
- 缺冒号 / 缺 host / 缺 port → ValueError("*_endpoint_requires_host_port")。
- 非数字端口 → ValueError("*_endpoint_port_invalid")。
- 端口越界（0 / 负数 / 65536）→ ValueError("*_endpoint_port_invalid")。
- IPv6 风格（含多个冒号）rpartition 取最后一个冒号后的端口。
- 错误 code 前缀正确（tcp_* vs udp_*）。
"""

from __future__ import annotations

import pytest

from embeddebug.serial_station.drivers.tcp_client import _parse_endpoint as tcp_parse
from embeddebug.serial_station.drivers.udp_datagram import (
    _parse_endpoint as udp_parse,
)


# ── 合法 endpoint ────────────────────────────────────────────────────────


def test_tcp_parse_valid_ipv4():
    assert tcp_parse("127.0.0.1:8080") == ("127.0.0.1", 8080)


def test_udp_parse_valid_ipv4():
    assert udp_parse("192.168.1.10:5000") == ("192.168.1.10", 5000)


def test_tcp_parse_boundary_ports():
    """端口边界 1 和 65535 合法。"""

    assert tcp_parse("host:1") == ("host", 1)
    assert tcp_parse("host:65535") == ("host", 65535)


def test_udp_parse_boundary_ports():
    assert udp_parse("host:1") == ("host", 1)
    assert udp_parse("host:65535") == ("host", 65535)


def test_tcp_parse_hostname_with_dots():
    """主机名含点（如 example.com）合法。"""

    assert tcp_parse("example.com:443") == ("example.com", 443)


# ── 缺字段：requires_host_port ────────────────────────────────────────────


def test_tcp_parse_missing_colon_raises_requires():
    with pytest.raises(ValueError, match="tcp_endpoint_requires_host_port"):
        tcp_parse("127.0.0.1")


def test_udp_parse_missing_colon_raises_requires():
    with pytest.raises(ValueError, match="udp_endpoint_requires_host_port"):
        udp_parse("127.0.0.1")


def test_tcp_parse_missing_host_raises_requires():
    """缺 host（":8080"）→ requires_host_port。"""

    with pytest.raises(ValueError, match="tcp_endpoint_requires_host_port"):
        tcp_parse(":8080")


def test_udp_parse_missing_host_raises_requires():
    with pytest.raises(ValueError, match="udp_endpoint_requires_host_port"):
        udp_parse(":8080")


def test_tcp_parse_missing_port_raises_requires():
    """缺 port（"host:"）→ requires_host_port。"""

    with pytest.raises(ValueError, match="tcp_endpoint_requires_host_port"):
        tcp_parse("host:")


def test_udp_parse_missing_port_raises_requires():
    with pytest.raises(ValueError, match="udp_endpoint_requires_host_port"):
        udp_parse("host:")


def test_tcp_parse_empty_string_raises_requires():
    with pytest.raises(ValueError, match="tcp_endpoint_requires_host_port"):
        tcp_parse("")


def test_udp_parse_empty_string_raises_requires():
    with pytest.raises(ValueError, match="udp_endpoint_requires_host_port"):
        udp_parse("")


# ── 非数字端口：port_invalid ──────────────────────────────────────────────


def test_tcp_parse_non_numeric_port_raises_invalid():
    with pytest.raises(ValueError, match="tcp_endpoint_port_invalid"):
        tcp_parse("host:abc")


def test_udp_parse_non_numeric_port_raises_invalid():
    with pytest.raises(ValueError, match="udp_endpoint_port_invalid"):
        udp_parse("host:abc")


# ── 端口越界：port_invalid ────────────────────────────────────────────────


def test_tcp_parse_port_zero_raises_invalid():
    with pytest.raises(ValueError, match="tcp_endpoint_port_invalid"):
        tcp_parse("host:0")


def test_udp_parse_port_zero_raises_invalid():
    with pytest.raises(ValueError, match="udp_endpoint_port_invalid"):
        udp_parse("host:0")


def test_tcp_parse_port_negative_raises_invalid():
    with pytest.raises(ValueError, match="tcp_endpoint_port_invalid"):
        tcp_parse("host:-1")


def test_udp_parse_port_negative_raises_invalid():
    with pytest.raises(ValueError, match="udp_endpoint_port_invalid"):
        udp_parse("host:-1")


def test_tcp_parse_port_overflow_raises_invalid():
    """端口 65536 越界（> 65535）。"""

    with pytest.raises(ValueError, match="tcp_endpoint_port_invalid"):
        tcp_parse("host:65536")


def test_udp_parse_port_overflow_raises_invalid():
    with pytest.raises(ValueError, match="udp_endpoint_port_invalid"):
        udp_parse("host:65536")


# ── 多冒号（IPv6 / 多段主机名）rpartition 语义 ──────────────────────────


def test_tcp_parse_multi_colon_takes_last_as_port():
    """含多个冒号时 rpartition 取最后一个冒号后的端口（IPv6 风格 host）。"""

    host, port = tcp_parse("::1:8080")
    assert port == 8080
    assert host == "::1"


def test_udp_parse_multi_colon_takes_last_as_port():
    host, port = udp_parse("a:b:9999")
    assert port == 9999
    assert host == "a:b"


# ── 错误 code 前缀隔离（tcp_* vs udp_* 不串） ───────────────────────────


def test_tcp_and_udp_error_codes_do_not_cross():
    """tcp parser 不发出 udp_* code，反之亦然。"""

    with pytest.raises(ValueError) as exc_info:
        tcp_parse("bad")
    assert "udp_" not in str(exc_info.value)

    with pytest.raises(ValueError) as exc_info:
        udp_parse("bad")
    assert "tcp_" not in str(exc_info.value)
