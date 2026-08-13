# ADR-0161：命令空态内容组水平居中

日期：2026-08-12  
状态：accepted  
范围：`presentation/command_batch_empty_state.py`

## 问题

命令管理页没有批量命令时，`CommandBatchEmptyState` 已经占据页面唯一可伸缩画布，但 glyph、
标题、说明和 CTA 仍贴在画布左侧。宽屏下右侧出现大块无语义空白，用户会感觉组件散开或页面
没有完成排版。

## 决策

继续由 `CommandBatchEmptyState` 作为 presentation 排版 owner：

1. 保留现有 `Expanding/Expanding` 空态外壳和命令页唯一 vertical stretch slot；
2. 在横向 root layout 两侧增加 stretch；
3. 让 glyph 与 copy 使用自然内容宽度，不再让 copy layout 吸收整行水平空间；
4. 保留 glyph、copy、CTA 的原有顺序、焦点语义、signal 和共享 frame 接口。

这让内容组在画布中水平居中，同时依靠既有 word-wrap 和自然 size hint 适应窄宽度；不引入固定
最大宽度、第二个滚动 owner 或新的响应式状态。

## 备选方案

### 固定最大宽度容器

拒绝：会增加窄窗口裁切和额外宽度策略，当前自然内容宽度已经足够稳定。

### 在命令 controller 中计算布局或宽度

拒绝：会把 presentation 排版泄漏到命令状态 owner，破坏高内聚低耦合边界。

### 用 root trailing spacer 或重新分配 vertical stretch

拒绝：ADR-0155 已确定空态是唯一 vertical stretch owner，本轮不改变该决策。

## 验证边界

真实 Qt offscreen 覆盖 `980×720`、`1240×820` 和三主题：

- content union `(glyph + eyebrow + title + hint + CTA)` 的中心偏差为 `1px`；
- 空态在 980px 为 `916×310`，1240px 为 `1176×399`，无横向/纵向滚动；
- 空态、标题、CTA accessible name/description 均非空，CTA 仍发出既有 `new_requested` signal；
- 隔离 signal、低动效、暂停、隐藏/恢复、关闭通过；共享时钟样本 `65` frames、均值 `8.261ms`、
  p95 `9.299ms`、有效约 `121.05Hz`，target `120Hz`、timer `8ms`；
- 三主题截图确认内容组居中且无白色断层。

本轮没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。嵌入式 C/C++
public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。

## 审查状态

架构师调用与独立 reviewer 调用均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。父代理
完成 correctness、architecture、security、performance、readability 五轴 review，以及保持行为
不变的简化评估；确认本轮仅改变一个 presentation leaf 的水平空间分配。
