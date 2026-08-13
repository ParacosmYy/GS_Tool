# M4b profile/codec/组件视图 review（2026-08-09）

本轮按项目工作流启动产品、架构、UI、开发、验证、打包/流程六个只读角色；父级是唯一源码写入者。J-Link RTT 没有启动，仍按用户要求排在所有基础能力之后。

## 交付边界

- `domain.components`：严格 schema v1、`FieldSpec`、`ComponentProfile`、固定字段 `BinaryComponentCodec` 和有界 `ComponentFrameRow`；
- 支持 `hex`、`utf8`、`uint`、`int`、`float32`，显式 offset/length、byteorder、scale、unit；不执行 Python、表达式、命令、DLL 或动态 codec；
- `application.components.ComponentPipelineWorker`：独立 daemon worker，队列数量/字节双上限，配置/reset generation，组件背压可见；
- `ProtocolEventBridge` 把 raw UART/TCP Client stream event 从 application event bus 送入 parser，修复 presentation 作为 parser ingress owner 的扩展风险；
- `ComponentEventBridge` 把协议帧送入 component worker，raw recorder、协议帧和字段结果职责分离；
- UI 显式加载 JSON profile、显示 bounded component table、支持全部/有效/错误过滤和当前最多 256 行 CSV 导出；
- UDP、TCP Server peer、BLE notification/read 仍保持 raw only；M4b 不扩张它们的组件语义。

## 资源与错误边界

| 资源 | 上限 |
|---|---:|
| Profile JSON | 64 KiB |
| 字段数 | 32 |
| 单字段 bytes | 1 KiB |
| 字段名 | 64 字符 |
| component queue | 128 items / 256 KiB |
| UI rows | 256（表格最多显示最近 200） |
| parser event batch | 512 frames |

未知 schema/键、重复字段、类型/长度/端序错误和字段越界都转为结构化错误或 row-level error；
字段失败不会删除 `DecodedFrame` 或 JSONL raw。CSV 是视图导出，不替代原始记录。

## 六角色与独立复核

| 角色 | 结论 |
|---|---|
| 产品 | 父级冻结为 profile/固定字段/table/filter/CSV 最小切片；TLV、曲线、回放和迁移后置。 |
| 架构 | 通过整改；parser 与 component worker 分离，event bus bridge 代替 presentation 直接喂 parser。 |
| UI 设计 | 通过；协议配置旁增加 profile 加载、过滤、表格和导出入口，终端/raw 保持原布局。 |
| 开发 | 通过；新增代码只跨 domain/application/presentation 端口接线，不引入运行时第三方依赖。 |
| 验证 | 通过非硬件门；inline profile/codec/worker/bridge、静态、offscreen、打包通过。 |
| 打包/流程 | 通过；默认包继续不收集 Bleak/WinRT，BLE 变体显式 `-Ble`，组件 profile 为用户数据不内置任意脚本。 |

独立 assurance 子代理均确认本仓库没有 MCU、C/C++、BSP/HAL/CMSIS、RTOS 或固件目标；
vendor/芯片/SDK/编译器契约为 N/A，没有宣称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。
父级复核了 worker 锁/队列/关闭、profile 输入、字段 bounds、observer 重入和 UI 生命周期；
未发现 P0/P1 阻断。保留风险是长时间压力、真实设备吞吐和后续复杂 codec 的资源/迁移语义。

## 实际验证

- `uv sync --locked --extra dev`、`.\scripts\check.ps1`、`uv lock --check`：通过；
- `uv run --locked ruff format/check`、`python -m compileall -q src`：通过；
- inline vectors：profile dump/load round-trip、端序/scale、未知 kind 拒绝、字段结果、component worker event、raw→protocol→component bridge：通过；
- `QT_QPA_PLATFORM=offscreen` + Qt event loop：MainWindow、组件表格/profile controls、session/protocol/component/recorder shutdown：通过；
- 默认 onedir：`dist/SerialForge/SerialForge.exe`，2,906,713 bytes，SHA-256 `E993D32E5BD769F0FB670FF33DFFF8374622C7F73E253F6F78DA4C84A531EA7D`；实际启动/WM_CLOSE 通过；文件名扫描未发现 Bleak/WinRT/Bluetooth/SEGGER/J-Link/probe-rs；
- 默认 onefile：`dist/SerialForge.exe`，47,403,109 bytes，SHA-256 `0145B9CD106FB20FF4BAD253C6714BF7678C3DB275A2BB9217F31897D84EF66D`；实际启动/WM_CLOSE 通过，结束无残留 `SerialForge` 进程；
- BLE-enabled onefile：`.\scripts\package.ps1 -Mode onefile -Ble`，48,815,998 bytes，SHA-256 `D0666AD52F9DF94C430BDC4ED72BF1A33B52710C4F50A599CB6FE269CB5D21F6`；实际启动/WM_CLOSE 通过。随后已重新构建默认 onefile，最终 `dist` 主线不带 BLE extra；
- 默认 PyInstaller warning 只含 delayed optional `bleak` import；`uv` 默认环境 `importlib.util.find_spec('bleak') is None`。

## 未运行项目

- 未接 USB-UART、真实网络或 BLE 设备，未验证帧吞吐、拔插、通知、配对、MTU 和长时间背压；
- 未在 clean Windows 机器验证驱动、字体、安装器、签名和实际第三方许可归档；
- 未实现/验证 TLV、JSON Lines codec、复杂 transform、schema migration、曲线、回放和大型数据集；
- M6 J-Link RTT 仍未实现，SEGGER/J-Link 驱动、授权和硬件等用户环境条件继续留到最后。

## Gate 结论

M4b 达到代码、静态、手工、offscreen 和发行启动的非硬件门；下一阶段仅在用户/产品继续要求时推进高级 profile/可视化，J-Link RTT 仍保持最后路线。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
