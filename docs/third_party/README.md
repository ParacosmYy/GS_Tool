# Third-party notices

当前锁定依赖的归属清单见 [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)。它是基于 `pyproject.toml`/`uv.lock` 的版本盘点，不是法律意见。

发行前仍需从实际发行物和官方许可文本复核：

- PySide6/Qt；
- pyserial；
- bleak 及 Windows Runtime 绑定；
- PyInstaller 及其传递依赖；
- 外部 RTT 工具、供应商 DLL 和授权条款（SerialForge 当前只做 Telnet attach，不分发它们）。

当前不包含 SEGGER/J-Link SDK、DLL 或 RTT 专用二进制；RTT 代码已作为最后一个能力接入，
用户仍需自行安装并启动提供 RTT Telnet 的 J-Link 工具。
