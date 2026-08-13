# ARCH-6p MainWindow Composition Shell 交接

日期：2026-08-10  
状态：源码、静态门、真实组合根 offscreen vector 与 onefile 交付均已完成  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付结果

`controllers/bootstrap.py` 按 feature owner 增加显式 callback wiring：protocol、derived、replay、BLE、terminal、lifecycle 各自使用
`partial(owner_function, window)`，并在 timer/signal 使用前完成绑定。`MainWindow` 现在只包含构造和四个 Qt lifecycle override，
不再保存协议、派生、回放、BLE、终端或生命周期业务转发。

## 角色调用与审查

第一轮六个角色、第二轮六个 import-fix 角色和独立复核均已调用；均在窗口内超时后关闭，未将超时当作通过。父代理完成 correctness、readability/simplification、architecture、security、performance 五轴审查。
本轮无嵌入式 C/C++/MCU/固件改动，适用性 N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
ARCH6P_COMPOSITION_PASS callbacks=14 main_window_lifecycle_methods=4 state=closed
```

向量为 Qt offscreen 真实组合根，主窗口未显示，已完成关闭和 worker 清理；未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收。
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
ARCH6P_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-arch-6p
size: 47,891,078 bytes
SHA-256: 70604D4065895FAF07EE86D675FFF8845E7020C82556D0D18ED41188FCA6CA86
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
