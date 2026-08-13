# ARCH-104 / UI-1.177 交接：命令空态工作区画布

日期：2026-08-12

## 结果

命令管理页没有批量命令时，空态现在是页面唯一的可伸缩内容槽：

- `controllers/command_workspace_builder.py` 删除 root trailing stretch，并把
  `CommandBatchEmptyState` 作为唯一 vertical stretch owner；
- `command_batch_empty_state.py` 将卡片设为 expanding vertical policy，glyph 保持垂直居中，
  copy 区使用前后 stretch，使标题、说明和 CTA 在画布焦点位置呈现；
- `controllers/commands.py` 的批量状态、visible 投影、snapshot、执行和连接控制完全未改；
- 没有新增 timer、thread、paint loop、scroll owner、业务状态或 OTA/debug coupling。

## 证据

- `scripts/check.ps1`：pass；source limit 179 files <= 1000，theme token audit pass，ruff pass。
- 真实 Qt offscreen 三主题 `star_trail` / `moonlit_ocean` / `sakura_night`，窗口尺寸
  `980×720` / `1240×820`，四 workspace 切换：sibling geometry 无重叠。
- focus 模式 command page：`1240×820` 下 page `(0,0,1204,564)`，empty state
  `(14,151,1176,399)`，scroll hint 为 `内容已全部显示` / `complete`；最终三主题截图在
  `C:\Users\Gs\AppData\Local\Temp\serialforge-arch104-final-star_trail.png`、
  `...moonlit_ocean.png`、`...sakura_night.png`。
- overview 模式保留既有 scroll owner；980/1240 下无横向滚动异常，焦点按钮、scroll hint、
  tab/accessibility 文案保持非空。
- 真实 `app.exec()` 动效采样：`TARGET_HZ=120`、`8ms`、`frames=116`、均值 `8.536ms`、
  p95 `10.288ms`、有效约 `117.16Hz`。暂停、低动效、隐藏均为 timer inactive。
- 编译/静态检查没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。

## 架构与审查记录

- 本轮按要求调用架构师后才编辑代码；相关调用均在等待窗口内超时并关闭，未形成外部 PASS：
  `019ff3ba-701f-71e2-bf76-887c32ca7882`、`019ff3bd-4df9-7660-aaed-b9b0a6386a5b`、
  `019ff3c1-cc24-76d2-a666-58d9f6f8ecee`、`019ff3c7-f003-7793-98ec-c767f59b65f6`、
  `019ff3c9-84e8-7013-8097-11d162752826`、`019ff3cb-561f-7f52-9aba-640d03a64abb`。
- 父代理独立复核：空态布局 owner 保持在 presentation builder/component；命令状态继续由
  controller 投影；动画 1ms/elapsed 方案因实测无收益且可能丢预算而撤回；未发现安全输入、
  生命周期、耦合或文件行数问题。
- embedded C/C++ public-vendor-source applicability：N/A；没有固件、MCU、BSP/HAL/RTOS、OTA
  实现或 C/C++ 修改，不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。

## 交付与未运行项

本轮 onefile 已使用 `local-arch-104` 打包并覆盖 canonical、根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`；三者均为 `48,018,981` bytes，SHA-256 为
`58421049705382E4CEB343B6B2414D7B329A0957966AE02D62966B59C6423E29`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，正式发行资格为 `release_eligible=false`，硬件验收为 `not_run`。EXE 启动、
真实 Windows 可见窗口/高刷新显示器/HIDPI、高负载、硬件连接、OTA/RTT 实连、签名和正式硬件验收
仍未运行或未授权。
