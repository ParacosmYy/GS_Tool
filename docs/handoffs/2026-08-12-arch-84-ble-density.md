# ARCH-84 / UI-1.157 BLE GATT 配置密度交接

日期：2026-08-12  
范围：BLE GATT presentation-only 布局重排与 builder owner 拆分  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；父代理是本轮唯一写入者；未创建或操作 Git/Codex worktree

## 实现

- 新增 `src/serialforge/presentation/controllers/ble_builder.py`，独立拥有 BLE surface 的控件创建和
  四段任务布局：扫描与筛选、设备发现与连接、GATT 特征与收发、边界提示。
- `connection_builder.py` 删除 BLE 7 列网格，只把 `build_ble_panel(window)` 返回的原有
  `BleControlBindings.title/panel` 加入连接页 root。
- `form_fields.build_labeled_field()` / `build_field_row()` 提供一致的垂直 label、行间距和伸缩；既有
  外层 `settingsScroll` 继续负责纵向内容，不创建 nested scroll 或第二个布局 owner。
- 没有修改 BLE runtime、domain/application、worker、TransportKind、写入模式、callback、默认值或
  shared MotionController；没有触发真实扫描/连接/配对/通知/写入。

## 复核与简化

本轮先后请求 Luna/max 与 Terra/max 架构师做只读裁决；两个服务窗口均超时，随后关闭未完成子代理，
未伪造架构师 PASS。父代理依据现有 `BleControlBindings`、`connection_builder`/`ble.py` owner 关系
完成架构 fresh-pass：BLE 组合集中到一个文件，连接壳只做接入；没有增加通用 form factory、状态模型、
timer 或跨层 facade。简化审查结论：新 builder 的唯一局部 `_bounded_combo` 只负责 BLE 长选项的尺寸
边界，不把 BLE policy 复制到 shared helper；当前结构无需进一步抽象。

## 非破坏性验证

```text
uv run ruff check src/serialforge/presentation/controllers/ble_builder.py src/serialforge/presentation/controllers/connection_builder.py  pass
uv run python -m compileall -q src/serialforge/presentation/controllers/ble_builder.py src/serialforge/presentation/controllers/connection_builder.py  pass
scripts/check.ps1                             pass
source-limit                                   170 files <= 1000 lines
theme-audit                                    3 themes / 22 semantic tokens / 19 selectors
BLE157_CONTRACT                                BleControlBindings, 13 fields, write itemData preserved
BLE157_LAYOUT                                  980x720 hmax=0 overlap=false exact_white=0
BLE157_LAYOUT                                  1240x820 hmax=0 overlap=false exact_white=0
BLE157_THEME                                   3 themes, hmax=0
BLE157_CLOSE                                   window hidden, timer inactive
visual                                         980x720 + 1240x820 screenshots reviewed
```

实际字体为 `Microsoft YaHei UI`；offscreen Qt 的既有 font-directory warning 不代表发行包缺少 Windows
系统字体。真实可见 GUI、HIDPI、读屏、签名、硬件连接、BLE 扫描、OTA/debug 执行、刷写、部署和 HIL 未运行。

## 交付状态

源码、文档与 onefile 交付已完成。canonical、根目录 `SerialForge.exe`、根目录
`SerialForge-latest.exe` 均为 `47,998,877` bytes，SHA-256 为
`07E5CF0BD1F9D0ADFD9C9C9CC23A61FFD70F8673DBA3257BE9F820A379E7D623`；archive listing SHA-256 为
`9991DD041A86C1828261B9D8731E17A04BA20F90739193F11F6DE12423455D78`。provenance source revision 为
`local-arch-84`，manifest verifier 通过，签名 `NotSigned`，`release_eligible=false`，
`hardware_acceptance=not_run`；根目录 EXE 启动/关闭烟测通过，精确同路径残留为 0。

嵌入式 C/C++ 适用性：N/A。本轮没有 firmware/MCU/BSP/HAL/RTOS/bootloader/Flash 修改，不声明厂商要求
或认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
