# UI-1.113 协议与派生数据动作 affordance

日期：2026-08-11  
范围：协议/组件/Dataset 页面现有四个动作按钮的可用态与禁用态提示。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：D:\Workplace\Agent_Workplace\SerialForge；未检测到 .git，未创建/操作 worktree

## 结果与边界

- 加载组件 Profile/Codec、导出组件 CSV、加载 Dataset 配置、导出 Dataset CSV 均补齐 action-specific tooltip 与 accessible description。
- 派生来源不可用时，四个动作继续复用既有 enable gate，同时在原动作语义后追加明确的“当前不可用”原因；不再用同一条通用提示覆盖四种动作。
- 四条基础文案收敛到 presentation/protocol_action_hints.py 的不可变 contract；protocol.py 与 protocol_config.py 只消费 contract 字段，避免 owner 之间的文案漂移。
- 只修改 presentation/controllers/protocol.py 的静态控件语义和 protocol_config.py 的既有 enabled projection；不新增按钮、业务状态、signal、timer、线程、依赖、I/O、DTO 或跨层 affordance registry。
- 原始终端、原始记录、发送、连接、OTA contract-only、RTT/J-Link attach-only 边界保持不变。

## 架构与审查证据

- owner 边界：protocol.py 负责创建四个按钮及静态语义；protocol_config.py 负责既有派生来源 enabled/disabled projection；MainWindow、application、domain 和 transport 不承载 Qt 文案。
- 父代理完成五轴审查：行为保持、owner-local 高内聚低耦合、无新增状态源、源码行数、主题/lifecycle 复用均通过。
- 运行时尝试调用 Luna/max/Fast 架构师、六角色前置评审和独立复核，但当前协作运行时持续返回 agent thread limit reached；因此独立复核不计为通过，已记录为未完成证据，不伪装成 pass。

## Embedded assurance

本轮只修改 Python/PySide6 presentation，不包含 MCU、固件、嵌入式 C/C++、BSP/HAL、RTOS、ISR/DMA、驱动或硬件协议实现。公开厂商资料不适用，不声明 MISRA、ISO 26262、硬件或安全认证合规。embedded assurance 子代理记录了 applicability=N/A、independent scan、simplification=N/A 和静态验证证据。

## 验证

scripts/check.ps1：PASS（154 files <=1000；3 themes；22 semantic tokens；19 selectors；legacy_qss_literals=0）
compileall：PASS
ruff：PASS
UI113_DERIVED_ACTION_AFFORDANCE_VECTOR_PASS：3 themes；enabled/disabled 两态；4 actions；tooltip/accessibleDescription 同步
provenance verify：PASS

向量使用短时 Qt offscreen 组件 panel，没有调用 show、没有启动持续 GUI、EXE、真实设备、网络、OTA、J-Link 或硬件。未创建或运行测试专用资产。

## 打包

artifact：dist/release/0.1.0/core/onefile/app/SerialForge.exe
root：SerialForge.exe（pending-user-close；PID 46108、49236 持有旧文件）
source revision：local-ui-1.113
size：47,934,870 bytes
SHA-256：3CBA66937B7006831920BBD395604EA7B838E70E5600B64A22B2D01E432278AA
archive listing SHA-256：C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
Python / PyInstaller：3.12.13 / 6.22.0
signature / release_eligible：NotSigned / false
hardware_acceptance：not_run

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
