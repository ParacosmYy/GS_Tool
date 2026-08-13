# ADR 0046：协议配置摘要与应用状态 surface

日期：2026-08-10  
状态：accepted  
范围：`presentation` 协议配置面板

## 背景

协议页同时展示配置编辑器、解析统计、流水线和派生数据。用户修改 framing 或最大帧长后，原有状态文字
容易被控件密度和长文案淹没，难以判断当前值是草稿还是已经应用。

## 决策

- 在 `protocolConfigSurface` 内增加独立的 `ProtocolConfigContextSurface`，只展示当前 framing、checksum、
  最大帧长与“草稿/已应用”阶段。
- `controllers/protocol_context.py` 是唯一 projection owner：它读取原生控件的可见文本和值，并复用
  `protocolStatus.property("state")`；不重新计算传输能力、历史来源或 parser gate。
- `ProtocolConfigContextProjection` 是 immutable DTO；surface 不持有 ViewModel、parser、配置提交动作或常驻
  timer，装饰 rail 只接收 lifecycle 的共享 `MotionController` frame/stop。
- 组件宽度限制在 190–520 px，默认 QSS 和三个 theme override 显式覆盖所有语义状态，避免窄窗口横向溢出和
  系统 palette 白色回退。

## 被拒绝的替代

- 将摘要文本拼接进 `protocolStatus`：会混合统计状态和编辑上下文，破坏既有 status surface 契约。
- 建立通用 `ConfigSummarySurface`：当前只有协议配置一个稳定责任边界，通用抽象会隐藏状态 owner，增加跨模块
  耦合，暂不引入。

## 后果与验证

协议页增加一条 32 px 以内的可读上下文带，但不改变编辑、应用、重置、解析或派生行为。验证包括源码行数、
Ruff/compileall、三主题 1180×780 离屏截图无近白像素、projection 多状态、共享 frame/stop 和可访问描述。
真实 Windows 字体、持续 GUI、读屏、硬件、正式发行与签名验收仍不在本 ADR 范围内。
