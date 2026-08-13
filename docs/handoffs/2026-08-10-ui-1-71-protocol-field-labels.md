# UI-1.71 协议/遥测字段标签主题层级

日期：2026-08-10  
范围：协议/遥测配置页普通字段标签的主题语义层级。  
父代理：Codex；父代理是本轮唯一写入者。  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建或操作 Git/Codex worktree。

## 用户结果

- 协议/遥测配置页的 8 个普通字段标签统一进入 `QLabel[role="muted"]` 次级视觉层级：预设、Framing、校验、最大帧(B)、
  Delimiter Hex、长度字节、字节序和过滤。
- 5 个 section 标题继续使用 `role="section"`，5 个状态 owner、hint 和动态说明保持原有语义，主题 stylesheet 不再让普通字段
  依赖系统白色 label fallback。
- 原生 combo/line edit/spin box、协议回调、配置值、parser/apply/reset、状态投影、焦点、Tab 顺序和布局列均保持不变。

## 架构边界

`src/serialforge/presentation/controllers/protocol.py` 新增局部 `_field_label(text)`。helper 只创建 `QLabel` 并设置
`role="muted"`，由既有默认 stylesheet 和三套 theme override 提供实际颜色。它不读取 ViewModel/DTO，不接收 callback，不写入
协议配置，不创建 timer，不维护主题状态，也不扩展到全局 label 工厂。section/status/hint/dynamic label 仍由各自 owner 保留。

## 实际修改

- `src/serialforge/presentation/controllers/protocol.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0058-protocol-field-label-hierarchy.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`
- `docs/handoffs/current.md`

## 角色调用与独立质量复核

```text
产品角色       019feb88-e8ff-7ea0-976f-2d522663555e  called before source edit; wait timed out; closed
架构角色       019feb88-e94e-79a3-9bc9-be2bd9cb306f  called before source edit; wait timed out; closed
UI 设计角色    019feb88-e9a7-75b3-b3f3-b00d2b87b52b  called before source edit; wait timed out; closed
开发角色       019feb88-e9f1-7231-8d96-379eab93f407  called before source edit; wait timed out; closed
验证角色       019feb88-ea3c-7c62-8957-ec6fbfd2d25b  called before source edit; wait timed out; closed
打包/流程角色  019feb88-ea85-7213-beb2-877a7466556d  called before source edit; wait timed out; closed
独立质量复核   019feb8b-9a95-73f2-9966-87d7c0fc2a7f  called after implementation; wait timed out; closed
```

六角色与独立复核在运行时窗口内超时，未把超时当成通过。父代理完成五轴审查：

- correctness：只变更 8 个普通 `QLabel` 的 presentation property，5 个 section/status owner 未降级。
- readability/simplicity：局部 helper 只有单一职责，避免 8 处重复 `setProperty`；文件仍低于 1000 行。
- architecture：helper 留在 protocol builder，不扩散到 shared module；不增加跨层依赖、DTO 或状态源。
- security：没有输入/存储/网络/密钥路径变化，没有新依赖或外部资源。
- performance：只在 panel 构建时创建既有 Qt label，不增加重绘循环、timer、动画或热路径分支。

简化评估结论：现有局部 helper 是最小复用边界，复用已有 `QLabel[role="muted"]` selector；新增全局工厂、主题服务或动态状态层
只会扩大耦合，因此不采用。

## 验证

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1
PASS 148 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; legacy_qss_literals=339; Ruff/compileall

.venv\Scripts\python.exe -m compileall -q src
PASS

短时 Qt offscreen protocol panel vector（仅内存对象，不是测试资产）
UI171_PROTOCOL_LABELS_PASS theme=star_trail fields=8 sections=5 statuses=5
UI171_PROTOCOL_LABELS_PASS theme=moonlit_ocean fields=8 sections=5 statuses=5
UI171_PROTOCOL_LABELS_PASS theme=sakura_night fields=8 sections=5 statuses=5
```

第一次 vector 验证脚本把状态 owner 数误设为 4；失败证据显示实际 owner 是 `protocol/component/dataset/curve/replay` 五项。修正断言后
三主题重新通过，生产代码没有因该验证修正而变化。

未运行完整 GUI、HIDPI/读屏/视觉帧差分、EXE 启动、真实 UART/TCP/BLE/RTT/J-Link/OTA、硬件、签名和正式发行验收；没有创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包状态

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.71` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- size：`47,888,719` bytes
- SHA-256：`6331DE6F5145CB9670A3CEB4D1379E280EB6099D1FCB155A8C96122B2371A1E4`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
