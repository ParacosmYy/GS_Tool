# ARCH-128 / UI-1.201 协议页与命令页局部响应式宽度

日期：2026-08-12  
范围：`controllers/protocol.py`、`controllers/command_workspace_builder.py`、`command_batch_empty_state.py`

## 本轮结果

协议页新增局部 `_ResponsiveProtocolRow`，并对配置上下文、组件 table/preview、Dataset preview 的既有实例施加收缩 contract；命令页新增
局部 action row 与 empty header owner。三者都只重排或调整既有 presentation widget，不持有业务状态、DTO、signals、timer、transport、
MotionController 或共享 scroll owner。

协议页的 component actions、Dataset header 与 DatasetCurveWidget advisory 经架构师复核后保持现状：没有真实外层 overflow 证据时不做预防性
重排或修改叶子可读性最小宽度。

## 证据

- 三主题 × 9 宽度 × 4 页 = 108 行：连接、协议、命令、扩展外层 `hmax=0`，且 content 不超过 viewport。
- 协议 table、component preview、Dataset preview、curve 同时可见追加 27 行：协议外层仍 `hmax=0`，content 不超过 viewport。
- 文件行数：protocol 891、command builder 321、empty state 474，均低于 1000 行。
- compileall 与 Ruff 在源代码编辑后已通过；最终 `scripts/check.ps1` 与 onefile 将在本轮打包前再次执行。

架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` `APPROVE`；独立代码审查
`019ff66c-28af-7561-9958-a4da1fea6c48` 最终 `APPROVE`（Critical/Required=0，保留两条 advisory）；简化评估
`019ff66c-290b-7cd0-9d06-736a955dd567` 无必须简化项。独立审查线程未重复执行完整矩阵，但纳入了当前 checkout 的独立可复核输出；
此限制将在最终交接中保留，不把它表述为真实 GUI/HIDPI/显示器 120Hz/硬件验收。

## 尚待交付

最终性能复核已完成：真实组合根在 `app.exec()` 内、show/rearm 完成且 timer active 时记录 `249 frames / 119.063Hz`，
`suspended=False`、`ambient=True`、`rearm_required=False`、`visible=True`；独立性能审查更新为 `APPROVE`，无 Critical/Required，
不建议改动 `lifecycle_motion.py`。这仍是 scheduler/frame cadence 证据，不等同真实显示器/compositor 120fps。

执行最终静态门禁与主题审计，以 `local-arch-128` 构建 onefile，并覆盖根目录两个 EXE；GUI/EXE startup、真实 HIDPI、显示器 120Hz 和硬件链路
仍未授权运行。

## 最终交付

- compileall、Ruff、`scripts/check.ps1`、source-limit 与 theme token audit：pass。
- canonical `dist/release/0.1.0/core/onefile/app/SerialForge.exe`、根目录 `SerialForge.exe`、
  `SerialForge-latest.exe` 三者均为 `48,079,010` bytes，SHA-256 均为
  `720131D96530341D0FFD9A271588A2BEDE1FF9829C9D3DE95D230E1DE5FE3688`。
- `PYINSTALLER_ARCHIVE.txt` SHA-256：`489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`；
  provenance verify：pass；revision：`local-arch-128`。
- engineering build，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
