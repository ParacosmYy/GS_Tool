# ARCH-6r：UART 控件组合绑定

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

新增 `src/serialforge/presentation/connection_bindings.py`，提供 frozen/slots
`UartControlBindings` 与 `uart_bindings_for()`。`connection_builder.py` 是唯一组装 owner；
连接状态、连接动作、组合/焦点、preset、生命周期、Modbus timing 和终端端点投影均通过该
typed bundle 消费 UART widget 引用。TCP、UDP、BLE 和 RTT 的既有边界未被本切片扩大。

## 行为保持

保留常用波特率下拉、UART preset、COM 端口可编辑语义、Modbus timing、连接 gate、UART
section/title 显隐、Tab 顺序、focus mode restore、tooltip/accessibility 和三套主题 QSS。
缺少 bundle 时投影函数安全返回；配置路径明确报错，不生成虚假端口。

## 验证证据

- `ARCH6R_COMPILE_PASS`
- `ARCH6R_RUFF_PASS`
- `162 files <= 1000` 源码行数门禁
- `scripts/check.ps1` 通过，3 themes / 22 tokens / 19 selectors / `legacy_qss_literals=0`
- `UART_DYNAMIC_FIELDS_OWNER_ONLY_PASS`：UART 动态字段只在 `connection_builder.py` 直接构造
- `ARCH6R_UART_VECTOR_PASS 24 focus_restore=pass themes 3`
- 三主题 × 980/1180 × 四 Tab：`hmax=0`、`exact-white=0`、`near-white=0`
- `local-arch-6r` onefile 已完成：canonical/root/root-latest 均为 `47,963,397` bytes，SHA-256
  `BF0EDF73CC7BDFBA5567DC9CC6A55D8365F6979D65BDEF56EB55637790095529`，archive listing SHA-256
  `C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，provenance verify pass；
  `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。根目录两个 EXE 已覆盖并
  与 canonical 字节一致。

运行时有一个既有 Qt font-directory warning（PySide6 安装目录缺少 fonts 子目录）；系统字体
仍可用于离屏渲染，未把该 warning 误判为 UI 白色回退。GUI/EXE 启动、读屏、真实设备、硬件
烧录和部署验收未运行。

## 角色与简化审查

架构师线程 `019ff103-cb0d-7610-8306-66dffee0b675` 已调用但超时关闭。独立嵌入式 reviewer
线程 `019ff108-4743-7c21-989d-861a1dd8a496` 未在等待窗口内返回；父代理完成 owner、依赖、
初始化顺序、Qt 生命周期、行为保持、可访问性、主题和性能风险审查。复用既有 workspace
typed binding 模式，并保持 builder 内迁移兼容字段，避免引入第二套状态或一次性大规模改名。

嵌入式 C/C++/固件适用性：N/A；无 public vendor source applicability；无硬件授权操作。
