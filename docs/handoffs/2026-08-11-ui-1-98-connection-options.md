# UI-1.98 连接页选项 affordance 交接

日期：2026-08-11  
范围：网络、RTT、TCP Server、BLE connection panel 的选择式配置与中文辅助说明

## 结果

- 网络主机/端口、本地绑定、连接/读写超时、UDP 报文上限补齐 tooltip/accessibility description，保留既有类型范围与默认值。
- RTT 通道改为 `0 · 终端` / `1 · 数据`，BLE 写入模式改为 `写入 · 等待响应` / `写入 · 不等待响应`，并显式不可编辑；底层 `itemData` 未改变。
- TCP Server allowlist、LAN 确认、最大客户端、发送目标，以及 BLE 扫描/过滤/设备/缓存/配对/特征/通知控件补齐上下文提示。
- 超时控件的零值特殊状态由英文 `None` 改为“未设置”，zero-as-unbounded 语义保持不变。
- 本轮没有新增 OTA/debug backend、网络探测、vendor 工具启动、连接动作、状态源、timer 或依赖；`main.py`/`MainWindow` 仍保持 composition/lifecycle shell 边界。

## 角色与独立复核

```text
产品角色       019fec75-bdf2-7c70-b198-1605e01aad20  called; wait timed out; closed
架构角色       019fec75-be38-76b3-a53b-3fcec46cdf89  called; wait timed out; closed
UI 设计角色    019fec75-be84-75b1-8d74-276dacb9f6bc  called; wait timed out; closed
开发角色       019fec75-bed3-78a1-8f8c-1dd9751abad1  called; wait timed out; closed
验证角色       019fec75-bf27-7e30-a1dd-ac98480a640c  called; wait timed out; closed
打包角色       019fec75-bf72-73e0-a49b-e61257489863  called; wait timed out; closed
独立质量复核   019fec79-110e-7c22-8664-c0c0395430b8  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认展示 label 与 typed data 分离、
零值文案不改变 runtime 语义；readability/simplicity 确认复用既有 timeout/combobox helper；architecture 确认修改仅位于
presentation builder；security 确认没有新增输入、I/O、密钥或 vendor 依赖；performance 确认没有新增 timer、线程或热路径工作。
嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI198_CONNECTION_OPTIONS_VECTOR_PASS themes=3 rtt_localized=1 ble_write_localized=1 typed_data=1 none_localized=1 affordances=16
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

真实组合根通过 Qt offscreen 内存向量，未显示主窗口；Qt 报告 PySide6 环境缺少 fonts 目录的 warning，不影响断言。
未运行真实串口、网络、BLE、HIDPI、读屏、硬件、EXE 启动、签名或正式发行验收。没有创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.98
size: 47,923,829 bytes
SHA-256: DECC92C2D15BB110FB0977AEAB57A8BDD9E814B36F7037BBAFF0F0E5FBE2D753
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
