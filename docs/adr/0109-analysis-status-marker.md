# ADR-0109：共享分析状态 marker 与移动态 pulse

日期：2026-08-11  
状态：Accepted  
范围：SerialForge presentation / protocol-derived analysis surfaces

## 背景

协议、Component、Dataset、Curve 和 Replay 的 status label 已经通过底部五节点 rail 表达状态，
但在窄工作区中，用户需要更快识别 active、waiting、history 和 error 的入口。该视觉增强不能
把统计数字、解析进度、设备吞吐或回放时间轴复制到装饰层。

## 决策

- 继续由共享 `AnalysisStatusLabel` 负责绘制；五个 controller 和 `StatusSurfaceController`
  继续拥有文字、`state/source` projection 与业务事实。
- marker 只读取已有 property：source=history 时复用 purple，状态颜色沿用既有 `_state_color()`；
  active/waiting/draft 的低对比度 pulse 只消费 shared `(phase, animated)`。
- base stylesheet 仅增加左侧 padding/字重，避免 marker 与中文文案重叠；variant 主题继续通过
  semantic token 提供颜色，QSS 不增加业务判断。
- 原生 QLabel 绘制、AccessibleName/Description、底部 rail、NoFocus、鼠标透明、`stop()` 和
  reduced-motion/暂停/隐藏/最小化/关闭静态回退全部保持；不新增 timer、signal、线程、I/O 或
  OTA/AES/RTT/J-Link 依赖。

## 验证

静态门禁、compileall、Ruff、provenance verify、真实组合根 980/1180 横向 range、五类 status
最小宽度和三主题 offscreen vector 通过；生产字体为 `Microsoft YaHei UI`，截图已人工查看。

架构师线程 `019fed3c-101d-7873-bef8-bde944acc6b8` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt 绘制、可访问性、简化和生命周期审查。未修改嵌入式 C/C++；embedded
applicability=N/A。
