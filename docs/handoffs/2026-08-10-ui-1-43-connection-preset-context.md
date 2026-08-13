# Handoff：UI-1.43 连接快速配置自定义选中态与语义上下文

日期：2026-08-10  
范围：连接快速配置 ComboBox 的三主题自定义选中态、动态 QSS 回退和空态/内置/自定义辅助文本同步。  
父代理：Codex；父代理是本轮唯一源码/文档写入者。  
工作区：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建或操作 worktree。

## 用户结果

- 选择自定义连接快速配置后，关闭下拉框仍有“自定义 · 名称”文字和独立主题化底色，不再依赖系统 palette 或单一颜色。
- 未选择、内置、自定义三态的 tooltip 和 `AccessibleDescription` 与当前选择一致；选择仍只填充连接表单，不自动连接。
- 活动/禁用状态下自定义底色不会覆盖 disabled 回退；原生 ComboBox popup、键盘、焦点和删除按钮 gate 保持不变。

## 实际改动

- `src/serialforge/presentation/property_refresh.py`、`src/serialforge/presentation/contracts.py`
  - 将动态属性 value 与 `DynamicPropertyRefresher` typed callback 收窄并对齐为 `str | bool`，保留“未变化跳过、变化后
    setProperty → unpolish/polish → update”的既有原语。
- `src/serialforge/presentation/connection_preset_surface.py`
  - 新增 `is_custom_connection_preset()`，集中判断 builtin/custom。
  - 新增 `update_connection_preset_context()`，集中维护三态 tooltip 与 `AccessibleDescription`。
  - refresh 路径同时更新 `customSelected` 和上下文；保留“自定义 ·”可见前缀。
- `src/serialforge/presentation/controllers/connection_builder.py`
  - 选择路径复用 surface predicate/context 和动态属性刷新，不复制 builtin key 规则。
- `src/serialforge/presentation/controllers/connection_presets.py`
  - 删除 `apply_connection_preset()` 对 ComboBox tooltip 的覆盖，避免把选择器上下文降级为纯 preset 描述；connection hint 仍按原逻辑更新。
- `src/serialforge/presentation/theme_stylesheet_base.py`、`theme_variant_shell.py`
  - 增加 `QComboBox#connectionPresetCombo[customSelected="true"]` 的 normal/hover/focus/disabled 主题规则。
  - disabled 显式回退到 disabled 颜色，不影响通用 ComboBox popup 规则。
- `scripts/check_theme_tokens.py`
  - 将自定义选中态 selector 纳入必需 selector，三主题缺失时静态门禁失败。
- 文档：`docs/adr/0030-connection-preset-custom-state-context.md`、`docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、
  `tasks/plan.md`、`tasks/todo.md`、`docs/handoffs/current.md`。

## 架构与六角色评审

- 产品：`019fea60-cdc5-7c11-9c4c-fb82f6d3eb13`，结论为 P1/有条件通过；要求自定义语义不能只靠颜色、辅助描述不能固定写“内置”。
- 架构：`019fea60-ce0c-7c42-bd98-519bb7bb8ec1`，条件 GO；确认 surface → property refresh → Qt、theme → ThemeSpec 的依赖方向，无跨层改动。
- UI 设计：`019fea60-8bfd-7641-af97-fde9c36abc99`，建议保留原生 ComboBox，补齐动态辅助描述；不新增 ComboBox 动画。
- 开发：`019fea60-8c4a-7640-8e5c-4a0ccd714969`，建议复用 `refresh_dynamic_property`，不改 ViewModel/domain/transport。
- 验证：角色代理 `019fea6b-434f-7661-bc7c-97fd518dd1d1` 两轮等待超时后关闭；父级已完成下列静态/offscreen 证据。
- 打包/流程：角色代理 `019fea6b-4392-7120-8ea4-b056e3ce34e3` 两轮等待超时后关闭；父级已完成 onefile/root EXE 重建与 provenance 核验。
- 追加架构调用：`019fea66-c9eb-7fa3-bc24-0146edf60ee7` 两轮等待超时后关闭；其要求已由父级按同一边界审计执行。
- 独立质量复核：`019fea65-5184-7913-9b40-6cd5dcffc761` 两轮等待超时后关闭；父级完成 correctness/readability/architecture/security/performance 五轴复核。
- 初始六角色并发曾遇到子代理线程上限；后续角色陆续以只读结果返回或按上方 pending 状态记录，未有子代理写入源码。

## 验证证据

```text
scripts/check.ps1
  PASS source line limit: 126 files <= 1000
  PASS theme token audit: 3 themes, 22 semantic tokens, 12 selectors, legacy_qss_literals=320
  PASS Ruff / all checks passed

UI143_CONTEXT
  PASS star_trail/moonlit_ocean/sakura_night
  PASS empty/builtin/custom AccessibleDescription + tooltip
  PASS controller dispatch keeps customSelected=True and applies custom key
  PASS custom/disabled visual delta > 0; near_white_custom=0; near_white_disabled=0

UI143_IMPORT modules=125 qapplication_instance=False
UI143_COMPILE pass
```

离屏截图：

- `build/ui_review_ui143_preset_star_trail.png`
- `build/ui_review_ui143_preset_moonlit_ocean.png`
- `build/ui_review_ui143_preset_sakura_night.png`

截图运行环境报告 `QFontDatabase: Cannot find font directory .../PySide6/lib/fonts`；中文方框是该环境字体缺失告警，不能外推 Windows 字体结论。

## 未运行项目与授权边界

- 未启动持续 GUI、EXE、后台服务或网络监听；未做 Windows 原生 style、真实 Tab/键盘、读屏、HIDPI 或正式对比度验收。
- 未连接或操作 UART、TCP/UDP、BLE、RTT/J-Link、OTA 设备；未刷写、擦除、部署或操作硬件。
- 未创建、修改或运行 unit test、mock、fixture、harness 或其他 test-only 资产；只使用了短时进程内 Qt offscreen 数据向量和 build 截图审计。
- 本项目是 Python/PySide6 desktop；嵌入式 C/C++ assurance applicability=N/A，不引用厂商资料，不宣称 MISRA、ISO 26262、WCAG、认证或硬件合规。

## UI-1.43 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.43`
- size：`47,805,312` bytes
- canonical/root SHA-256：`101DA448C2B95061718230B70715370EC0D29815271F8FBF7E741015AD82AD83`
- archive listing SHA-256：`1DE0907DDB8F3A9FA3C0040B991ED18A316A2CD69BDE808402699F8B601DF6B5`
- provenance：`provenance.py verify` pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 下一步

1. 用户授权后补做 Windows 原生 style、真实 Tab/键盘、读屏/HIDPI 与持续 GUI/EXE 启动验收。
2. 保持 `signature=NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`，除非用户另行授权签名/硬件验收。
3. 后续如需“表单被手动修改”提示，先定义 dirty 语义，再由架构师评审独立切片；不要把 `customSelected` 直接升级为业务事实源。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
