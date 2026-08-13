# ADR 0014：M5f 有界通用协议预设

## 状态

已接受；M5f 代码完成，真实设备和标准协议 codec 仍未验收。

## 背景

协议面板已经能编辑 `ProtocolConfig`，但重复填写 framing、delimiter、长度字段和 checksum
会增加 UART/TCP Client 的首次使用成本。下一步需要提高可用性，同时保持协议解析器、组件管线
和 transport 生命周期不变，并继续把 J-Link RTT 留在最后硬件阶段。

## 决策

- 在 `domain/protocol_presets.py` 提供最多 16 个不可变内置 `ProtocolPreset`；每项只包含有界
  key、label、description 和既有 `ProtocolConfig`；
- presentation 只把选中的 preset 载入编辑区，点击“应用”后才调用现有 ViewModel 配置入口；
- 配置与某个内置 preset 相等时显示该 preset，否则回退为“自定义”；不保存、不执行脚本、不
  动态导入协议实现；
- 只提供 generic framing/checksum 模板，不把 Modbus RTU、MAVLink 或 NMEA 的完整语义包装成
  已完成能力；标准协议后续各自使用独立 codec/profile 边界。

## 结果

预设目录是纯 domain 代码，不新增运行时依赖，不增加端口或 transport 分支。配置应用仍复用
既有 reset/错误/派生管线，原始终端和 recorder 路径不变。代价是预设目前只覆盖通用 framing，
用户仍需为标准协议的字段和校验细节加载后续 profile/codec。

## 验证

使用 immutable catalog/config 手工向量、Ruff/compileall、Qt offscreen 选择/应用/关闭和默认
onedir/onefile 启动门验证；真实 UART/TCP/UDP/BLE、Modbus/MAVLink/NMEA 设备交互和干净 Windows
仍待授权环境。
