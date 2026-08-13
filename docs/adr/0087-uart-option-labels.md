# ADR-0087：UART 选项的用户文案与 typed value 分离

日期：2026-08-11  
状态：Accepted  
范围：UART connection panel 的五个 selector

## 背景

UART 表单的底层值已经是有界预设和 domain enum，但数据位、校验、停止位、流控仍显示英文或裸数字；
用户需要把技术枚举映射到实际串口线格式，屏幕阅读器也缺少统一的字段说明。

## 决策

- 保留 25 个常用波特率预设，并显式设置波特率 `QComboBox` 不可编辑，继续禁止任意数字输入。
- 将数据位、校验、停止位和流控的 visible label 本地化为中文；只改变 label，不改变 `itemData`。
- 为五个 selector 补齐 tooltip 和 accessible description，说明选择范围与实际含义。
- 不把 UI 文案放进 domain/application，不新增 option catalog、业务状态、连接动作、timer 或主题色。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI197_UART_OPTIONS_VECTOR_PASS themes=3 baud_presets=25 localized=data/parity/stop/flow non_editable=1 typed_data=1
```

向量使用真实组合根的 Qt offscreen 内存对象且未显示主窗口；未连接串口、未启动 EXE 或硬件，未创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。本轮没有嵌入式 C/C++ 变更，MCU vendor source applicability=N/A。
