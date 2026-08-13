# ARCH-88 / UI-1.161 命令管理页密度收敛

日期：2026-08-12

## 实现

- 新增 `controllers/command_workspace_builder.py`，成为命令页唯一组合 owner。
- 批量命令下拉选择独占 selector row；新建、编辑、删除、执行、停止进入独立 actions row，保留
  trailing stretch。
- `terminal.py` 仅保留错误通知；`workspace.py` 接入新 builder，外层 `commandPage` scroll 不变。

## 契约

`CommandBatchControlBindings` 9 个字段、window refs、currentIndexChanged、五个 action callbacks、
空态 `new_requested`、status/results projection、Tab order、shared motion 和 close lifecycle 保持。
命令选择不会自动执行，停止动作和连接 enable gate 不变；未新增 timer、线程、backend 或业务状态。

## 验证

```text
uv run ruff check src scripts                         pass
uv run python -m compileall -q src                   pass
scripts/check.ps1                                    pass
ARCH88_IMPORT_PASS command_workspace_owner=command_workspace_builder binding_fields=9
source line limit                                     pass (175 files <= 1000)
theme token audit                                     pass (3 themes, 22 semantic tokens, 19 selectors)
GUI/EXE startup                                       not-run
```

ARCH-88 架构师服务调用超时并关闭，未伪造独立 PASS；父代理完成 architecture、correctness、
readability/simplicity、security、performance fresh-pass，Required=0。真实几何、键盘焦点、三主题、
显示器 FPS、硬件/HIL 与签名验收待授权。嵌入式 C/C++ applicability 为 N/A。

## 交付

`local-arch-88` onefile 已完成并覆盖 canonical artifact、根目录两个 EXE：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 48,002,961 bytes
sha256: EC5E4DE1DDDB4537FF3CA00B14542D71736C0DDBA2AA27C6275DE9369098C609
archive listing sha256: A152062F579FF9CCA9D2421EFDC6FA5D2FA6918A0691BE46EEE359AE451A47FF
provenance: pass; source revision local-arch-88
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
root EXE startup: not-run (current checkout policy)
```
