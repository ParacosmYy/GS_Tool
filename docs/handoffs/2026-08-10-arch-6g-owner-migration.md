# ARCH-6g：状态来源与 TCP Server readiness owner 归还

日期：2026-08-10  
范围：删除两个仅单调用点的 MainWindow facade；不改变连接按钮、LAN 授权条件、状态 surface source、Qt signal 或生命周期。

## 交付

- `status_surface_source()` 归还 `controllers/status_surfaces.py`，bootstrap 以显式 source callback 注入
  `StatusSurfaceController`。
- `tcp_server_readiness()` 归还 `controllers/connection.py`，connection controls 直接调用；保留 IPv4、回环、LAN confirm、
  allowlist 数量和原有错误文案。
- 删除 `MainWindow._status_surface_source`、`MainWindow._tcp_server_readiness` 及 lifecycle 中不属于 owner 的函数；
  MainWindow 从 782 行降至 770 行。

## 架构审查

- 架构师 Luna max `019fe9dd-5a2c-7a23-a36e-4a60c9d98521` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。
- 父代理按已核对的最小依赖图实现：bootstrap 只注入 source callback，connection owner 自持网络 readiness；没有引入
  controller 循环依赖或放宽网络授权 gate。

## 验证

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (119 files <= 1000)
ARCH6G_OWNER_MIGRATION                       pass
  facades=2-removed; status-source=owner; tcp-readiness=owner; loopback=pass; lan-gate=pass
  themes=3; sizes=2; hscroll=0; motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6g_owner_migration.png
PACKAGE_ARCH6G_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,788,341 bytes
  SHA256: `840F1EC82602147B021E513BC164D79732D53C9F5498DBAF14841AD9F779D103`
  provenance/archive/hash: pass; archive listing SHA256 `9836C898423B2A3E131EC2C30EBC55D40B70ED775FDCE3FE442ED8DFF05B2B52`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏运行出现 PySide6 fonts 目录缺失提示，中文显示为方框；这不代表 Windows 打包环境字体缺失。未启动持续 GUI/EXE、真实
UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件；没有创建、修改或运行 unit test、mock、fixture、harness。项目为 Python/PySide6
桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
