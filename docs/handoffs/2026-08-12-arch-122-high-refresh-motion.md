# ARCH-122 / UI-1.195 交接记录

日期：2026-08-12

## 交付范围

- `presentation/widgets.py`：唯一 `MotionController` 使用 4ms `PreciseTimer`；内部 tick/deadline/activity 全部使用 `time.perf_counter()`；
  frame budget 只保留小数余量，禁止事件循环恢复后的过期补帧 burst。
- `presentation/embedded_station_overview.py`：`_ResponsiveMetricGrid` 只重排现有 metric tuple，按 6/3/2/1 列响应式变化；value label
  可收缩换行，完整动作值保留在 `accessibleDescription`。
- 未改变 application/domain/infrastructure、DTO、bindings/signals、scroll owner、theme contract、OTA/AES/RTT/J-Link backend。

## 架构与审查

架构师 `019ff579-3e62-7551-9cf5-4907a3638a9a`：APPROVE，要求 perf_counter 统一时间域并保留4ms单时钟。  
独立 reviewer `019ff57c-bd7a-79b3-838c-6471f5a8d87c`：APPROVE，无 Critical/Required findings；真实显示器/HIDPI/paint CPU 仍是可选后续风险。  
简化 reviewer `019ff57c-bdd6-7f30-965b-cb05faefd0b1`：无需扩大简化；仅指出未读取 `_rearmed_after_show` 可删，本轮保留以避免扩大生命周期变更。  
embedded C/C++ public-vendor-source applicability：N/A；不作认证合规声明。

## 验证

- `uv run --locked --extra dev python -m compileall -q src`：pass。
- `uv run --locked --extra dev ruff check src`：pass。
- `scripts/check.ps1`：pass，182 files <=1000 lines，3 themes/22 tokens/19 selectors，legacy_qss_literals=0。
- 10.2s offscreen scheduler：1224 frames，119.992Hz，平均8.334ms，p95 12.097ms。
- 50ms elapsed 注入：single `_tick` emit=1，budget=0.259，`0 <= budget < 1`；无追赶 burst。
- activity 30ms deadline、pause/suspend/close：pass。
- 合法52字符动作文本在640/800/980px：grid不超过parent，value可收缩，全文 accessibility description：pass。
- QFontDatabase fonts-directory warning 仍是 offscreen 环境提示；不代表主题白块或布局失败。
- 未运行真实 Windows GUI/HIDPI、显示器合成 FPS、EXE startup、硬件连接、OTA/RTT 实连、签名验收。

## 交付指纹

`local-arch-122` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。canonical、root、root-latest 均为
`48,044,048` bytes，SHA-256 为 `CF85FAFB7F757FA09853670D051077A31FABECF52A91A8CF205A8145C2DF0B6D`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名为 `NotSigned`，
`release_eligible=false`，`hardware_acceptance=not_run`。
