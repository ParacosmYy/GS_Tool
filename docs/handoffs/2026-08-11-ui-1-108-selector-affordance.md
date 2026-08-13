# UI-1.108 用户选择器 affordance 契约

日期：2026-08-11  
范围：`connection_builder.py`、`controllers/terminal.py`、`command_batch_editor.py` 的用户选择器提示与无障碍语义。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- transport、终端显示模式、发送格式、发送历史、批量命令、批量编辑器快捷命令和当前步骤格式均显式声明不可编辑。
- 所有用户可见 `QComboBox` 都补齐 accessible name、tooltip、accessible description；说明选择只改变展示、加载或编辑状态，不会自动连接、发送或执行。
- UART 端口保留唯一可编辑 combo：支持枚举端口或输入 `COMx`，同时补齐“不会自动连接”的 accessibility 文案。
- 所有 itemData、枚举值、currentIndexChanged/editTextChanged signal、连接 gate、批量执行语义、主题 QSS、键盘顺序和 OTA/debug contract-only/attach-only 边界均保持不变。
- 未新增状态源、DTO、timer、依赖、资源、事件总线或跨层 helper；每个 owner 继续在自身 builder/editor 内声明控件契约。

## 角色与审查证据

两轮六角色前置评审和最终独立复核均在限定等待窗口内超时并关闭；超时不视为通过。父代理完成五轴检查、架构边界检查、复用/简化评估和结果整合。

前置角色第一轮：`019fecbe-9fa4-7292-97e2-12a1b49d26e5`、`019fecbe-9ff4-7ef1-9ef0-195ddf84dba7`、`019fecbe-a042-7e73-b1bd-fff28e6bb26e`、`019fecbe-a09f-7703-a4eb-5da66d3fdac8`、`019fecbe-a0ec-71d2-9a54-06f842337ce6`、`019fecbe-a13a-7a80-85c5-5b0469b82048`。  
追加边界评审：`019fecc0-596a-7260-9771-a8d1ae0bfb0b`、`019fecc0-59c3-79e1-9899-5e34d9ae9fd0`、`019fecc0-5a15-77c2-84b3-f0a00ff14bd7`、`019fecc0-5a62-72a2-8870-d21d73cba961`、`019fecc0-5abf-7492-ba27-0ffa516d79f8`、`019fecc0-5b15-7cb3-b150-1701ae9c47f6`。  
最终独立复核：`019fecbe-d5d7-7920-971e-aab68db782fb`。

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI108_SELECTOR_AFFORDANCE_VECTOR_PASS                   combos=27 editable=('UART 端口',) hints=all themes=3 responsive_sizes=2 near_white=0 close_lifecycle=pass
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

首轮组合根审计先定位到 transport/终端/历史/批量 selector 缺少提示，补齐后又定位到 UART 端口缺少 accessible description；产品代码按最小范围补齐。最终验证脚本使用 `try/finally` 先执行 `window.close()`、取消 discovery、等待线程池，再关闭 application service，避免验证脚本断言提前退出造成异步 signal 清理噪声。仅剩 PySide6 环境字体目录提示，不影响渲染断言。未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收。

本轮 Python/Qt presentation 代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。embedded enterprise workflow 的 applicability、independent review、simplification assessment 与 authorized non-destructive verification 已记录。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.108
size: 47,930,586 bytes
SHA-256: 27EBF48272AACF3E5CCC3586ABFC8988272A737E80A6D24D57FD001A8B9E6FA4
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

