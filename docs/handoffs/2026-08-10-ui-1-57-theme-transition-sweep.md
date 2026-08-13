# SerialForge UI-1.57 交接

日期：2026-08-10

## 用户结果与范围

- 主题切换不再只有整体淡入：在既有 root fade 上增加一次性语义色几何 sweep，增强二次元主题切换的可感知反馈。
- sweep 是鼠标透明、不可聚焦、空 accessibility 的 presentation overlay，不改变 `apply_theme()`、焦点、键盘、无障碍树或业务状态。
- fade、sweep、effect 由同一个 `theme_transition.py` owner 管理；快速连续切换、低动效、暂停、隐藏、最小化和关闭均回到静态主题。

## 实际修改文件

- `src/serialforge/presentation/theme_transition_surface.py`：新增基于 ThemeSpec token 的几何 sweep overlay。
- `src/serialforge/presentation/theme_transition.py`：统一创建/停止/完成 root fade 与 sweep geometry animation。
- `src/serialforge/presentation/qt.py`：补充 `QRect` presentation import boundary。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0044-theme-transition-sweep.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

## 架构审查与父代理整合

```text
架构角色  019feafc-8cc9-7560-b70c-2474b935e82a  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：theme_transition.py 保持唯一一次性过渡 owner；ThemeTransitionSurface 只读 ThemeSpec 并由同一 stop/finish 清理
```

父代理整合：overlay 不接入 MotionController，不新增常驻 timer，不承载业务 progress；快速切换只保留最新一次 transition。

## 验证与限制

```text
scripts/check.ps1       PASS 136 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI157_TRANSITION         PASS overlay running; mouse-transparent; empty accessibility; finish cleanup
UI157_REDUCED_MOTION     PASS theme change falls back to no overlay
UI157_PIXEL              PASS three themes; 1180x780; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

## 打包收据

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.57`
- size：`47,839,961` bytes
- canonical/root SHA-256：`2654BE1BEC460BD05D18871D6CA570023A795CF30C2B9066A4D76E8A0AA6EF85`
- archive listing SHA-256：`70EA034A42EB126D8338A1E3D0E3AFEBB6FFF6EC284AE4FECE9E856EBCBB111D`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
