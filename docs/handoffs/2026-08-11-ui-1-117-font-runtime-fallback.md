# UI-1.117 字体运行时 fallback 与中文可读性

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付范围

- 新增 `src/serialforge/presentation/font_runtime.py`，集中完成可用字体发现、标准本机字体 fallback
  和 QApplication 字体选择。
- `qt.py` 增加 `QFont`/`QFontDatabase` presentation 导出；`main.py` 在 QApplication 创建后调用，
  MainWindow、controller、application/domain、transport 和 OTA/debug 边界不变。
- stable stylesheet 的字体族顺序调整为 `Microsoft YaHei UI`、`Microsoft YaHei`、`Segoe UI`、
  `Noto Sans SC`、generic sans-serif。
- 不复制、下载或打包字体资产；本机字体注册只存在于当前进程，失败时保留 Qt 默认并继续启动。

## 证据

```text
UI117_FONT_RUNTIME_PASS family= Microsoft YaHei UI families=2
UI117_AUDIT_RENDER_PASS family= Microsoft YaHei UI screenshots=12 themes=3 tabs=4
UI117_RESPONSIVE_VECTOR_PASS family= Microsoft YaHei UI themes=3 tabs=4 horizontal_overflow=none
source line limit: pass (157 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
compileall: pass
ruff: pass
provenance verify: pass
```

三主题与四个工作区的 1180×780 截图、980×680 响应式截图均已生成并人工查看；中文可读，未见白色
背景带或横向溢出。offscreen 仍报告 PySide6 自带 fonts 目录不存在，但 runtime 已注册本机
`Microsoft YaHei UI`，该提示不再导致中文方框。

## 交付物

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.117`  
size：`47,940,611` bytes  
SHA-256：`A8382CDA77F2601BDD4465468881C0F056EB86FD307090BDC89B02C4C7162333`  
archive listing SHA-256：`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`  
签名：`NotSigned`；`release_eligible=false`；硬件验收：`not_run`

根目录覆盖仍被 PID `46108`、`49236` 的旧 SerialForge.exe 占用；未强制终止用户进程。

## 审查与适用性

架构师与独立质量审查线程在限定窗口内超时，未把超时记作通过。父代理完成 presentation owner
边界、系统字体优先、注册失败回退、重复注册、无资源复制、行数和行为保持审查。未修改嵌入式
C/C++；embedded applicability=N/A，不声明 MISRA、ISO 26262、硬件或安全认证合规。
