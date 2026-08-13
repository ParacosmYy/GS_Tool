# UI-1.100 Pipeline 状态语义与共享动效交接

日期：2026-08-11  
范围：Pipeline summary presentation surface

## 结果

- `PipelineSurfaceLabel` 消费 lifecycle 已写入的 `state/source` 动态属性，按 `idle/active/transition/draft/blocked/history` 显示有界节点
  进度、来源色、未完成节点透明度和 blocked 静态叉标。
- 只有 active/transition/draft 使用既有共享 MotionController 的 phase 产生 pulse；history/blocked/idle、reduced-motion、隐藏/最小化和关闭
  保持静态，无新增 QTimer、线程、状态源、设备 I/O、外部依赖或资源。
- 保留 Pipeline 文本、accessible description、QSS properties、application/domain/OTA-debug contract；非法动态属性安全回退到 idle/live。

## 角色与独立复核

```text
产品角色       019fec89-84a6-7183-bfb2-5d110ae0ebbf  called; wait timed out; closed
架构角色       019fec89-84f1-7052-87ca-612d94f8df52  called; wait timed out; closed
UI 设计角色    019fec89-8540-7a71-9d94-bfe749371d5f  called; wait timed out; closed
开发角色       019fec89-858a-7df3-8bc4-88c9cae1298f  called; wait timed out; closed
验证角色       019fec89-85db-7532-b593-da91d1b86165  called; wait timed out; closed
打包角色       019fec89-8622-7fc3-8d94-73b4a56f07f2  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec8b-ccf5-7de1-ae98-ed04169b6195  called after implementation; wait timed out; closed
```

所有角色/独立复核均因运行时超时而未返回完整报告，超时不视为通过。父代理完成 correctness、readability、architecture、security、performance
五轴复核：确认 surface 只消费既有属性/共享帧，状态映射有界、QPainter 生命周期安全、token-only、没有新增热路径 timer/线程/I/O。
简化评估结论：复用 `refresh_dynamic_property`、`ThemeSpec`、共享 MotionController 与既有 lifecycle fan-out；没有需要删除的行为或新增
通用抽象。

嵌入式 R&D 适用性：N/A。未修改嵌入式 C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、bootloader、Flash、真实 OTA 或硬件 debug；未声明
厂商要求、MISRA、ISO 26262、认证或硬件合规。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src        PASS
.venv\Scripts\python.exe -m ruff check src           PASS
UI100_PIPELINE_STATE_VECTOR_PASS themes=3 states=6 sources=2 renders=36 moving=3 fallback=1 stop=1
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

向量使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未显示主窗口；仅有 PySide6 缺少 fonts 目录 warning。未运行
可见 GUI/HIDPI/读屏、真实设备/网络、EXE 启动、OTA、签名、硬件或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.100
size: 47,925,850 bytes
SHA-256: 7038F0C57040B6B518A051E1B779D1F58F01DF0B452A8991C54586F62D33BCA5
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

`SerialForge.exe` 是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
