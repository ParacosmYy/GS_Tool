# SerialForge UI-1.68 交接：发送快捷键提示

日期：2026-08-10  
项目：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；本轮唯一写入者  
范围：在发送控制带增加主题化 `Ctrl+Enter 发送` 静态提示，保持原生发送/快捷键/焦点边界。

## 交付结果

- `terminal.py` 装配 `QLabel#sendShortcutHint`，提供一致的 AccessibleName/Description 和 tooltip。
- 控件 `NoFocus`、无动作、不进入 Tab 顺序，不写入 `window._*` facade；既有 `Ctrl+Enter` shortcut、send action、send gate、
  ViewModel 和状态 owner 未改变。
- `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 为默认和三套主题提供 keycap 的文字、背景、边框、圆角语义色，
  禁止系统 palette 产生白色 fallback。
- 宽度约束为 176–184 px。第一次向量发现 132 px 会裁切约 156 px 字体文案，已在架构边界内修正，并将 CSS 内边距纳入验证。

## 角色与质量门

```text
六角色（产品/架构/UI/开发/验证/打包）均在源码修改前调用，等待超时后关闭：
019feb72-b0c9-7181-adf5-2e0a97253fc5
019feb72-b113-7ef0-b9c2-517dda3ad7d6
019feb72-b169-7a32-a17a-309ccf64978b
019feb72-b1bd-7c33-a7b9-b460016732b6
019feb72-b212-77a3-9aa7-4d5236c2a724
019feb72-b259-7920-9d2b-15113acd882d
架构修正复核：019feb75-6193-7ec0-baa5-017658ef7ed0，宽度/样式修正前调用，等待超时后关闭
独立质量复核：019feb77-5dca-7b51-9a9b-9db1e5fb3113，完成后调用，等待超时后关闭
```

父代理五轴审查：GO。正确性、可读性/简化、架构、无障碍/安全、性能均无阻断项；独立角色未返回意见，未被计为通过。
简化评估：复用既有 send bar 空列、Qt QLabel、base/variant QSS 和既有 action owner，不新增 facade、状态模型、timer、事件
过滤器、依赖或第二套快捷键路径。

## 验证

```text
check.ps1                         PASS 147 files <=1000; 3 themes; 22 tokens; 19 selectors
compileall -q src                 PASS
UI168_SEND_HINT_LAYOUT_PASS       PASS 760/952/1152; text area >= font metrics; NoFocus; role=subtle
UI168_THEME_OVERRIDE_PASS         PASS star_trail/moonlit_ocean/sakura_night
provenance.py verify              PASS
root/canonical hash               PASS
```

未启动完整 GUI、SerialForge.exe 或后台服务；未执行 HIDPI/读屏/真实视觉/真实串口、网络、BLE、RTT/J-Link、OTA、硬件、签名和
正式发行验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。Qt offscreen 仅做短时内存布局向量，
环境缺失 `.venv` 字体目录的警告不代表 Windows 字体结论。嵌入式 C/C++ 适用性：N/A。

## 最新 EXE

- 根目录：[SerialForge.exe](../../SerialForge.exe)
- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- source revision：`local-ui-1.68`
- size：`47,885,847` bytes
- SHA-256：`A69CE9AD35442595573E2063D9831D57EE9CC7DCE7287D1C8A2D6F0860DBF12E`
- archive listing SHA-256：`C90B0C21DBC68E1B814A2DE8560D675CDFC90B5881A06AC1C1BDFC727338C600`
- `signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
