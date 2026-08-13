# UI-1.118 scrollbar signal rail 可见性

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付范围

- `theme_stylesheet_controls.py` 的 vertical/horizontal scrollbar thumb 默认色从 neutral border 提升
  到既有 `BORDER_STRONG`，补充 surface border；hover/pressed 使用既有 accent。
- `theme_variant_controls.py` 对称覆盖为 `accent_purple`、`accent_pink`、`accent`，三主题保持一致
  的语义层级。
- 原生 QScrollArea/QPlainTextEdit 的 scroll range、arrowless policy、焦点/键盘和 terminal 语义不变；
  没有新增 widget、timer、业务状态、DTO、线程、I/O 或 callback。

## 证据

```text
UI118_SCROLL_SIGNAL_VECTOR_PASS sizes=2 themes=3 tabs=4 scrollbars=pass pressed=pass screenshots=24
source line limit: pass (157 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
compileall: pass
ruff: pass
provenance verify: pass
```

三主题的 980×680/1180×780 四页面截图已生成并人工查看；配置页 thumb 明显可发现，未见白色背景带。

## 交付物

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.118`  
size：`47,938,827` bytes  
SHA-256：`F04CC42821DFC6F3BEB3F7DEE143C83B51D22D19025EF22DCD5C736C55C18A3C`  
archive listing SHA-256：`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`  
签名：`NotSigned`；`release_eligible=false`；硬件验收：`not_run`

根目录覆盖仍被 PID `46108`、`49236` 的旧 SerialForge.exe 占用；未强制终止用户进程。

## 审查与适用性

架构师与独立质量审查线程在限定窗口内超时，未把超时记作通过。父代理完成 token 复用、selector
覆盖、native scroll 行为、白色回退、对比度、简化和行数审查。未修改嵌入式 C/C++；embedded
applicability=N/A，不声明 MISRA、ISO 26262、硬件或安全认证合规。
