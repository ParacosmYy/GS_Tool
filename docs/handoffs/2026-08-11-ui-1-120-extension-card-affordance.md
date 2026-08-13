# UI-1.120 扩展工具站能力卡 affordance

日期：2026-08-11  
父代理：Codex；本地 checkout 唯一写入者  
范围：presentation-only 扩展能力卡视觉层级与响应式布局。

## 变更

- `embedded_extension_panel.py` 的三组能力卡 grid 增加两列等权 stretch。
- `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 对普通、contract-only、attach-only 卡片
  增加 hover border selector，并沿用既有 `ThemeSpec` 语义 token。
- 卡片仍为静态 `QFrame`、NoFocus、无 clicked signal；tooltip 与 AccessibleName/Description 共用
  同一边界说明；只读 DTO、
  OTA contract-only 和 RTT/J-Link attach-only 边界不变。
- 工作区已有一次性 180ms page fade 继续负责页面过渡，没有新增 timer 或动效时钟。

## 验证

- `scripts/check.ps1`：pass；source line limit `157 files <= 1000`；theme token audit pass。
- compileall：pass；Ruff：pass。
- 真实组合根使用 `configure_application_font()`，验证三主题、980/1180、四 Tab 切换、7 张卡片、
  vertical range `728`、horizontal range `0`、NoFocus 和 3 条 hover selector；`UI120_EXTENSION_CARD_VECTOR_PASS`。
- 1180×780 默认主题、980×680 樱雾夜航、底部卡片和 hover synthetic render 已人工查看；中文可读，
  未见白色背景带或横向溢出。
- provenance verify：pass；canonical onefile 见下方。

## 架构与 assurance

架构师线程 `019fed29-379a-70c2-8551-eae81e2cbfe9` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token 对称、静态语义、焦点/无障碍和简化审查。未修改嵌入式 C/C++、固件、BSP/HAL、
RTOS、驱动、协议实现、OTA 或 RTT/J-Link 后端；embedded applicability=N/A。未运行真实硬件、
GUI EXE 正式启动、HIDPI、读屏和真实链路验收。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.120`；47,940,170 bytes；SHA-256
`A5C9FFB974C712D65B7212A252F860972924AF3817455BC5B8432FD8AE8E0C46`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。
