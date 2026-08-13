# SerialForge UI-1.56 交接

日期：2026-08-10

## 用户结果与范围

- 快速配置选择后，连接带内直接显示 `内置/自定义 · 配置名 · 传输类型`，并保留原有下一步提示；未选择时显示明确的空态说明。
- 摘要只展示已有 bounded `ConnectionPreset` DTO，不保存密钥、不自动连接；原 `connection_hint` QLabel 的文字、tooltip、AccessibleName/Description 和 controller wiring 保持不变。
- 已选 preset 使用共享 shell frame 绘制低对比度底部 signal rail；低动效、暂停、隐藏、最小化和关闭时静态回退。

## 实际修改文件

- `src/serialforge/presentation/connection_preset_context_surface.py`：新增连接快速配置摘要 surface 与主题化 rail。
- `src/serialforge/presentation/connection_preset_surface.py`：将 combo selection 投影到 context surface。
- `src/serialforge/presentation/controllers/connection_builder.py`：装配 surface，并保留原 `connection_hint` label 引用。
- `src/serialforge/presentation/controllers/connection_presets.py`：保存/删除 catalog 后同步摘要。
- `src/serialforge/presentation/controllers/lifecycle.py`：接入共享 MotionController frame/stop。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0043-connection-preset-context-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

## 架构审查与父代理整合

```text
架构角色  019feaf2-c103-7da1-a686-56ac90a3c965  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：ConnectionPresetContextSurface 只读既有 ConnectionPreset；connection controller 保持动作/状态 owner；共享 frame/stop 为唯一动效入口
```

父代理整合：surface 与 preset projection 均留在 presentation；不改变 preset codec、domain transport port、连接 gate 或自动连接语义。

## 验证与限制

```text
scripts/check.ps1       PASS 135 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI156_STATE_VECTOR       PASS none/builtin/custom/none; hint label identity/accessibility retained
UI156_MOTION             PASS shared frame; selected preset pulse; stop fallback
UI156_PIXEL              PASS three themes; 1180x780; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

## 打包收据

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.56`
- size：`47,838,432` bytes
- canonical/root SHA-256：`635228DA938D6F55B71369512F81D091B45090A7C3EA705009493F0A5CAF4F58`
- archive listing SHA-256：`3CEB06277CE97D62220E59860944F33BA0831E956F817778337492CF7CA83ED6`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
