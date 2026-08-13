# ADR-0166：短页面垂直节奏与原生滚动边界

## 状态

已接受（ARCH-115 / UI-1.188，2026-08-12）

## 背景

ARCH-114 已让 themed workspace shell 正确承接可用高度，但短连接页和其他短设置页仍然顶部堆叠，
viewport 下半部分出现大块无语义背景。长协议页、扩展工具站则必须保持顶部起始与原生滚动，命令空态
还拥有自己的 Expanding canvas。

## 决策

新增 `ResponsiveScrollArea` 作为共享 `scroll_page()` 的薄 presentation wrapper：

1. 只引用既有 page `QVBoxLayout`，不复制 builder 内容、不创建新滚动层；
2. layout `sizeHint()/minimumSize()` 不超过 viewport 时使用垂直 `AlignVCenter`；
3. 内容超出 viewport 时恢复垂直 `AlignTop`，继续由原生 `QScrollArea` 滚动；
4. 保留当前水平 alignment、stretch、size policy、页面 API、accessibility、route strip 和 focus snapshot；
5. resize、show、scroll rangeChanged 触发同一幂等刷新，不启动 timer 或 MotionController。

## 被否决的替代方案

- 在每个页面 builder 中复制居中公式：引入四个 owner，后续维护和验证成本更高；
- 使用 `AlignHCenter`：会破坏表单的横向填充，造成窄列；
- 为短页新增固定 spacer 或手工 `setGeometry()`：会与 ARCH-114 根 stretch 和 Qt layout contract 冲突；
- 改 command empty state 的 Expanding 行为：会破坏其已有空态画布语义。

## 验证与审查

架构师 `019ff4ad-186a-7fb2-8a28-83e47bf9fa20` 有条件批准：只包装现有 layout、只切换垂直 alignment、
保留 command empty owner 和原生滚动；其验证矩阵已执行。架构师 `019ff4b1-dc64-76c1-85ec-0c3858663ba7`
批准删除不存在的 viewport resized 信号，仅保留 resize/show/rangeChanged 刷新。独立 reviewer
`019ff4b4-c995-7fb0-9ad2-c0b403706716` 本轮等待窗口超时关闭，未形成外部结论；父代理完成六轴
review 与 behavior-preserving simplification assessment。无嵌入式 C/C++ 改动，public-vendor-source
applicability 为 N/A。

三主题×三尺寸×四页面响应式矩阵 217 checks、生命周期矩阵 752 checks 均为 0 failures；静态
source-limit、theme audit、check、compileall、ruff 均通过。真实 GUI/HIDPI、EXE startup、硬件、签名
和 OTA/RTT 实连未运行。

## 交付

`local-arch-115` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；三者均为
`48,028,531` bytes，SHA-256 为
`8E98DB669C51C64813B701015A9DC2E2C9C7BAE4729025278BEECAF218713924`；archive listing SHA-256 为
`CD3C60D336971C88071C3A0D1C3B74D6AD090D1D2704F753C13B058E236913A0`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
