# UI-1.107 连接状态 rail 彗尾与中心光点

日期：2026-08-11  
范围：`src/serialforge/presentation/connection_status_surface.py` 的 presentation-only 自绘增强。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果

- 在既有连接状态 rail 的运动光点后增加三枚渐隐彗尾，并增加外环与中心光点，增强二次元状态反馈的连续性。
- 只复用 `ConnectionStatusRail` 已有的 `_phase`、`_animated`、`_state`、节点位置和 `ThemeSpec` 状态色；没有新增 timer、状态源、业务字段、公开 API、资源、线程或 I/O。
- `discovered`、`closed`、`error` 保持静态；`opening`、`open`、`closing` 仅在既有共享帧允许时动态绘制；停止帧保持稳定。
- `NoFocus`、鼠标透明、空 accessibility、生命周期 stop/reduced-motion 策略、连接事实和 OTA/debug contract-only/attach-only 边界均保持不变。

## 角色与审查证据

前置只读角色均按项目约束使用 Luna/max/Fast，并在限定等待窗口后超时关闭；独立复核同样超时，超时不视为通过。父代理完成五轴检查、架构边界检查、复用/简化评估与结果整合。

| 角色 | run id | 结果 |
| --- | --- | --- |
| 产品 | `019fecb5-c63e-7d80-a046-e98f03ad403b` | timeout，closed |
| 架构 | `019fecb5-c692-7db0-bee4-9657fa0a013a` | timeout，closed |
| UI | `019fecb5-c6e3-73b2-9ece-4b74a7492a0f` | timeout，closed |
| 开发 | `019fecb5-c736-71c2-912e-b74cb64ea16f` | timeout，closed |
| 验证 | `019fecb5-c77f-7fb1-bd35-133b24e91c1e` | timeout，closed |
| 打包 | `019fecb5-c7cc-73f1-af1d-95d3d51c0e38` | timeout，closed |
| 独立复核 | `019fecb6-6121-7073-80ac-e82d0d9ed8ee` | timeout，closed |

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI107_CONNECTION_RAIL_VECTOR_PASS                       themes=3 states=6 animated_states=3 comet_trail=checked stop=checked near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

验证脚本使用真实组合根和固定离屏画布，没有调用 `.show()`，没有创建测试文件；首轮因使用布局重排后的动态尺寸导致向量脚本自身坐标断言失效，未改动产品代码，随后修正为固定画布尺寸并通过。未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收。

本轮 Python/Qt presentation 代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。embedded enterprise workflow 的 applicability、independent review、simplification assessment 与 authorized non-destructive verification 已记录。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.107
size: 47,929,693 bytes
SHA-256: 6785E9F51A9E907A647E2B84A050EE54F93B1925C91B3581DDEEABFC3292434D
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

