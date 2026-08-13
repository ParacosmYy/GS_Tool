# UI-1.135 扩展工具站能力分组摘要

日期：2026-08-11  
范围：只读扩展工具站接入概览的 application DTO 与 presentation metrics。  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是唯一源码写入者。  
架构师线程：`019fedb2-4d88-79b3-ba2d-08cca3ccf335`，Luna/max/Fast，两个等待窗口未返回后关闭，未计为独立通过；
首屏裁剪修复架构师线程：`019fedb9-d420-76d0-a6a0-154c8dac31f2`，两个等待窗口未返回后关闭，未计为独立通过。
API 兼容性架构审查线程：`019fedc0-9eaa-7c33-873d-1007434a816d` 已完成；确认
`group_summaries` 追加到 dataclass 末尾，旧 positional 构造前缀保持兼容。

## 变更

- `application/extension_station.py` 新增 `ExtensionStationGroupSummary`，用 bounded immutable
  DTO 表达一组能力的 key、用户标签、能力数量和激活数量；总摘要校验分组 key 唯一且计数一致。
- `extension_station_summary()` 仍是 capability catalog 到 station summary 的唯一派生入口，产生
  OTA 传输、OTA 安全、调试输出三组计数；当前 catalog 仍为 7 个槽位、0 个激活后端。
- `presentation/embedded_station_overview.py` 只消费 `ExtensionStationSummary`，把总览指标与三组
  指标投影到六列 `QGridLayout`；presentation 不解释 contract/attach 状态，不创建动作、timer、
  socket、crypto、vendor SDK 或真实 OTA/J-Link 后端。

## 验收门

已验证三主题 × 980×680/1180×780 × 四 Tab 共 24 组横向 scroll maximum=0，overview accessible
description、分组总数不变量、旧 positional summary 构造兼容、共享 frame/stop/隐藏静态回退通过；截图
`build/ui_review_ui135_extension_groups.png` 的 `exact_white=0`。compileall、Ruff、source-limit、
`scripts/check.ps1` 和 onefile provenance 通过。父代理完成 owner、几何、可访问性、行为保持、
性能与简化审查。未修改嵌入式 C/C++；public vendor applicability=N/A；真实硬件与 GUI/EXE
启动验收未运行。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.135`；47,953,759 bytes；SHA-256
`5E5A71B3BAA59D581CD7093B53225395AB59C26011EA32696254D4DD994CF22C`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；signature `NotSigned`；
`release_eligible=false`；hardware acceptance `not_run`。根目录覆盖仍等待 PID 46108、49236 退出。
