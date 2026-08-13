# OTA / 安全升级 / RTT-J-Link 扩展边界交接

日期：2026-08-10

范围：contract-only 的 OTA、OTA security、RTT/J-Link debug 输出目录；UI station band 与开发约束同步更新。

## 已完成

- `src/serialforge/ota/contracts.py` 定义有界镜像描述、XMODEM/YMODEM/TFTP 传输选择、传输请求、进度快照和 `OtaTransferPort`。
- `src/serialforge/ota/security/contracts.py` 定义 AES-256-GCM / AES-128-CCM 候选、安全策略、验证结果和流式 security port；不包含密码学实现或原始密钥。
- `src/serialforge/ota/transports/{xmodem,ymodem,tftp}/` 建立独立协议槽位，当前均明确为未实现。
- `src/serialforge/debug/contracts.py` 定义 RTT/J-Link 原始日志 DTO/port；`debug/{rtt,jlink}/` 建立独立槽位，禁止扩张为探针控制。
- `src/serialforge/presentation/README.md` 明确 UI 只消费 DTO/capability，不接触 device handle、vendor SDK、socket、pyserial 或密钥。
- `docs/CONSTRAINTS.md`、`docs/ARCHITECTURE.md`、`docs/adr/0023-ota-debug-extension-boundaries.md` 写入目录、依赖方向、安全、授权、三协议占位和验收门槛。
- UI-1.8 将观测/发送工具条包装为暗色 station band，来源状态由既有历史标志驱动。

## 明确未完成

- XMODEM/YMODEM/TFTP 真正传输、bootloader 兼容性、重试/恢复/掉电续传。
- AES 加解密 backend、签名链、密钥托管、nonce、anti-rollback、目标身份、激活/回滚。
- J-Link SDK/DLL、RTT probe 操作、memory/halt/run/reset/flash、真实目标板验证。
- OTA/debug UI 页面和 composition/application worker 接入。
- GUI/offscreen/HIDPI/读屏/截图和真实硬件验证；用户未授权启动软件或连接硬件。

## 依赖方向

```text
presentation (PySide6)
        ↓ immutable DTO / capability
application worker (future)
        ↓ ota/debug contracts
ota transports / ota security / debug adapters
        ↓ target-specific backend (future, explicit authorization)
bootloader / external RTT Telnet service / J-Link environment
```

当前没有把任何 future backend 接进 composition root。`infrastructure/rtt_transport.py` 的既有
attach-only Telnet bridge 保持不变。

## 验证证据

- Ruff check：通过。
- Ruff format check：通过。
- `python -m compileall -q src\serialforge`：通过。
- `pwsh.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Mode onefile`：通过；provenance verification 通过。
- canonical/root EXE：`47,629,954` bytes，SHA-256 `D4362895D36C9458C534C00357C72A7F1AD9559B1156FDFACD22E05B38631154`，字节一致。
- 静态审计确认新 contract 不导入 PySide6、pyserial、socket、SEGGER/J-Link SDK 或外部 crypto 库。
- 代码质量/简化复核：未发现 Critical/Required；保留 contract-only 小接口，不新增通用插件工厂、第二套状态源或动态加载器；安全 profile 对签名和 anti-rollback fail-closed。
- 未创建、修改或运行测试专用资产；未启动 GUI/EXE/服务、未连接网络/串口/BLE/J-Link、未刷写/复位目标。
- 本轮没有嵌入式 C/C++/固件/驱动修改；MCU vendor source applicability=N/A，不能据此宣称固件或认证合规。

## 下一步

1. 由用户指定目标 MCU、bootloader、镜像格式、三协议中实际优先项和目标厂商公开版本资料后，再启动独立 OTA implementation slice。
2. 由用户授权一次 GUI/offscreen 视觉验收后，检查链路连接页、两个 station band、滚动条 corner 和下拉菜单是否零白色泄漏。
3. 当前 onefile 已覆盖根目录 `SerialForge.exe`；下一次源码变更后仍必须重复唯一打包入口并重新核对 hash。
