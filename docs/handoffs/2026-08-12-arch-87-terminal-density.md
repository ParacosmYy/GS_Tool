# ARCH-87 / UI-1.160 终端 live band 密度收敛

日期：2026-08-12

## 实现

- 新增 `controllers/terminal_toolbar_builder.py`：实时观测控件使用控制行 + 活动行，暂停状态与
  数据活动获得独立伸缩空间。
- 新增 `controllers/send_bar_builder.py`：发送输入行保护格式/输入/发送按钮，状态与快捷动作单列，
  输入控件设置 220px 最小宽度并获得 stretch。
- `controllers/terminal.py` 收敛为错误、历史和批量命令组合；`bootstrap.py` 直接装配两个新 owner。

## 契约与审查

`TerminalControlBindings` 27 个字段、window refs、signals/callbacks、dynamic properties、
focus/accessibility、shared MotionController、close lifecycle 均保持；未新增 timer、线程、backend、
scroll owner 或业务状态。ARCH-87 架构师和独立 reviewer 服务调用均超时并关闭，未伪造 PASS；父代理
完成 fresh-pass，Required=0，简化评估保留了原 typed binding 与 single-owner wiring。

## 验证

```text
uv run ruff check src scripts                         pass
uv run python -m compileall -q src                   pass
scripts/check.ps1                                    pass
ARCH87_IMPORT_PASS terminal_toolbar_owner=terminal_toolbar_builder send_bar_owner=send_bar_builder
source line limit                                     pass (174 files <= 1000)
theme token audit                                     pass (3 themes, 22 semantic tokens, 19 selectors)
GUI/EXE startup                                       not-run
```

真实 980×720/1240×820 几何、三主题截图、显示器 FPS、硬件/HIL 和正式发行签名未运行；待明确授权后
再补充，不把静态宽度预算当成运行时通过证据。嵌入式 C/C++ applicability 为 N/A，无 firmware 或
target hardware 修改。

## 交付

`local-arch-87` onefile 已完成并覆盖 canonical artifact、根目录两个 EXE：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 48,002,962 bytes
sha256: 860643732A32225DAA19139459DA4DE1DF9027340F9EA313266FD7696DAEF79B
archive listing sha256: 5E1BAFBD5AC2AAEF5D78AD61BEB3551D8710F34B39AF662B889964FB8BA107C1
provenance: pass; source revision local-arch-87
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
root EXE startup: not-run (current checkout policy)
```
