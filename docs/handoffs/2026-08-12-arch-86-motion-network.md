# ARCH-86 / UI-1.159 动效疏朗与网络面板 owner

日期：2026-08-12

## 实现

- `SignalFieldWidget` 保留固定 148×34、共享 `frame_changed` 和静态 fallback；将移动点从 5 个改为
  3 个等间距轨道点，缩短 trail，减少网格线与 sparkle，降低小面积装饰面的堆叠感。
- `MotionController` 未改动：仍为唯一共享计时器，`TARGET_HZ=120`、`PreciseTimer`、8ms scheduler
  target、elapsed phase、暂停/低动效/隐藏/关闭 fence 保持。
- `controllers/network_builder.py` 新增为 TCP/UDP/RTT 控件构造 owner；`connection_builder.py` 只
  组装 shell/UART、插入 network/BLE typed bundle，并保留 endpoint signal wiring。
- `controllers/connection_primitives.py` 新增无状态视觉 helper；网络 grid 间距为 12×10px，减少
  控件相互贴靠，不改变字段或业务语义。

## 契约

`NetworkControlBindings` 字段、window `_network_*` 兼容引用、默认值、范围、server peer callback、
allowlist/LAN gate、`network_bindings_for(window)` accessor 和 `frame_changed(float, bool)` 均保持。
没有新增 timer、线程、backend、socket、Telnet、J-Link、OTA/AES 执行路径或硬件动作。

## 验证证据

```text
uv run ruff check src scripts                         pass
uv run python -m compileall -q src                   pass
scripts/check.ps1                                    pass
ARCH86_IMPORT_PASS target_hz=120 interval_ms=8 network_owner=network_builder
source line limit                                      pass (172 files <= 1000)
theme token audit                                     pass (3 themes, 22 tokens, 19 selectors)
```

ARCH-86 Luna/max 架构师后续返回 `PASS / Required=0`：确认 MotionController 唯一时钟、SignalField 三点
等距、`NetworkControlBindings` 27 字段、peer/endpoint 信号未重复或遗漏。此前仅审查网络提取边界的
架构师调用在服务窗口内超时并关闭，未伪造结果；本轮父代理保持唯一写入并完成 fresh-pass。

本轮没有启动 GUI/EXE，没有运行三主题几何、真实显示器帧率、硬件连接、HIL、签名或正式发行验收；
GUI/EXE startup 记录为 `not-run`。这是当前 checkout 的非破坏性验证边界，不代表功能失败。

## 嵌入式 assurance

本轮是 Python/PySide6 presentation-only，无 C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、driver、
协议固件、bootloader、Flash/NVM、power 或 motor-control 修改；public first-party vendor source
applicability 为 N/A。未声称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。嵌入式简化评估为 N/A；
通用代码 review 与 simplification 将按 ADR 0137 的无行为改变边界记录。

## 交付

`local-arch-86` onefile 已完成并覆盖 canonical artifact、root `SerialForge.exe`、
`SerialForge-latest.exe`：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 48,001,801 bytes
sha256: F170A3893B519061953F24A414F9848B8B4D6D862011EDE50C3DA405C70EB280
archive listing sha256: 1CA579D3A3938886CDC7CB1EA3E4488D89467A296FC64C600CE7CC3C101B0867
provenance: pass; source revision local-arch-86
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
root EXE startup: not-run (current checkout policy)
```
