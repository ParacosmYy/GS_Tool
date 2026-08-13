# M5f 六角色复核记录：有界通用协议预设

日期：2026-08-09  
范围：通用 framing/checksum 预设目录、显式应用边界、UART/TCP Client 作用域、Qt 交互和双模式打包。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 | 关键建议 |
|---|---|---|---|
| 产品 | `019fe49f-a433-7412-9056-63b2d6ee26a5` | pass with conditions → 已整合 | 只提供通用 preset；不能把 Modbus RTU/MAVLink/NMEA 完整语义包装成已完成 |
| 架构 | `019fe49f-a47f-7d71-9e82-8b8bc15e4b36` | pass with conditions → 已整合 | domain-only immutable DTO；未来外部目录需独立注入/校验，不能让 UI 成为协议注册中心 |
| UI 设计 | `019fe49f-a4ba-7970-be21-9f652ae4b8a9` | pass with conditions → 已整合 | 选择只载入编辑区；非 UART/TCP Client 显示 raw-only 并禁用协议配置控件 |
| 开发 | `019fe49f-a4f9-7de0-bbdf-afc9735e5136` | pass with conditions → 已整合 | 复用现有 ProtocolConfig/configure/reset；修正 PySide6 StrEnum 取回为字符串的边界 |
| 验证 | `019fe49f-a533-7ec3-8424-fbad07ab735e` | pass conditionally → 已整合 | 域向量、offscreen、作用域、关闭、静态和双模式包；真实设备与标准 codec 后置 |
| 打包/流程 | `019fe49f-a571-7db2-a811-8cb79b5199f7` | revise → 已整合 | M5f 后必须重建 onedir/onefile；默认包继续不含 BLE/J-Link vendor runtime |

六个子代理已完成并关闭；父代理负责整合、最终源码检查和验证。

## 变更摘要

- `domain/protocol_presets.py` 新增不可变 `ProtocolPreset`、有界 catalog 校验和 config 匹配函数；
- 内置 5 项 generic preset：Raw、Line LF/CRLF、Delimiter AA 55、Length U8 LE、Length U16 LE + CRC16/Modbus；
- 目录最多 16 项，key/label/description 有界，key 和 config 不重复；不读磁盘、不动态导入、不执行用户代码；
- 协议面板新增预设下拉框和 tooltip；选择只填充编辑区，显式点击“应用”才调用既有 `configure_protocol()`；
- UART/TCP Client 才启用协议/组件配置；UDP、TCP Server、BLE、RTT 保留 raw-only 文案并禁用相关控件；
- 修正 PySide6 对 `StrEnum` item data 返回 `str` 时的显式 `FramingKind`/`ChecksumKind` 转换；
- 没有新增 application port、transport 分支、运行时依赖或标准协议库；文档和 ADR 记录后续标准协议边界。

## 重要语义与未决风险

预设选择不是 parser 配置；只有“应用”才会清空 parser partial state、component rows 和 Dataset 派生窗口。
Raw recorder、终端、发送和会话生命周期不因预设失败改变。当前 catalog 只覆盖 generic framing/checksum，
不宣称 Modbus RTU 地址/功能码/RTU 间隔、MAVLink 版本/签名/CRC-extra 或 NMEA `*HH` 校验。

架构复核指出：`ProtocolFramesDecodedEvent` 当前没有 parser generation，worker 在 parser lock 释放后发布
事件；配置切换与已完成解析帧的发布存在迟到事件风险。该问题属于后续独立 generation/source gating 切片，
本轮没有扩大事件 schema 或伪造已解决状态。

## 验证证据

已运行：

- `uv lock --check`：通过；
- `uv run --locked ruff format --check --no-cache src`：通过，48 files；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- `scripts/check.ps1`：通过；
- inline domain vectors：5 项 catalog、数量/重复 key/config、config 匹配、自定义回退、Raw/Line decoder：通过；
- Qt `QT_QPA_PLATFORM=offscreen`：预设选择不生效、显式应用生效、UDP raw-only 禁用、切回 UART、关闭和 worker 清理：通过；
  已知 `QFontDatabase` 缺少 PySide6 字体目录提示，不影响退出；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit，PID `22932`；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap/GUI child startup/WM_CLOSE/exit，bootstrap `18396`、GUI `87232`；
- 默认 onedir 文件名扫描中的 `Bleak|WinRT|Bluetooth|SEGGER|JLink|probe-rs` 匹配数：`0`；
- PyInstaller `Analysis-00.toc`/`PYZ-00.toc` 已包含 `serialforge.domain.protocol_presets`；
- 验证结束时没有残留由本轮启动的 `SerialForge` EXE 进程。

最终产物：

| 产物 | 大小 | SHA-256 |
|---|---:|---|
| `dist/SerialForge/SerialForge.exe` | 3,030,152 B | `42DF56D59307D8EA96B589CEF786C61F1FE16A3172DC8A90EEDE69D09B1CD03B` |
| `dist/SerialForge.exe` | 47,525,190 B | `C5ECB78140BA0500894654F15DA41182D584BC6D58DB069990309E0FB9E155D8` |

## 未运行/未宣称

- 未运行真实 UART 长时间吞吐、USB 拔插、真实 LAN 压力、UDP 丢包/大 datagram、BLE 扫描/配对/MTU/通知/写入实机；
- 未运行 Modbus RTU、MAVLink、NMEA 的标准协议 codec/profile 或真实设备交互；
- 未运行 J-Link RTT；RTT 仍需要用户已安装并启动的 J-Link Telnet 服务和授权目标板，驱动/工具继续留到最后；
- 未运行干净 Windows、DPI/字体覆盖、代码签名、版本资源和第三方许可证发行审查；
- 未创建或修改单元测试、mock、fixture、test harness 或其他 test-only asset；
- 当前任务没有嵌入式 C/C++、固件、MCU、BSP/HAL/RTOS 或硬件源代码变更，因此厂商固件要求适用性为 N/A。

## 简化评估

通过复用既有 immutable `ProtocolConfig`、`configure_protocol()` 和 `reset_protocol()`，M5f 只增加一个
domain catalog 与 presentation editor adapter；没有为 preset 增加 application port、transport worker、
协议插件运行时、脚本或新的依赖。非 UART/TCP Client 直接禁用控件，避免把未接入 parser 的路径伪装成支持。
后续标准协议应使用独立 codec/profile 和 source/generation 语义，不继续向通用 preset 堆叠例外分支。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
