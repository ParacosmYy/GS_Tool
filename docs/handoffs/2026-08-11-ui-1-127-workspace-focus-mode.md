# UI-1.127 交接：工作区专注设置模式

日期：2026-08-11  
范围：为配置页提供可逆的专注视图和一次性高度过渡。

## 实现

`workspace.py` 在既有 route strip 创建 `workspaceFocusButton`，默认“专注设置”，点击后改为
“返回总览”。`workspace_focus_transition.py` 是唯一状态/动画 owner，只操作
`liveObservationBand`、`terminalSurface`、`sendControlBand` 和 `workspaceTabs` 的 presentation
高度/可见性；session、terminal ingestion、recording、pipeline、连接和设备状态不受影响。

模式不写入 QSettings。`QParallelAnimationGroup` 为一次性动画，不创建常驻 QTimer；主题切换、
reduced-motion、暂停、hide、minimize、close 会停止并静态套用当前模式，按钮文案和 accessible
description 始终同步。

## 验证

- 动态中间帧：下方 surface `94/144/96 → 59/91/60 → 7/10/7 → 0/0/0`，Tab 最大高度同步扩展。
- 反向恢复：三块 surface 恢复可见、最大高度恢复为 Qt 默认、Tab 最大高度恢复 350。
- reduced-motion：transition 为 `None`，直接进入静态目标；hide 清理 transition，show 保持模式。
- 三主题 × 980×680/1180×780 × 四 tab：exact-white=0、当前页 horizontal maximum=0、
  route/tab overlap=false。
- `scripts/check.ps1`、compileall、Ruff 通过；源码 158 个文件均不超过 1000 行。

架构师线程 `019fed62-45b0-7dc0-9a44-736aad31aff3` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、生命周期、焦点/无障碍、性能和简化审查。未修改嵌入式 C/C++；
`embedded-enterprise-workflow` 与 `embedded-code-review-simplifier` 对本轮源码不适用；public vendor
applicability=N/A；无目标硬件操作，硬件验证保持 `not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.127`；47,947,582 bytes；SHA-256
`85E4B0695D12DD7287E142E0E8E7E1B08159A66363E69DB82AF14AD21D75ED5A`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；根目录旧包 SHA-256 为
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。
