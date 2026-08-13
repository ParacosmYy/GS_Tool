# ARCH-77 / UI-1.150：动效 cadence 与协议页密度

日期：2026-08-12  
范围：共享 presentation 动效时钟、协议/组件/Dataset/Curve/Replay 控件布局、根目录 onefile 交付。

## 结果

- `presentation/widgets.py:MotionController` 继续是唯一动效时钟、唯一 `PreciseTimer` 和唯一
  `frame_changed` source；改为非 single-shot 的 8ms 周期 scheduler slot，phase 仍按
  `time.monotonic()` elapsed 推进。8ms 是约 120Hz 的调度目标，不是显示器精确 120fps 承诺。
- `presentation/form_fields.py` 只提供无状态的 labeled-field/peer-row 组合 helper。
- `presentation/controllers/protocol.py` 将协议、组件、Dataset、曲线和回放控件整理为语义 field/action
  rows，保留 `ProtocolPanelWidgets`、回调、typed `itemData()`、显式 Tab 顺序、accessibility 文案、
  四个 surface owner 和外层 `QScrollArea`。
- 不涉及 OTA/debug/transport/device handle、application/domain 状态或嵌套滚动容器。

## 架构与独立审查

- 架构师：Luna/max，只读 conditional approve；固定 8ms 需诚实表述为约 120Hz scheduler target，
  不升级 Terra。
- 独立审查：Luna/max，Required=1（修正文案，已完成）；Optional=2（buddy 关联、tuple 小简化，均不
  构成阻断）；FYI=3。结论为代码行为 PASS，无生命周期或协议布局阻断。
- 简化评估：绝对 deadline 试验在当前离屏事件循环中出现约 17–18ms 跳帧，改为一个周期性
  `PreciseTimer` 更平滑；没有增加 timer、线程、事件总线或 renderer 状态。

## 验证

- `uv run ruff check src`：pass。
- `uv run python -m compileall -q src`：pass。
- `scripts/check.ps1`：source line limit 167 files、theme token audit 3 themes / 22 semantic tokens / 19 selectors / 0 legacy literals：pass。
- `ARCH77_LAYOUT_PASS`：980×720、1240×820 × 三主题；协议页 horizontal maximum=0；9 个关键控件可见。
- `ARCH77_MOTION_PASS`：离屏 1.25s 采集 156 frames，平均 7.996ms，范围 4.668–11.670ms；该数字只证明
  8ms scheduler target，在当前 offscreen 环境可运行，不代表显示器 FPS。
- `ARCH77_LIFECYCLE_PASS`：hide/show 后 shared timer 恢复且 onboarding focus 保持；reduced-motion、pause、
  minimize/close 的静态 fence 由既有 lifecycle owner 负责，未新增清理路径。
- QFontDatabase fonts directory warning 仍是离屏环境既有警告，不影响使用 `configure_application_font()`
  的截图文字；未进行硬件 flash/deploy/target 操作。

## 嵌入式 R&D assurance

- 本轮没有修改嵌入式 C/C++、固件、MCU/BSP/HAL/RTOS/ISR/DMA/driver/OTA backend/Flash/NVM 或硬件接口；
  public first-party vendor source applicability：N/A。
- independent review、behavior-preserving simplification assessment 和 authorized non-destructive
  validation 已记录；未声明 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。

## 包交付

本轮已使用 `local-arch-77` 重新构建 onefile，并覆盖：

- `SerialForge.exe`
- `SerialForge-latest.exe`

签名保持 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`；最终大小、SHA-256、
archive listing SHA-256 和 provenance 以打包命令输出为准。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,984,250 bytes
SHA256: 11E505D955E750AD5062D5A1B552E9D92AE2DF9A8D3751DEAE0612040E9140D9
archive listing SHA-256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; source revision local-arch-77
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```
