# 2026-08-12 · ARCH-85 / UI-1.158 派生空态表面收敛

## 目标

修复协议/组件/Dataset 页面空数据时“空态卡 + 空白预览”重复占位造成的视觉松散与滚动距离过长，
同时保持企业级 owner 边界、用户可理解的等待提示和既有 120Hz scheduler 口径。

## 实现

- `src/serialforge/presentation/controllers/protocol.py`
  - 组件帧预览和 Dataset 预览仍由同一个 protocol panel 组合；初始化时 hidden。
  - 没有增加滚动容器、业务状态、timer 或跨层依赖。
- `src/serialforge/presentation/controllers/derived_data.py`
  - 无协议帧/来源不适用时隐藏组件预览，保留 `ComponentEmptyStateSurface` 的语义 guidance。
  - 无 Dataset sample/来源不适用时隐藏 Dataset 预览；样本出现后恢复可见。
  - 既有摘要上限、placeholder、AccessibleDescription、tooltip、scroll follow-latest 和数据路径保持。

## 架构与简化复核

本轮为 Python/PySide6 presentation-only，不涉及 embedded C/C++、MCU、BSP/HAL、RTOS、driver、
OTA/AES 实现或目标硬件；public first-party vendor source applicability 为 N/A，不声称 MISRA、
ISO 26262、ASIL、ASPICE 或认证合规。用户要求的架构师角色已调用 Luna/max，但本轮服务窗口超时后
关闭，未伪造独立 PASS；父代理完成 owner、correctness、readability/simplicity、security、
performance fresh-pass。代码简化结论是删除重复空白呈现，不改变业务行为。

## 非破坏性验证

```text
uv run ruff check src/serialforge/presentation/controllers/protocol.py src/serialforge/presentation/controllers/derived_data.py  pass
uv run python -m compileall -q ...  pass
uv run python scripts/check_source_limits.py  pass
ARCH85_EMPTY font=Microsoft YaHei UI component_preview_visible=False component_empty_visible=True dataset_preview_visible=False horizontal_max=0 vertical_max=778
ARCH85_DATA component_preview_visible=True
ARCH85_MOTION120 frames=124/1s target=120 interval_ms=8
ARCH85_CLOSE visible=False motion_timer=False
hardware=not-run
```

视觉证据：`C:\Users\Gs\AppData\Local\Temp\serialforge_arch85_protocol_empty_980.png`，已检查组件空态
与 Dataset 标题不再被空白预览重复撑开；offscreen 环境仍可能输出既有 Qt font-directory warning，
Windows 系统字体已由 `font_runtime` 选择 `Microsoft YaHei UI`。

## 包交付

onefile、canonical/root/root-latest 覆盖、provenance、启动/关闭烟测已完成：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 47,997,051 bytes
sha256: 6794C51C907D5A2FFA32E0A1F9B841CB920A9F970D6B5C4CDAAB38405B6B51F5
archive listing sha256: 9991DD041A86C1828261B9D8731E17A04BA20F90739193F11F6DE12423455D78
source revision: local-arch-85
signature: NotSigned
release_eligible: false
hardware_acceptance: not_run
ARCH85_EXE_STARTUP_PASS process_count=2
ARCH85_EXE_SHUTDOWN_PASS remaining=0
```

根目录交付：[SerialForge.exe](../../SerialForge.exe) 与
[SerialForge-latest.exe](../../SerialForge-latest.exe)。正式发行资格和硬件验收仍保持
`release_eligible=false` / `hardware_acceptance=not_run`，因为本地包未签名且未操作目标硬件。
