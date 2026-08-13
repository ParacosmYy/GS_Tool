# ARCH-6o Transport / Protocol Framing 组合根交接

日期：2026-08-10  
状态：源码、静态门、真实组合根 offscreen vector 与 onefile 交付均已完成  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付结果

`controllers/bootstrap.py` 通过两个显式命名 callback 绑定 transport 与 protocol framing owner：

```python
window._on_transport_changed = partial(on_transport_changed, window)
window._on_protocol_framing_changed = partial(on_protocol_framing_changed, window)
```

`MainWindow` 删除两个纯转发方法和对应 imports；connection builder、preset、replay、composition、protocol config 的现有调用保持不变。
transport panel、六种链路、connection refresh、protocol draft/status/timing、首屏 hydration、Qt payload、lifecycle 与 accessibility 均未改变。

## 角色调用与审查

六个产品/架构/UI/开发/验证/打包角色均已调用但窗口内超时后关闭；独立质量复核 `019fec10-2795-7383-bb26-2239d853e6eb` 同样超时后关闭。
没有把任何超时当作通过。父代理完成 correctness、readability/simplification、architecture、security、performance 五轴审查；嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
ARCH6O_COMPOSITION_PASS transport=bound framing=bound facade_removed=True state=closed
```

向量为 Qt offscreen 真实组合根，主窗口未显示；未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收。
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
ARCH6O_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-arch-6o
size: 47,893,668 bytes
SHA-256: 4DC716FA8B652158058F777284968B8D5E8CC6C3C12DDEF52CBE0B3A919891F8
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
