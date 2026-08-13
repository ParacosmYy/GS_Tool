# ARCH-6n 连接控件刷新组合根交接

日期：2026-08-10  
状态：源码、静态门、真实组合根 offscreen vector 与 onefile 交付均已完成  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付结果

`controllers/connection.py` 仍是连接控件 enable/hint/busy projection 的唯一 owner。组合根 `controllers/bootstrap.py` 通过
`partial(update_connection_controls, window)` 绑定 `window._refresh_connection_controls`，所有原有调用迁移到该 callback；
`MainWindow._update_connection_controls`、对应 `SessionState`/controller import 已删除。

本轮不改变 session/replay gate、BLE/TCP Server readiness、首屏 hydration、Qt signal 参数、关闭生命周期、accessibility 或业务协议。

## 角色调用与审查

第一轮六个产品/架构/UI/开发/验证/打包角色均已调用但窗口内超时后关闭；第二轮六个角色只用于 import 顺序修正，也均超时后关闭。
独立质量复核 `019fec0b-98ed-7762-b7ab-9c2569497fcb` 在窗口内超时后关闭。没有把任何超时当作通过。

父代理五轴审查记录：correctness、readability/simplification、architecture、security、performance 均完成；最小实现是单一命名 callback，
没有新增状态源、无限制 callback map、mixin、timer、线程、网络、存储、密钥或依赖。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
ARCH6N_COMPOSITION_PASS callback=bound facade_removed=True state=closed
```

向量为 Qt offscreen 真实组合根，主窗口未显示；未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收。
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
ARCH6N_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-arch-6n
size: 47,894,630 bytes
SHA-256: A56986CF93FB1B3B100C78B3AA172ECF5542BB14BF4BABB8D07537DB32925DB7
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
