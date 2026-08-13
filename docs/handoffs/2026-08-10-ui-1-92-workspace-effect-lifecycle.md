# UI-1.92 工作区 Tab 淡入 effect 生命周期交接

日期：2026-08-10  
范围：`workspace_runtime.py` 的一次性 page opacity effect 清理

## 结果

- 新增 `_release_workspace_effect()`，统一恢复 opacity、Qt validity 检查和 page effect detach。
- stop、自然完成、快速切换、低动效/暂停、隐藏、最小化和关闭路径不再留下 page graphics effect。
- 不改变 Tab index、route、焦点、无障碍、业务状态、共享 MotionController、动画时长或新增计时器。

## 角色与独立复核

```text
产品角色       019fec4b-8e28-7c91-8966-e21760978ea5  called; wait timed out; closed
架构角色       019fec4b-8e71-78f2-8615-999d28f4d8fe  called; wait timed out; closed
UI 设计角色    019fec4b-8ed5-72e2-a8f0-e06705a3ff81  called; wait timed out; closed
开发角色       019fec4b-8f1c-7091-9154-6cd917d6d68a  called; wait timed out; closed
验证角色       019fec4b-8f68-7381-a6a8-99b3d65bf02f  called; wait timed out; closed
打包角色       019fec4b-8fb7-7160-a9be-cb60f5f556fd  called; wait timed out; closed
独立质量复核   019fec4d-33a2-73a3-a3a7-f408e893211d  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认 stop/finish
都清除 page effect 且 rapid switch 不复用已解绑对象；readability/simplicity 确认释放逻辑集中为一个
helper；architecture 确认 workspace runtime 仍是 presentation owner；security 确认无输入、I/O、
设备、密钥或依赖变化；performance 确认只增加一次性 effect release 和 validity 判断。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI192_WORKSPACE_EFFECT_VECTOR_PASS natural=1 stop=1 rapid_switch=1 reduced_motion=1 detached=4
```

向量使用 Qt offscreen 内存对象且未显示主窗口；未启动可见 GUI/EXE，未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link、
硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.92` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致，provenance verify 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.92
size: 47,915,018 bytes
SHA-256: E983B2B6EC35B1877F4E46A5063181E4E0DF99262B344DF38FCE8E261035D7B7
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
