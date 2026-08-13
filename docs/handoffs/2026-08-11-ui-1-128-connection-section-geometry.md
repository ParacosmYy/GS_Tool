# UI-1.128 交接：连接页 section 标题几何收敛

日期：2026-08-11  
范围：修复连接页 UART/网络/BLE section 标题在最小窗口和专注设置模式下被垂直拉伸造成的空白。

## 实现

`src/serialforge/presentation/controllers/connection_builder.py` 新增 `_section_label()`，统一
构造三个连接 section 标题。helper 保留既有 `role="section"` 主题语义，只设置横向
`QSizePolicy.Preferred`、纵向 `QSizePolicy.Fixed`；没有修改 QSS、connection runtime、transport
DTO、panel 可见性、连接状态或焦点/无障碍文案。

修复前 980×680 专注设置视图中 `UART 参数` 标题高度约为 106px；修复后运行时高度为 26px，
UART 参数面板紧随标题展开，视觉空白消失。

## 验证

- 三主题 × 980×680/1180×780。
- UART、TCP Client、TCP Server、UDP、BLE GATT、J-Link RTT 六种连接方式，共 36 组真实组合根。
- active section title/panel 无重叠；标题高度集合为 `[26]`。
- 当前页 horizontal maximum=0；exact-white=0。
- 专注模式与总览模式过渡均完成收敛。
- `scripts/check.ps1`：通过；源码 158 个文件均不超过 1000 行。
- `python -m compileall -q src`：通过。
- `uv run ruff check src`：通过。
- onefile provenance verify：通过。
- 视觉证据：`build/ui_review_ui128_connection_section.png`。

架构师线程 `019fed71-9d02-7731-b268-c48f59327b28` 在限定窗口内超时，未计为独立通过；父代理
完成 correctness/readability/architecture/security/performance、简化与生命周期复核。未修改嵌入式
C/C++；`embedded-enterprise-workflow` 与 `embedded-code-review-simplifier` 对本轮源码不适用；
public vendor applicability=N/A；未操作目标硬件。

## 交付

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.128`  
size：47,947,626 bytes  
SHA-256：`D7EB3B79B0E86F46AB24AF5B951496B25F1797627021DA4A7786545A87A951B1`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
signature：`NotSigned`；`release_eligible=false`；hardware acceptance：`not_run`

根目录 `SerialForge.exe` 仍被 PID `46108`、`49236` 占用，尚未强制结束；关闭旧实例后再进行
根目录覆盖。
