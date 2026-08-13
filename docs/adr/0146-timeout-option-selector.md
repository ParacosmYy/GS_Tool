# ADR 0146：超时配置改为不可编辑选项选择器

- 日期：2026-08-12
- 状态：accepted
- 范围：`presentation/bounded_value_combo.py`、连接 builders 与 runtime bindings

## 背景

连接页的超时参数以可编辑浮点框呈现。用户需要理解秒的小数、手动输入并承担超出合理范围的
配置成本；这与串口工具“常用值直接选择”的使用目标不一致。

## 决策

新增 presentation-only `BoundedFloatCombo`，由 `controllers/composition.py:timeout_combo()`
统一构造 UART、TCP/UDP/RTT、BLE 八个超时控件。选项以 `ms/s` 显示，内部 item data 仍保存秒数；
允许 `0` 的字段显示“未设置”。绑定保留 `.value()`/`.setValue()`，运行时和 domain DTO 不改。

`setValue()` 对超界值执行与原 `QDoubleSpinBox` 相同的边界裁剪；不在 catalog 的程序化旧值只添加
一个不可编辑临时 item，以保持兼容，不开放用户文本输入，也不把选项值存进第二份业务状态。
后续其他数值字段是否选项化，必须由各自 bounded context 单独评估，不能把端口/帧长等无关规则
强行放进 timeout catalog。

## 结果与边界

用户获得 1 ms–60 s 范围内的常用选项，八个控件共享一致的可访问性和主题化 `QComboBox` 外观；
连接构建、session、transport、OTA/debug contract、唯一 MotionController 和 120Hz scheduler
均不变。未新增 timer、线程、事件总线、scroll owner、设备访问或跨层依赖。

## 验证

真实组合根离屏验证 `ARCH95_TIMEOUT_SELECTOR_PASS`：8 个控件均不可编辑，option count 为
`[10, 11, 12, 16, 16, 16, 16, 16]`，默认值保持，超界裁剪和 `0.123` 秒程序化值读取通过；
980×720 首屏无新增横向挤压。Ruff、compileall、`scripts/check.ps1` 和 source-limit/theme audit
通过。架构师与独立 reviewer 调用均超时关闭，父代理完成五轴复核和行为保持简化评估；本轮不涉及
嵌入式 C/C++，public-vendor-source applicability 为 N/A。
