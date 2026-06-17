# PRD-135 - Python/PyQt Migration Specs

> 用途：锁定 Python/PyQt 迁移决策边界。本 Specs 是文档落库入口，不是运行时代码实现入口。

## 1. 目标

- 新增 `docs/prd/PRD_135_Python_PyQt_Migration.md`。
- 新增 `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_Specs.md`。
- 明确 Python/PyQt 作为并行迁移主线，而不是立即替换 C++/Qt 默认入口。
- 明确 uv 管理方式：启动、测试、打包、验证都通过 `uv run ...` 命令收束。
- 明确 PyInstaller 只属于未来 Python 打包 lane，不能复用 PRD-099 的 C++/Qt 包装语义。
- 明确 PyQt6 GPLv3/商业授权门槛。
- 明确本轮三轴状态：工程 `E1设计中`，用户 `U0无功能变更`，设备 `D0未验证`。

## 2. 非目标

- 本轮不写 Python/PyQt 运行时代码。
- 本轮不修改 `pyproject.toml`。
- 本轮不新增 PyQt6、pyqtgraph、numpy、pytest-qt、PyInstaller 等依赖。
- 本轮不新增 PyInstaller `.spec`。
- 本轮不修改 C++ 源码、CMake、测试或 README。
- 本轮不修改 `EmbedDebug.bat`。
- 本轮不改变 `uv run start-embeddebug` 的 C++ 启动语义。
- 本轮不提升任何功能的 E/U/D 状态。

## 3. 约束

- 已加载 `CLAUDE.md`。
- 已加载 `docs/constraints/01-project-overview.md`。
- 已加载 `docs/constraints/02-workflow.md`。
- 已加载 `docs/constraints/03-architecture.md`。
- 已加载 `docs/constraints/07-directory-structure.md`。
- 已加载 `docs/serial_station_architecture.md`。
- 新功能代码必须先有 PRD。
- 构建目录只能是 `build/`。
- 用户已有改动不得回退。
- Serial Station 必须维持 `ui/controller/core/protocols/services/workers` 边界。
- Python runtime canonical path 需要后续更新 `07-directory-structure.md` 后才能创建。

## 4. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 主要文件 | `docs/prd/PRD_135_Python_PyQt_Migration.md` | 新增 |
| 主要文件 | `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_Specs.md` | 新增 |
| 只读参考 | `pyproject.toml` | 只读 |
| 只读参考 | `docs/prd/PRD_099_UV_Package_Tool.md` | 只读 |
| 只读参考 | `docs/prd/PRD_100_UV_Start_Tool.md` | 只读 |
| 只读参考 | `docs/prd/PRD_134_SerialStation_VOFA_Core_Parity.md` | 只读 |
| 只读参考 | `src/apps/serial_station/` | 只读 |
| 禁止修改 | `src/**` | 禁止 |
| 禁止修改 | `cmake/**` | 禁止 |
| 禁止修改 | `tests/**` | 禁止 |
| 禁止修改 | `README.md` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |
| 禁止修改 | `pyproject.toml` | 禁止 |

## 5. 命令边界

当前 C++ lane 保持：

```powershell
uv run start-embeddebug
uv run package-embeddebug
uv run verify-package-embeddebug
uv run test-embeddebug-tools
```

后续 Python lane 预留：

```powershell
uv run start-embeddebug-py
uv run test-embeddebug-py
uv run package-embeddebug-py
uv run verify-package-embeddebug-py
```

`uv run package-embeddebug-py` 未来可以封装 PyInstaller。不要直接把用户工作流设计成裸 `uv run pyinstall`，因为项目需要统一设置 workpath、distpath、license gate、artifact hygiene 和 smoke verification。

## 6. 后续实施批次

| 批次 | 目标 | 前置条件 |
|------|------|----------|
| B1 | 治理与 canonical paths | 本 PRD 合入，更新目录约束 |
| B2 | Python/PyQt 工具骨架 | PyQt license route 已记录 |
| B3 | 纯协议域迁移 | golden fixtures 策略已落库 |
| B4 | 服务迁移 | 日志/导出/回放/profile 对照 fixtures |
| B5 | 传输层 | fake transport 先于真实串口 |
| B6 | PyQt MVP | pytest-qt smoke gate |
| B7 | 用户工作流 parity | 侧-by-side parity matrix |
| B8 | 可视化/VOFA parity | 性能 harness 先于 UI 声明 |
| B9 | Python side-by-side package | PyInstaller onedir + verifier |
| B10 | 默认入口切换 | 单独 PRD、显式批准、C++ baseline 可回退 |

## 7. 验收标准

- [ ] `docs/prd/PRD_135_Python_PyQt_Migration.md` 存在。
- [ ] `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_Specs.md` 存在。
- [ ] PRD 明确 no immediate C++ cutover。
- [ ] PRD 明确 `uv run start-embeddebug-py`、`uv run package-embeddebug-py` 等 Python lane。
- [ ] PRD 明确 PyQt6 GPLv3/商业授权门槛。
- [ ] PRD 明确 PyInstaller `onedir` 优先，且 workpath 不得使用仓库 `build/`。
- [ ] PRD 明确本轮状态为 `E1/U0/D0`。
- [ ] 本轮没有修改源码、CMake、测试、README、`EmbedDebug.bat` 或 `pyproject.toml`。
- [ ] 没有新增第二构建目录。
- [ ] 用户已有改动没有被回退。

## 8. 失败条件

- 本轮新增 Python/PyQt 运行时代码。
- 本轮新增依赖或修改 `pyproject.toml`。
- 文档允许立即把 `uv run start-embeddebug` 改为 Python。
- 文档把 PyQt license gate 写成可选项。
- 文档允许 PyInstaller 默认使用仓库 `build/`。
- 文档声称 Python 迁移已经完成。
- 文档缺少 E/U/D 口径。

## 9. GO 配置

本轮 GO 只做文档存在性和差异检查：

```powershell
Test-Path docs/prd/PRD_135_Python_PyQt_Migration.md
Test-Path docs/superpowers/specs/PRD_135_Python_PyQt_Migration_Specs.md
rg "no immediate C\+\+ cutover|No immediate C\+\+ cutover|EmbedDebug.bat" docs/prd/PRD_135_Python_PyQt_Migration.md
rg "PyInstaller|license|E1|U0|D0|start-embeddebug-py" docs/prd/PRD_135_Python_PyQt_Migration.md
git diff --check -- docs/prd/PRD_135_Python_PyQt_Migration.md docs/superpowers/specs/PRD_135_Python_PyQt_Migration_Specs.md
```

本轮不要求构建或启动 smoke，因为没有修改源码、构建、启动入口、资源、依赖或路径执行逻辑。

## 10. 收口状态

| 轴 | 状态 |
|----|------|
| Engineering | E1 |
| User | U0 |
| Device | D0 |

后续若进入代码实现，必须重新按对应 PRD/Specs 执行构建、测试和启动验证。
