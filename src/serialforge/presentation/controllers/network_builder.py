"""TCP, UDP, and RTT endpoint surface composition.

The builder owns only network-facing widgets and their typed binding bundle.
Connection lifecycle, endpoint validation, and transport selection remain in
the controller layer that consumes ``NetworkControlBindings``.
"""

from __future__ import annotations

from ...domain.models import (
    DEFAULT_UDP_DATAGRAM_BYTES,
    MAX_TCP_SERVER_CLIENTS,
    MAX_UDP_DATAGRAM_BYTES,
)
from ..bounded_value_combo import (
    TCP_CLIENT_OPTION_VALUES,
    UDP_DATAGRAM_OPTION_VALUES,
    BoundedIntCombo,
)
from ..connection_bindings import NetworkControlBindings
from ..qt import (
    QCheckBox,
    QComboBox,
    QGridLayout,
    QLabel,
    QLineEdit,
    QPlainTextEdit,
    QSpinBox,
    QWidget,
)
from .composition import timeout_combo
from .connection_primitives import (
    configure_bounded_combo,
    configure_responsive_hint,
    field_label,
    section_label,
)


def build_network_panel(window) -> NetworkControlBindings:
    """Build the shared TCP/UDP/RTT panel without changing its runtime contract."""

    network_panel = QWidget(window)
    network_layout = QGridLayout(network_panel)
    network_layout.setContentsMargins(0, 0, 0, 0)
    network_layout.setHorizontalSpacing(12)
    network_layout.setVerticalSpacing(10)

    window._remote_host_label = field_label("远端主机")
    network_layout.addWidget(window._remote_host_label, 0, 0)
    window._network_host = QLineEdit("127.0.0.1")
    window._network_host.setAccessibleName("网络远端主机")
    window._network_host.setPlaceholderText("主机名或 IPv4 地址")
    window._network_host.setToolTip("TCP Client/UDP 连接的目标主机名或 IPv4 地址。")
    window._network_host.setAccessibleDescription(
        "输入 TCP Client 或 UDP 连接的目标主机名或 IPv4 地址。"
    )
    network_layout.addWidget(window._network_host, 0, 1, 1, 3)
    window._remote_port_label = field_label("远端端口")
    network_layout.addWidget(window._remote_port_label, 0, 4)
    window._network_port = QSpinBox()
    window._network_port.setAccessibleName("网络远端端口")
    window._network_port.setRange(1, 65_535)
    window._network_port.setValue(9_000)
    window._network_port.setToolTip("选择 TCP Client/UDP 连接使用的远端端口，范围 1–65535。")
    window._network_port.setAccessibleDescription("选择网络连接的远端端口，范围为 1 到 65535。")
    network_layout.addWidget(window._network_port, 0, 5)

    window._local_host_label = field_label("本地绑定")
    network_layout.addWidget(window._local_host_label, 1, 0)
    window._network_local_host = QLineEdit("0.0.0.0")
    window._network_local_host.setAccessibleName("网络本地绑定主机")
    window._network_local_host.setToolTip(
        "选择监听或本地绑定的主机地址；0.0.0.0 表示绑定所有网卡。"
    )
    window._network_local_host.setAccessibleDescription(
        "输入 TCP Server、UDP 本地绑定的主机地址；0.0.0.0 表示绑定所有网卡。"
    )
    network_layout.addWidget(window._network_local_host, 1, 1, 1, 3)
    window._local_port_label = field_label("本地端口")
    network_layout.addWidget(window._local_port_label, 1, 4)
    window._network_local_port = QSpinBox()
    window._network_local_port.setAccessibleName("网络本地绑定端口")
    window._network_local_port.setRange(0, 65_535)
    window._network_local_port.setValue(0)
    window._network_local_port.setToolTip("0 表示由系统分配临时端口")
    window._network_local_port.setAccessibleDescription(
        "选择本地绑定端口；0 表示由系统分配临时端口，范围为 0 到 65535。"
    )
    network_layout.addWidget(window._network_local_port, 1, 5)

    window._connect_timeout_label = field_label("连接超时")
    network_layout.addWidget(window._connect_timeout_label, 2, 0)
    window._network_connect_timeout = timeout_combo(3.0, 60.0, minimum=0.001)
    window._network_connect_timeout.setAccessibleName("网络连接或监听超时")
    window._network_connect_timeout.setToolTip("选择网络连接或 TCP Server 监听等待的最长时间。")
    window._network_connect_timeout.setAccessibleDescription("选择网络连接或监听等待的最长时间，单位为秒。")
    network_layout.addWidget(window._network_connect_timeout, 2, 1)
    network_layout.addWidget(field_label("读超时"), 2, 2)
    window._network_read_timeout = timeout_combo(0.2, 60.0, minimum=0.001)
    window._network_read_timeout.setAccessibleName("网络读超时")
    window._network_read_timeout.setToolTip("选择网络读取等待的最长时间。")
    window._network_read_timeout.setAccessibleDescription("选择网络读取等待的最长时间，单位为秒。")
    network_layout.addWidget(window._network_read_timeout, 2, 3)
    network_layout.addWidget(field_label("写超时"), 2, 4)
    window._network_write_timeout = timeout_combo(3.0, 60.0, minimum=0.001)
    window._network_write_timeout.setAccessibleName("网络写超时")
    window._network_write_timeout.setToolTip("选择网络写入等待的最长时间。")
    window._network_write_timeout.setAccessibleDescription("选择网络写入等待的最长时间，单位为秒。")
    network_layout.addWidget(window._network_write_timeout, 2, 5)

    window._udp_limit_label = field_label("最大报文")
    network_layout.addWidget(window._udp_limit_label, 3, 0)
    window._udp_limit = BoundedIntCombo(
        1,
        MAX_UDP_DATAGRAM_BYTES,
        values=UDP_DATAGRAM_OPTION_VALUES,
        suffix=" B",
    )
    window._udp_limit.setAccessibleName("UDP 最大 datagram 大小")
    window._udp_limit.setValue(DEFAULT_UDP_DATAGRAM_BYTES)
    window._udp_limit.setToolTip("选择 UDP 单个报文允许的最大字节数。")
    window._udp_limit.setAccessibleDescription(
        "选择 UDP 单个报文允许的最大字节数，范围由传输层上限约束。"
    )
    network_layout.addWidget(window._udp_limit, 3, 1)
    window._network_hint = QLabel("TCP 是连续字节流；UDP 每次接收保留一个报文边界。")
    window._network_hint.setProperty("role", "muted")
    configure_responsive_hint(window._network_hint)
    network_layout.addWidget(window._network_hint, 3, 2, 1, 4)

    window._rtt_channel_label = field_label("RTT 通道")
    network_layout.addWidget(window._rtt_channel_label, 4, 0)
    window._rtt_channel = QComboBox()
    window._rtt_channel.setAccessibleName("RTT 通道")
    window._rtt_channel.setEditable(False)
    window._rtt_channel.addItem("0 · 终端", 0)
    window._rtt_channel.addItem("1 · 数据", 1)
    configure_bounded_combo(window._rtt_channel, minimum_width=120, maximum_width=160)
    window._rtt_channel.setToolTip("选择已有 SEGGER RTT Telnet 服务的通道；不会启动 vendor 工具。")
    window._rtt_channel.setAccessibleDescription(
        "从已存在的 SEGGER RTT Telnet 服务中选择通道；本工具不会自动启动 J-Link 或 vendor 工具。"
    )
    network_layout.addWidget(window._rtt_channel, 4, 1)
    window._rtt_hint = QLabel("RTT 只连接已有 Telnet 服务；请先启动 J-Link Commander/GDB Server。")
    window._rtt_hint.setProperty("role", "muted")
    configure_responsive_hint(window._rtt_hint)
    network_layout.addWidget(window._rtt_hint, 4, 2, 1, 4)

    window._server_allowlist_label = field_label("允许客户端")
    network_layout.addWidget(window._server_allowlist_label, 5, 0)
    window._server_allowlist = QPlainTextEdit()
    window._server_allowlist.setAccessibleName("TCP Server 客户端允许列表")
    window._server_allowlist.setPlaceholderText("每行一个 IPv4 或 CIDR，例如：127.0.0.1/32")
    window._server_allowlist.setMaximumHeight(70)
    window._server_allowlist.setToolTip(
        "每行填写一个允许连接的 IPv4 地址或 CIDR；留空表示不额外放行。"
    )
    window._server_allowlist.setAccessibleDescription(
        "填写 TCP Server 客户端允许列表，每行一个 IPv4 地址或 CIDR；留空表示不额外放行。"
    )
    network_layout.addWidget(window._server_allowlist, 5, 1, 1, 5)
    window._server_lan_confirm = QCheckBox("我确认允许 LAN 监听")
    window._server_lan_confirm.setAccessibleName("确认允许 TCP Server LAN 监听")
    window._server_lan_confirm.setToolTip("仅在明确需要对局域网提供 TCP Server 时勾选。")
    window._server_lan_confirm.setAccessibleDescription(
        "确认允许 TCP Server 绑定非回环地址并接受局域网连接。"
    )
    network_layout.addWidget(window._server_lan_confirm, 6, 0, 1, 3)
    window._server_max_clients = BoundedIntCombo(
        1,
        MAX_TCP_SERVER_CLIENTS,
        values=TCP_CLIENT_OPTION_VALUES,
        suffix=" 个",
    )
    window._server_max_clients.setAccessibleName("TCP Server 最大客户端数")
    window._server_max_clients.setValue(4)
    window._server_max_clients.setToolTip("选择每个 TCP Server 允许同时保持的最大客户端数。")
    window._server_max_clients.setAccessibleDescription(
        "选择每个 TCP Server 允许同时保持的最大客户端数，范围由安全上限约束。"
    )
    window._server_max_clients_label = QLabel("最大客户端")
    window._server_max_clients_label.setObjectName("serverMaxClientsLabel")
    window._server_max_clients_label.setProperty("role", "muted")
    window._server_max_clients_label.setAccessibleName("TCP Server 最大 client 数标签")
    network_layout.addWidget(window._server_max_clients_label, 6, 3)
    network_layout.addWidget(window._server_max_clients, 6, 4)
    window._server_peer_label = field_label("发送目标")
    network_layout.addWidget(window._server_peer_label, 7, 0)
    window._server_peer_combo = QComboBox()
    window._server_peer_combo.setAccessibleName("TCP Server 客户端发送目标")
    window._server_peer_combo.setPlaceholderText("暂无已连接客户端")
    window._server_peer_combo.setToolTip(
        "选择 TCP Server 当前发送数据的客户端；连接后才会出现可选项。"
    )
    window._server_peer_combo.setAccessibleDescription(
        "选择 TCP Server 当前发送数据的客户端；没有已连接客户端时不可选择。"
    )
    configure_bounded_combo(window._server_peer_combo, minimum_width=160, maximum_width=300)
    window._server_peer_combo.currentIndexChanged.connect(window._on_server_target_changed)
    network_layout.addWidget(window._server_peer_combo, 7, 1, 1, 5)
    network_panel.setProperty("role", "surface")
    window._network_panel = network_panel
    window._network_title = section_label("网络端点")
    window._network_bindings = NetworkControlBindings(
        panel=network_panel,
        title=window._network_title,
        remote_host_label=window._remote_host_label,
        remote_host=window._network_host,
        remote_port_label=window._remote_port_label,
        remote_port=window._network_port,
        local_host_label=window._local_host_label,
        local_host=window._network_local_host,
        local_port_label=window._local_port_label,
        local_port=window._network_local_port,
        connect_timeout_label=window._connect_timeout_label,
        connect_timeout=window._network_connect_timeout,
        read_timeout=window._network_read_timeout,
        write_timeout=window._network_write_timeout,
        udp_limit_label=window._udp_limit_label,
        udp_limit=window._udp_limit,
        hint=window._network_hint,
        rtt_channel_label=window._rtt_channel_label,
        rtt_channel=window._rtt_channel,
        rtt_hint=window._rtt_hint,
        server_allowlist_label=window._server_allowlist_label,
        server_allowlist=window._server_allowlist,
        server_lan_confirm=window._server_lan_confirm,
        server_max_clients_label=window._server_max_clients_label,
        server_max_clients=window._server_max_clients,
        server_peer_label=window._server_peer_label,
        server_peer_combo=window._server_peer_combo,
    )
    return window._network_bindings


__all__ = ["build_network_panel"]
