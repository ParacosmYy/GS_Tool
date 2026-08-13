# UI-1.58 发送表单上下文摘要交接

日期：2026-08-10

## 结果

已完成发送控制带的 UI-1.58 垂直切片，并覆盖根目录 onefile EXE。发送区现在会即时告诉用户当前编码、payload/wire 字节数和 CRLF 影响；坏 Hex 与超过 canonical payload 上限的输入会提前显式反馈。

## 实际修改

- `src/serialforge/presentation/send_context_surface.py`：`SendContextProjection`、纯 `project_send_form()` 和 `SendContextSurface`；复用 `MAX_COMMAND_PAYLOAD_BYTES`，不持有 ViewModel/transport，不创建 timer。
- `src/serialforge/presentation/controllers/send_context.py`：唯一 form → surface projection owner。
- `src/serialforge/presentation/controllers/terminal.py`：装配 surface；mode/CRLF 变化复用既有 `on_send_input_changed` 回调。
- `src/serialforge/presentation/controllers/connection.py`：在既有 send affordance projection 中刷新 context。
- `src/serialforge/presentation/controllers/lifecycle.py`：共享 MotionController frame/stop fan-out。
- `src/serialforge/presentation/theme_stylesheet_base.py`、`theme_variant_shell.py`：sendContext empty/ready/invalid 三态主题覆盖。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0045-send-context-surface.md`、`tasks/plan.md`、`tasks/todo.md`、`docs/handoffs/current.md`：记录边界与交付。

## 架构与质量记录

```text
架构角色  019feb04-b512-7983-9478-0f676f388d42  called before source edit; wait timed out; closed
架构角色  019feb0c-a4bf-7820-a061-3e38f3e7a4bb  called before canonical-limit correction; wait timed out; closed
独立质量复核  019feb0b-6f4c-7491-b49c-8e9ac7afa43c  called after implementation; wait timed out; closed
父代理    bounded audit GO；先发现并修正 1 MiB 显示上限与 domain 64 KiB canonical 上限不一致的问题
```

复核结论：presentation surface 只负责展示，controller 只投影 native form，`connection.py` 继续拥有发送 gate，`lifecycle.py` 继续拥有共享 frame/stop；无循环依赖、无新增控件级 timer、无 payload 持久化或传输耦合。行为保持简化评估：复用 domain canonical 上限并提前投影 invalid，删除了一个与真实发送约束不一致的独立显示上限；未改变 `send_current()`、ViewModel 或传输语义。

## 验证

```text
scripts/check.ps1       PASS 138 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI158_SEND_CONTEXT       PASS empty / valid HEX+CRLF / invalid HEX / canonical over-limit / shared frame / reduced-motion
UI158_THEME              PASS star_trail, moonlit_ocean, sakura_night; 1180x780; near_white=0
```

截图审计文件：`build/ui_review_ui158_send_context_star_trail.png`、`build/ui_review_ui158_send_context_moonlit_ocean.png`、`build/ui_review_ui158_send_context_sakura_night.png`。offscreen 只出现 PySide6 fonts directory 缺失提示；这不代表 Windows 字体结论。

没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产；没有启动持续 GUI 或真实 EXE、连接真实 UART/BLE/RTT/J-Link、执行 OTA/硬件操作、签名或正式发行验收。嵌入式 C/C++ 适用性：N/A。

## 交付包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.58`
- size：`47,851,307` bytes
- canonical/root SHA-256：`1C911A77887234AFF46470F850E1438C95BAA25D17243170380CFC1EE4489D8A`
- archive listing SHA-256：`778FB6B7F5F2651F8297C549F1578C44A59D15221D6EA09C453F3D9DCA07B359`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
