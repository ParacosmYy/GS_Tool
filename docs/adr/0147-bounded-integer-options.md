# ADR-0147：有限整数配置使用不可编辑选项

日期：2026-08-12

状态：已采用（ARCH-96 / UI-1.169）

## 背景

协议帧长、UDP 报文上限、TCP Server 客户端数和批量命令延时属于常用且有限的配置档位。让用户
直接输入详细数字会增加认知负担，也容易把不合法值带入 UI。端口与主机则仍属于自由输入，不能
为了统一外观而强行变成有限选项。

## 决策

- 在 `presentation/bounded_value_combo.py` 内提供 presentation-only 的 `BoundedIntCombo`，
  保持 `.value()`、`.setValue()`、`.setRange()` 的窄兼容接口。
- 四类业务值各自拥有 option catalog：帧长、UDP 报文、Server client 数和批量延时不共享领域
  规则；控件只负责显示、范围和旧值兼容。
- 非 catalog 的程序化旧值插入一个不可编辑临时项，保证导入/恢复不静默改值。
- 协议固定上限切换时，在一次 signal-blocked rebuild 中清空、重建和选择，避免 dirty callback
  被中间状态重复触发；用户真实选择仍发出既有 signal。
- selector 设置 96–148px 紧凑宽度，避免 980px 工作区横向溢出。UART/网络端口继续使用可输入
  `QSpinBox` 或端点输入。

## 影响与验证

该决策没有新增 timer、线程、业务状态、事件总线、scroll owner、设备 I/O 或 OTA/debug coupling。
真实组合根离屏验证通过：四工作区逐页 `horizontal_max=[0, 0, 0, 0]`，UDP `65507 B`、Server
`16 clients`、协议旧值 `12345`、批量延时 `1234 ms` 均可读取；120Hz MotionController
`TARGET_HZ=120`、`_BASE_INTERVAL_MS=8` 保持不变。Ruff、compileall、源码行数门禁和主题 token
审计通过。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability 为
N/A；不作 MISRA、ISO 26262、ASIL 或认证声明。架构师与独立 reviewer 调用超时关闭，未形成外部
结论；父代理完成五轴复核与行为保持简化评估。
