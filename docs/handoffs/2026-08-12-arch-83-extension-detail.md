# ARCH-83 / UI-1.156 扩展能力卡可用性与详情投影交接

日期：2026-08-12  
范围：扩展工具站 presentation-only 可用性与密度修复  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；父代理是本轮唯一写入者；未创建或操作 Git/Codex worktree

## 结果

- `embedded_extension_panel.py` 继续是唯一扩展页组装 owner。
- 7 张 OTA 传输、OTA 安全、RTT/J-Link 卡片现在是完整 `QPushButton` 语义的单选展示卡；整卡点击、
  focus、Enter、Space 走同一选择路径，始终只选中一张。
- 新增 `presentation/extension_capability_detail.py` 作为 stateless DTO→view 叶子，展示 title、
  group/reference、raw `key/group/reference/state`、summary、boundary 和固定“不执行” guard。
- `ExtensionPanelWidgets` 只暴露 `layout` 与 `station_overview`，详情不形成跨 controller 公共依赖。
- 详情复用既有外层 `QScrollArea`，没有 nested scroll、splitter、timer、线程、backend、设备 I/O 或
  OTA/AES/RTT/J-Link 执行路径。
- extension QSS 已拆到 `theme_stylesheet_extension.py`，composer 顺序为
  `BASE_STYLESHEET + EXTENSION_STYLESHEET + CONTROLS_STYLESHEET`；base 不再承载 extension 专属样式。

## 架构与复核

架构师（Luna/max）批准 panel owner、详情叶子、外层滚动和全卡片键盘语义；窄审查批准从
`ExtensionPanelWidgets` 移除 capability detail 公共字段。父代理完成五轴 code review：correctness、
readability/simplicity、architecture、security、performance 均无 Required findings。简化判断为保留
当前最小边界：不增加通用 selection model、不复制 DTO policy、不创建新的 animation owner；详情叶子
只保留稳定字段和固定 guard。旧 UI-1.120 的静态 NoFocus 规则已由 ARCH-83 明确 supersede，但其只读、
无 backend 边界保留。

## 非破坏性验证

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
source-limit                                   169 files <= 1000 lines
theme-audit                                    3 themes / 22 semantic tokens / 19 selectors
ARCH83_FONT                                    Microsoft YaHei UI
ARCH83_STRUCTURE_PASS                          cards=7 detail=1 horizontal_max=0
ARCH83_CLICK_PASS                              7/7 cards, one selected, detail synchronized
ARCH83_FOCUS_PASS                              7/7 cards, one selected, detail synchronized
ARCH83_KEY_PASS                                native QPushButton Enter + Space, one selected
ARCH83_DTO_UNCHANGED_PASS                      true
ARCH83_LAYOUT_PASS                              980x720 + 1240x820 × 3 themes, overlap=0, horizontal_max=0
ARCH83_THEME_PASS                              3 themes, horizontal_max=0, exact_white=0
ARCH83_REDUCED_PASS                            timer inactive, selection preserved
ARCH83_CLOSE_PASS                              window hidden, timer inactive
visual                                         980x720 screenshot reviewed
```

offscreen Qt 的 font-directory warning 属于当前环境字体探测提示；实际应用字体选择为
`Microsoft YaHei UI`，截图中文字正常。真实可见 GUI、HIDPI、读屏、签名、硬件连接、OTA/debug 执行、
刷写、部署和 HIL 未运行；没有操作目标硬件。

## 交付状态

源码、文档与 onefile 交付已完成。canonical、根目录 `SerialForge.exe`、根目录
`SerialForge-latest.exe` 均为 `47,994,096` bytes，SHA-256 为
`64A43DB59AA18A8B7F683DDFD99D2EE7531B223D4E60DEFFC5727D544547E1DD`；archive listing SHA-256 为
`C5FCC1126F66741D61881491EA6A44056C8842DD5BD997DE81CC8B484615B3C8`。provenance source revision 为
`local-arch-83`，manifest verifier 通过，签名 `NotSigned`，`release_eligible=false`，
`hardware_acceptance=not_run`；根目录 EXE 启动/关闭烟测通过，精确同路径残留为 0。

嵌入式 C/C++ 适用性：N/A。本轮没有 firmware/MCU/BSP/HAL/RTOS/bootloader/Flash 修改，不声明厂商要求
或认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
