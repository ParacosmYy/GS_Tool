# ADR 0023：OTA 与嵌入式调试扩展边界

状态：已接受；本轮仅完成 contract-only 目录骨架和约束记录，真实 OTA、加密 backend、J-Link
SDK/探针操作和硬件验收后置。

## 背景

用户希望把 SerialForge 逐步扩展成嵌入式调试工具站，并预留 OTA 传输、安全升级、AES 类加密、
RTT/J-Link 打印以及独立 UI 目录。当前产品是 Windows-first Python/PySide6 桌面工具，已有
UART、网络、BLE 和 attach-only RTT Telnet 切片。若直接把刷写、加密、探针控制塞进 MainWindow
或现有通用 transport，会让 UI 依赖硬件细节，并把尚未验证的能力误显示为已支持。

## 决策

- 在 `src/serialforge/ota/` 预留独立 contract 层；`contracts.py` 只定义有界镜像元数据、协议
  选择、传输请求、进度和 `OtaTransferPort`，不打开文件、不发送数据、不调用 Qt。
- 在 `ota/transports/` 分别预留 XMODEM、YMODEM、TFTP 三个 adapter 目录。三者只是协议变化
  边界，必须由目标 bootloader 的公开资料、包格式、超时/重试/恢复语义和硬件证据分别激活。
- 在 `ota/security/` 预留安全端口和策略 DTO；只允许带认证的 AES-GCM/CCM 候选，DTO 只包含
  `key_id`/签名 key 引用，不包含原始密钥。实现必须使用审查过的密码库或平台密码服务，
  在摘要、签名、目标身份、anti-rollback、nonce、密钥生命周期和激活/回滚验证完成前 fail-closed。
- 在 `src/serialforge/debug/` 预留独立 RTT/J-Link 原始日志 contract、`rtt/` 和 `jlink/`
  adapter 槽位。它们只表达有界 channel/payload/时间和显式 Down 写入，不获得 memory、halt、
  run、reset、flash 或任意 vendor 命令能力；现有 `infrastructure/rtt_transport.py` 仍负责
  已实现的 Telnet attach-only 桥接，未来 adapter 不得反向依赖 UI。
- `src/serialforge/presentation/` 保持独立 UI 目录；UI 只渲染 application DTO/capability，
  不导入 OTA/debug backend、设备库、socket 或密钥。

## 备选方案

### 直接把 OTA 控制放进 MainWindow

拒绝：会把文件选择、升级状态、传输协议、密钥策略和 Qt 生命周期绑在一个大型窗口中，无法
单独验证或替换协议，也容易让“发送完成”被误读成“设备升级完成”。

### 先引入一个通用插件市场/动态加载器

拒绝：动态代码加载会扩大供应链和权限边界；当前只保留静态、类型化、可审查的 adapter 端口，
待第三个真实实现和明确沙箱/签名方案出现后再评估。

### 仅使用 AES-CBC/ECB 作为“加密升级”

拒绝：单纯保密不提供完整性或认证；当前约束要求 authenticated encryption、签名和回滚策略，
且不在本轮自写密码学实现。

## 后果

- 本轮新增目录清晰展示未来能力，但不会改变当前 UART/网络/BLE/RTT 运行行为，也不会改变 core
  打包内容或引入新依赖。
- 未来实现需要新增 application orchestration、目标配置和独立验收记录；不能仅凭 contract
  文件把功能标记为完成。
- 三种协议的具体选择是路线占位，不是对所有 bootloader 的兼容性承诺；若目标要求 HTTP(S)、
  USB DFU、BLE DFU 或厂商协议，应新增独立 adapter 并写新的 ADR/公开来源记录。

## 验证与未决项

- 已执行：Ruff check、Ruff format check、Python compileall；新增 contract 不导入 PySide6、
  pyserial、socket、vendor SDK 或外部密码库。
- 未执行：OTA 文件传输、加密/签名、bootloader 激活/回滚、真实 RTT/J-Link、探针操作、GUI
  视觉验收和硬件验收；用户未授权这些操作，且当前没有目标 MCU/bootloader 型号和版本。
- 适用性：本轮没有嵌入式 C/C++/固件/驱动源码修改，MCU vendor requirement applicability=N/A；
  contract 仍需在未来绑定具体 MCU、bootloader、SDK、工具链后按目标一手资料复核。
