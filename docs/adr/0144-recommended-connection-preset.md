# ADR-0144：首屏推荐连接 preset 与 stale context 清理

- 状态：Accepted for ARCH-93 / UI-1.166
- 日期：2026-08-12
- 范围：连接 preset catalog/controller、window bootstrap、transport presentation runtime

## 背景

连接页已有安全的内置 preset 和完整 UART 选择项，但首次打开 combo 停在 placeholder，用户仍要
主动选择一组常用时序；同时手动切换 transport 后旧 preset 摘要可能继续显示，增加认知负担。

## 决策

1. 用 `DEFAULT_CONNECTION_PRESET_KEY = "uart-115200-8n1"` 表示安全推荐 profile。
2. 窗口组合完成后由 `hydrate_recommended_connection_preset()` 一次性选中该 entry，复用既有
   combo signal/apply path；不直接写 business state，不自动连接，不持久化 selection。
3. transport owner 在每次 transport projection 时检查当前 preset 的 transport；不匹配就回到
   placeholder/context empty。用户重新选择任意内置/自定义 profile 后，现有 apply path 恢复完整
   context 与字段投影。

## 不变量

- UART 端口仍必须由用户选择或输入，连接仍必须显式点击；session 不能因 hydration 变为 active。
- apply 的 transport signal blocking、network/BLE/RTT gate、save/delete、自定义 catalog、tooltip、
  accessibility 和低动效不变。
- 不新增 ViewModel 状态、timer、线程、设备扫描、密钥、scroll owner 或跨层依赖。

## 验证

- `ARCH93_PRESET_CONTRACT_PASS`：启动 profile、TCP stale clear、TCP apply、clear、横向范围和
  session gate 通过。
- `ARCH93_ALL_PRESETS_PASS`：7 个 builtin profile 全部应用，4 tabs visible scroll 无横向溢出。
- Ruff、compileall、`scripts/check.ps1` 通过；源文件均低于 1000 行。
- 未启动 EXE、未连接硬件、未做真实显示器/HIDPI/FPS 或签名验收。
