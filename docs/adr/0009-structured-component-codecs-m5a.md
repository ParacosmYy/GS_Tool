# ADR 0009：M5a schema v2 JSON/TLV component codec

## 背景

M4b 的 profile v1 能解析固定 offset/length 字段，但传感器日志和简单设备协议常把有用值放在
JSON 对象或平面 TLV 中。把这些判断塞进 framing、transport 或 Qt 会破坏当前的高内聚边界，
而任意 JSONPath、脚本和动态插件又会让配置不可控、打包不可预测。

## 决策

- 保留 `ComponentProfile` schema v1 的 JSON 形状、未知字段校验、`BinaryComponentCodec` 语义
  和显式加载入口；不自动迁移、不静默改写；
- 新增 schema v2 顶层 `codec` 块，由 `domain.codecs.ComponentConfigurationCodec` 严格加载，
  `ComponentCodecRouter` 只静态注册内置 JSON/TLV codec；
- JSON v1 只支持严格 UTF-8、有限 JSON Pointer、string/number/boolean/json 期望类型和
  number 的 scale/unit；禁止 wildcard、filter、slice、递归、表达式和脚本；
- TLV v1 固定为 `tag | length | value`，tag/length 宽度 1/2/4，共用显式端序，length 只表示
  value；无嵌套、填充和自动重同步，重复值通过显式 occurrence 绑定；
- codec 只消费 `DecodedFrame`，只生成 `ComponentFrameRow` 派生数据。无效 JSON、缺失字段、
  类型错误、截断 TLV、未知/重复 tag 和资源超限保持 raw Hex 与 row/field/codec error 可见；
- 本轮只扩展现有 UART/TCP Client RX component bridge。UDP、TCP Server、BLE、RTT 暂时继续
  raw-only，未来必须各自定义 datagram/peer/GATT/channel 上下文；
- 使用 Python 标准库，不新增运行时依赖；schema v2 只做 RX decode，不生成 TX。

## 取舍与风险

JSON Pointer 比 JSONPath 表达能力小，但语义容易审计且不需要第三方依赖。平面 TLV 不做自动
重同步，因此坏长度会让当前 frame 的 TLV 派生结果进入 error，但不会吞掉下一帧：下一帧由
上游 framing 决定。 `DecodedFrame.payload` 是 framing/checksum 之后的派生 payload，原始线缆字节
仍以 recorder 的 raw record 为真相。

codec worker 在解码时不持有配置锁；generation 变化会丢弃旧派生结果。队列、字段数、路径、
JSON 节点/深度、TLV 项数、字段值和显示长度都有边界。未来新增 codec 应先添加独立 typed
config、静态注册和对应验证矩阵，不能直接扩展现有 binary `FieldSpec`。

## 验收证据

本轮记录 JSON v2 round-trip、嵌套 JSON/scale、JSON malformed、TLV endian/scale、TLV truncation、
legacy v1 round-trip、worker event 和 raw-preserving vectors；随后执行 Ruff、compileall、
offscreen Qt 和 PyInstaller 默认包启动/退出。真实设备吞吐、多传输 codec 覆盖、迁移、曲线、
回放和 clean Windows 发行验收仍未运行。
