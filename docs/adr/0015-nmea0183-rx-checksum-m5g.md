# ADR 0015：M5g NMEA 0183 RX line checksum

## 状态

已接受；M5g 代码完成，真实 GNSS/UART、标准版本覆盖和字段 codec 仍未验收。

## 背景

通用 Line framing 已能处理 CRLF 文本帧，但 NMEA 0183 的 checksum 不是一个尾部原始字节：
它是 `$` 与 `*` 之间内容的 8-bit XOR，并以两个 ASCII 十六进制字符表示。直接复用现有
`XOR8` 会把 `*HH` 当成错误的二进制尾部。NMEA 官方标准正文受版权保护，不能把标准句型表
或完整文本复制进仓库；需要一个有界 RX 校验切片，并将设备规格作为工程参考而不是泛化的厂商要求。

## 来源适用性

- NMEA 官方页面：[NMEA 0183](https://www.nmea.org/nmea-0183.html)，用于标准归属、版本/版权
  边界和不宣称认证兼容；未将其付费标准正文复制为工程规则；
- Quectel 第一方公开文档：[LC29H(BS) GNSS Protocol Specification V1.0](https://www.quectel.com/content/uploads/2024/02/Quectel_LC29HBS_GNSS_Protocol_Specification_V1.0.pdf)，
  §2.1 的 `$`、`*`、两个 ASCII checksum 字符和 XOR 线格式仅适用于该设备规格，作为当前
  parser 手工向量的工程参考；其他设备仍需按实际授权手册确认。

## 决策

- 在现有 `ProtocolConfig` 中增加显式 `NMEA0183` checksum kind，但只允许 `Line (LF/CRLF)`；
- NMEA decoder 要求 `$` 开始、唯一 `*`、非空 ASCII body 和正好两个 ASCII hex 字符；格式错误
  发布 `INVALID_FORMAT`，数值不匹配发布 `INVALID_CHECKSUM`；
- valid 派生 payload 去除 `*HH`，原始 ingress、terminal 和 JSONL recorder 不改变；
- 增加一个 NMEA line preset 和协议面板选项；选择仍只填编辑区，必须点击“应用”；
- 不解析 RMC/GGA/AIS 字段，不做 TX 编码/自动补 checksum/ACK/重试，不覆盖标准版本和方言全集，
  不引入第三方 NMEA 库或动态插件。

## 结果与风险

NMEA 校验沿用既有 bounded line parser、worker、component 和 Dataset 生命周期，没有 transport
分支或新运行时依赖；错误帧仍可见。`DecodedFrame` 当前没有独立 `wire_payload` 字段，但 raw
terminal/recorder 已保留完整线缆字节；invalid checksum 的派生 payload 按既有 checksum 语义去除
校验后缀。parser generation/迟到事件风险保持记录，后续单独处理。

## 验证

使用 Quectel 公开示例及临时 bytes 向量验证正确/错误 checksum、大小写 hex、非法格式、分片、CRLF、
后缀去除和禁止非 Line 配置；再执行 Ruff、compileall、Qt offscreen、默认 onedir/onefile startup/close
和 vendor runtime scan。真实 GNSS/UART、噪声/丢字节、干净 Windows、标准许可/认证与字段 codec 未运行。
