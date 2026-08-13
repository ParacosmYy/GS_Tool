# ADR-0089：协议/遥测页 selector 文案与动态状态一致性

日期：2026-08-11  
状态：Accepted  
范围：协议解析、Component、Dataset、Curve、历史回放 presentation surface

## 背景

协议页的 enum 与 bounded selector 已经存在，但用户可见层仍混用 `Raw`、`Framing`、`Little/Big`、`series/sample` 等内部术语。
更隐蔽的问题是 Dataset 配置刷新会重新填充曲线 selector，导致初始中文文案在动态更新后退回英文；协议状态和流水线摘要也会显示 enum 内部值。

## 决策

- `presentation/controllers/protocol.py` 负责协议 preset、帧格式、校验、长度字段、字节序和回放速度的可见 label 与 affordance；所有 selector 显式不可编辑，`itemData(Qt.UserRole)` 保持原 enum/string/int/DTO。
- 内置 preset 的中文显示 label 由 presentation 映射提供，`ProtocolPreset` DTO、key、config、description 与 application/domain 逻辑保持不变。
- `protocol_config.py`、`derived_data.py`、`dataset_curve.py` 和 `lifecycle.py` 只把同一份既有状态投影为用户可读文本；动态启停、信号顺序、解析 gate、曲线 timer 和 snapshot 生命周期不改变。
- Dataset/Curve 动态刷新路径必须复用“数值序列/样本”文案，不允许初始化路径与更新路径各自维护相冲突的英文 fallback。
- 不新增协议能力、解析策略、业务状态源、timer、网络/设备 I/O 或外部依赖；真实 OTA/debug backend 仍保持 contract-only 边界。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI199_PROTOCOL_OPTIONS_VECTOR_PASS themes=3 preset_labels=8 framing_labels=6 checksum_labels=5 typed_data=1 dynamic_gates=1 dataset_refresh=1 replay_affordance=1
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

向量使用真实组合根的 Qt offscreen 内存对象，验证了三主题、selector label/data、长度前缀与 MAVLink 动态 gate、preset 状态、
Dataset 配置刷新和曲线空态；未显示主窗口、未连接设备、未运行真实协议/硬件/EXE 启动，也未创建、修改或运行 unit test、mock、
fixture、harness 或 test-only 资产。

Embedded workflow applicability：本轮仅修改 Python/Qt presentation/application-adjacent text projection，没有 MCU、BSP/HAL/CMSIS、
RTOS、ISR/DMA、驱动、固件 C/C++、bootloader、Flash/NVM 或真实 OTA/debug target；因此没有适用的 MCU/vendor public source constraint。
已完成 independent review、behavior-preserving simplification/reuse assessment 与静态/offscreen/provenance 验证；不构成 MISRA、ISO 26262、
ASIL、ASPICE 或任何认证声明。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
