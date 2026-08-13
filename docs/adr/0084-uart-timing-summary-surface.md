# ADR-0084：UART 参数摘要 surface

日期：2026-08-10  
状态：Accepted  
范围：UART connection form 的 presentation summary

## 背景

UART 表单已经把波特率、数据位、校验、停止位和流控收敛为 bounded combo，满足“不让用户手输常用数字”的要求。
但在密集网格中，用户需要横向回看多个字段才能确认当前 wire format；快速配置填入后也缺少一个统一确认点。

## 决策

- 新增 `UartTimingSummarySurface`，以只读 status rail 显示 `baud · data/parity/stop · flow` 摘要。
- surface 只接收 typed enum/value，不读取 ViewModel、domain config、transport 或协议状态；builder 负责把现有 combo signal 和快速配置结果接入。
- 无效暂态显示“当前 UART · 参数待选择”；有效状态同时更新 tooltip 和 accessible description。
- 使用已有 semantic theme token，不新增 palette、timer、业务状态或连接动作。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI194_UART_SUMMARY_VECTOR_PASS default=1 manual=1 preset=1 themes=3
```

Qt offscreen vector 未显示主窗口，覆盖默认 `115200 · 8N1 · 无流控`、手动切换、快速配置更新和三主题 polish。
未运行 HIDPI、读屏、真实设备、OTA/AES/RTT/J-Link、硬件、签名和正式发行验收；未创建、修改或运行 unit test、
mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。
