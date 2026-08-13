# UI-1.84 设置页 scroll surface 交接

日期：2026-08-10  
状态：源码、vector 验证与 onefile 交付完成  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付结果

连接、协议/遥测、命令管理三个共享设置页现在由 `scroll_page()` 统一声明横向 `ScrollBarAlwaysOff`，content 使用横向 `QSizePolicy.Expanding`；纵向保持 `ScrollBarAsNeeded`，
避免底部出现像白线的横向滚动/亮色槽。页面内容、`widgetResizable`、objectName、焦点顺序、AccessibleDescription、业务 controller 和主题切换保持不变。

## 修改范围

- `src/serialforge/presentation/controllers/composition.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0071-scroll-surface.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`

## 角色调用与独立复核

```text
产品角色       019febf6-81cf-7e03-9188-341bc8eb6787  called before source edit; wait timed out; closed
架构角色       019febf6-820e-70e1-afa7-4d08e10c3b3f  called before source edit; wait timed out; closed
UI 设计角色    019febf6-825e-7fc0-b60d-1d2e811fe1d1  called before source edit; wait timed out; closed
开发角色       019febf6-82aa-7ff2-8f36-74a75fe270d9  called before source edit; wait timed out; closed
验证角色       019febf6-82fe-7ac1-843c-021533e99357  called before source edit; wait timed out; closed
打包/流程角色  019febf6-8348-71f1-b164-8a9ee0aec156  called before source edit; wait timed out; closed
独立质量复核   019febf8-8382-7730-866a-68b78a3e614c  called after implementation; wait timed out; closed
```

角色没有返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认三页共享入口、横向关闭和纵向保留；readability/simplicity 确认两项设置足够；architecture 确认 composition owner 与 feature controller 边界不变；security 确认无外部输入/网络/存储/密钥/依赖变化；performance 确认没有新增 timer/effect/绘制负担。

简化评估结论：不添加页面级滚动代理、自定义滚动条或额外主题规则。

## 验证证据

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI184_SCROLL_VECTOR_PASS width=980 pages=3 horizontal=hidden vertical=as-needed
UI184_SCROLL_VECTOR_PASS width=1180 pages=3 horizontal=hidden vertical=as-needed
UI184_SCROLL_VECTOR_PASS width=1440 pages=3 horizontal=hidden vertical=as-needed
UI184_THEME_SCROLL_PASS theme=star_trail
UI184_THEME_SCROLL_PASS theme=moonlit_ocean
UI184_THEME_SCROLL_PASS theme=sakura_night
UI184_SCROLL_SURFACE_VECTOR_PASS responsive=980,1180,1440 themes=3
```

向量使用真实组合根与 Qt offscreen，主窗口未 `.show()`；可见 GUI、HIDPI、读屏、硬件、网络、OTA、签名和正式发行验收未运行。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

- 本轮命令：`scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.84`
- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.84`
- size：`47,892,763` bytes
- SHA-256：`D10B2F4972EF9D4115B45B87DB770DC66D5E9E7AE91EFE40B40DE070BB838E17`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

嵌入式 C/C++ 适用性：N/A。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
