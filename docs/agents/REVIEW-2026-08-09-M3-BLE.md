# M3 BLE GATT 六角色交付记录

日期：2026-08-09。范围：SerialForge 当前 checkout 的 Windows 单设备 BLE GATT Central
最小切片。J-Link RTT 按用户要求继续最后处理；本轮没有安装、引入或打包 J-Link/SEGGER/
probe-rs 依赖。

## 六角色结论

| 角色 | 结论 |
|---|---|
| 产品 | M3 只承诺手动扫描/过滤、显式选设备、服务/特征查看、read、两种 write、notify/indicate；不承诺自动重连、Bluetooth Classic SPP 或实机之外的 BLE 可靠性。 |
| 架构 | BLE 独立 `BleGattDiscoveryPort`、`BleGattTransportPort` 和 `BleGattSessionManager`；Bleak 对象不越过 infrastructure，GATT 操作不复用 UART/TCP stream。 |
| UI | BLE 面板提供扫描过滤、设备选择、连接/配对/缓存选项、特征属性、read、notify 和显式写模式；连接中不改变设备配置，断线不自动替换目标。 |
| 开发 | 所有 Bleak I/O 位于 dedicated asyncio worker；特征身份为 service UUID + characteristic UUID + handle；MTU 与 without-response 最大写长分开；不自动分片。 |
| 验证 | 无 BLE 硬件时只验证 domain/config、导入、静态、offscreen 启动/关闭和可选包；扫描、配对、服务发现、读写、通知、MTU 和断线重授权实机项目明确未运行。 |
| 打包/工作流 | `package.ps1` 默认不带 `ble`；`-Ble` 才同步 Bleak 并显式收集 Bleak/WinRT 子模块；主线与 BLE-enabled 包分开验证。 |

## 已交付边界

- `domain.models` 增加 BLE 设备、发现配置、服务/特征能力、GATT command 和 transport config；
- `domain.ports/events/errors` 增加 BLE 发现、异步传输、通知/读写/订阅事件和结构化错误；
- `infrastructure.ble_gatt_discovery` 使用可选 Bleak 扫描并只返回不可变快照；
- `infrastructure.ble_gatt_transport` 延迟导入 Bleak，负责 WinRT client、service snapshot、read/write/notify 和错误映射；
- `application.gatt_sessions` 独占 asyncio loop、Bleak client 生命周期和操作队列；
- `presentation.viewmodels/main_window` 接入手动 BLE 扫描、连接、特征读写和通知；
- `scripts/package.ps1 -Ble` 提供明确的 BLE-enabled 打包路径。

## 非硬件证据

已执行：

- `uv run --locked ruff format --no-cache src`；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- domain BLE value/config construction：通过；
- `import serialforge.composition`：通过，未导入 Bleak；
- Qt `offscreen`：切换到 BLE GATT 面板、确认 UART/network 面板隐藏、关闭和 session/recorder shutdown：通过；
- `scripts/package.ps1 -Mode onedir -Ble`：通过；BLE-enabled onedir 实际窗口启动和 `WM_CLOSE`：通过。
- 默认主线 `scripts/package.ps1 -Mode onedir` 和 `-Mode onefile`：通过；两个产物实际窗口启动、
  `WM_CLOSE` 和退出码 0；onedir/onefile 的最终 SHA-256 分别为
  `1AA604EFD453EAFB689C6C7E666F834E580D8EBDC08DD9BC58D6C447416C6DFB` 和
  `C7C301E6914837BCA0AD00D7165AFA845F2E4724BBA01012DF6B9650F818A33A`。
- 默认 onedir 产物没有名为 `bleak`、`winrt` 或 `bluetooth` 的打包文件；PyInstaller warning
  仅记录了可选 Bleak 延迟导入，符合主线包不带 BLE 的设计。两个 EXE 当前均为 `NotSigned`。

BLE-enabled onedir 构建日志只有可选 macOS `objc`/CoreFoundation 类收集警告，未发现 Windows
主路径错误。未把这些可选平台警告误报为 BLE 实机验证。

未执行：

- Windows Bluetooth adapter、Bleak WinRT scan、名称/service filter 实际结果；
- GATT service/characteristic discovery、read、write-with-response、write-without-response；
- notification/indication callback、pair/auth、MTU 和 backend 写长行为；
- 设备断电/离开范围后的断线事件、重新授权和多设备选择验收；
- 干净 Windows、真实 UART/LAN 和签名发行验收。

## 发行约束

BLE 主线使用可选 extra，不强迫 UART/network 用户安装 Bleak。发行前必须根据目标设备完成
实机清单、许可证/NOTICE 审核、签名和干净 Windows 安装验证；当前 EXE 仍未签名。
