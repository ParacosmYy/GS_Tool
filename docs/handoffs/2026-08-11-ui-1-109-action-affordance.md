# UI-1.109 关键动作与 UART 超时 affordance

日期：2026-08-11  
范围：`presentation/controllers/connection_builder.py`、`presentation/controllers/terminal.py` 的动作提示、无障碍描述与 UART 超时 affordance。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- 清除错误、清空终端预览、清除发送历史、快捷命令菜单、保存快捷命令、保存自定义连接和删除自定义连接均补齐 tooltip 与 accessible description。
- UART 读超时、写超时补齐“秒”单位、连接后生效和等待范围说明；未改变既有 `QDoubleSpinBox` 范围、默认值、runtime callback 或配置 DTO。
- 不新增状态源、signal、timer、线程、依赖、设备 I/O、公开 API 或跨层 helper；连接、发送、历史、原始记录、OTA/debug contract-only/attach-only 语义不变。

## 角色与审查证据

UI-1.109 前置六角色第一轮：`019fecc5-2a23-79f3-a11a-0b477889260e`、`019fecc5-2a76-7a11-bcb3-2f70b1baed49`、`019fecc5-2ac5-7b63-9d2e-e3172773d9fb`、`019fecc5-2b18-79b0-9e89-504cc5f1eefe`、`019fecc5-2b6f-7d92-bfed-738973ce10a9`、`019fecc5-2bb9-7413-97a8-73edecb37464`。  
第二轮边界复核：`019fecc6-a1c4-7571-9b12-10a35363fc4c`、`019fecc6-a210-7c43-8236-fe6e651ef28a`、`019fecc6-a266-7b83-ad4a-5b7585d9ca40`、`019fecc6-a2c0-7e70-8bcb-96394f645906`、`019fecc6-a308-7d40-a28a-b026c0e25811`、`019fecc6-a353-7311-a1a0-82094e8bcea1`。  
首轮源实现后的独立复核：`019fecc5-fb55-7b01-9996-a46edf3d3494`；最终补丁后的独立复核：`019fecc8-4cb4-78b3-bb1e-ac7dec09baf4`。上述代理均在限定等待窗口内超时并关闭，超时不视为通过；父代理完成五轴审查、架构边界、复用/简化评估和验证结果整合。

首轮验证故意暴露了保存/删除自定义连接仍缺少 accessible description，随后只在其 owner 内补齐两条文案；未扩大文件或业务改动范围。

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 selectors; legacy_qss_literals=0)
uv run --locked --extra dev python -m compileall -q src scripts  PASS
uv run --locked --extra dev ruff check src scripts              PASS
UI109_ACTION_AFFORDANCE_VECTOR_PASS                   actions=9 combos=27 editable=('UART 端口',)
                                                       themes=3 responsive_sizes=2 near_white=0 close_lifecycle=pass
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

Qt offscreen 组合根脚本没有调用 `.show()`；`try/finally` 中执行 window close、线程池等待和 application service shutdown。仅剩 PySide6 环境字体目录提示，不影响像素/生命周期断言。未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收。

本轮 Python/Qt presentation 代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。embedded enterprise workflow 的 applicability、independent review、behavior-preserving simplification assessment 与 authorized non-destructive verification 已记录；未修改嵌入式 C/C++。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.109
size: 47,931,714 bytes
SHA-256: 971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
