# PRD-135 B1 - Python/PyQt Governance Specs

> 用途：把 PRD-135 的 Python/PyQt 并行迁移主线写入目录、架构和 Serial Station 专项约束。B1 仍是治理批次，不写运行时代码。

## 1. 目标

- 更新 `docs/constraints/07-directory-structure.md`，批准 Python/PyQt canonical 路径。
- 更新 `docs/constraints/03-architecture.md`，记录 Python/PyQt 并行主线的架构边界。
- 更新 `docs/serial_station_architecture.md`，把 Python/PyQt Serial Station lane 纳入专项闭环。
- 保持 C++ baseline：`EmbedDebug.bat -> build/EmbedDebug.exe`。
- 保持 uv 管理方向：Python 启动、测试、打包、验证使用独立 `uv run ...` 命令。
- 本轮三轴状态：工程 `E1设计中`，用户 `U0无功能变更`，设备 `D0未验证`。

## 2. 非目标

- 不新增 Python/PyQt 运行时代码。
- 不新增 `python/embeddebug/` 目录。
- 不新增 `tests/python/` 目录。
- 不修改 `pyproject.toml`。
- 不新增 PyQt6、pyqtgraph、numpy、pytest-qt、PyInstaller 依赖。
- 不修改 C++ 源码、CMake、README 或 `EmbedDebug.bat`。
- 不切换默认启动入口。
- 不提升 Python migration 到 `E2`、`U1` 或 `D1`。

## 3. 约束加载

- 已加载 `CLAUDE.md`。
- 已加载 `docs/constraints/01-project-overview.md`。
- 已加载 `docs/constraints/03-architecture.md`。
- 已加载 `docs/constraints/07-directory-structure.md`。
- 已加载 `docs/serial_station_architecture.md`。
- 已加载 `docs/prd/PRD_135_Python_PyQt_Migration.md`。

## 4. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| 主要文件 | `docs/constraints/07-directory-structure.md` | 更新 |
| 主要文件 | `docs/constraints/03-architecture.md` | 更新 |
| 主要文件 | `docs/serial_station_architecture.md` | 更新 |
| 主要文件 | `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B1_Governance_Specs.md` | 新增 |
| 只读参考 | `docs/prd/PRD_135_Python_PyQt_Migration.md` | 只读 |
| 禁止修改 | `src/**` | 禁止 |
| 禁止修改 | `cmake/**` | 禁止 |
| 禁止修改 | `tests/**` | 禁止 |
| 禁止修改 | `pyproject.toml` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 5. Canonical Paths

Approved for future PRD-135 implementation batches:

- `python/embeddebug/`
- `python/embeddebug/serial_station/`
- `tests/python/`
- `tests/fixtures/serial_station/`
- `packaging/pyinstaller/`

Still prohibited:

- Python production runtime under `tools/`.
- Python production runtime under `src/`.
- CMake scanning or registering `python/`.
- PyInstaller workpath using repository `build/`.
- Reusing `uv run start-embeddebug` as the Python default entry before cutover.

## 6. Reserved Commands

```powershell
uv run start-embeddebug-py
uv run test-embeddebug-py
uv run package-embeddebug-py
uv run verify-package-embeddebug-py
```

Existing commands keep C++ semantics:

```powershell
uv run start-embeddebug
uv run package-embeddebug
uv run verify-package-embeddebug
uv run test-embeddebug-tools
```

## 7. 验收标准

- [ ] `07-directory-structure.md` 包含 `python/embeddebug/`、`tests/python/`、`tests/fixtures/serial_station/`、`packaging/pyinstaller/`。
- [ ] `03-architecture.md` 包含 Python/PyQt 并行主线边界。
- [ ] `serial_station_architecture.md` 包含 Python/PyQt lane 和 uv/PyInstaller 规则。
- [ ] B1 Specs 文件存在。
- [ ] 没有修改源码、CMake、测试、`pyproject.toml`、README 或 `EmbedDebug.bat`。
- [ ] 没有新增第二构建目录。
- [ ] 用户已有改动没有被回退。

## 8. GO 配置

```powershell
Test-Path docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B1_Governance_Specs.md
rg "python/embeddebug|tests/python|packaging/pyinstaller|start-embeddebug-py" docs/constraints/07-directory-structure.md
rg "Python/PyQt|python/embeddebug|PyQt6" docs/constraints/03-architecture.md docs/serial_station_architecture.md
git diff --check -- docs/constraints/03-architecture.md docs/constraints/07-directory-structure.md docs/serial_station_architecture.md docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B1_Governance_Specs.md
```

本轮不执行构建或启动 smoke，因为没有修改源码、构建、启动入口、资源、依赖或实际执行路径。

## 9. 收口状态

| 轴 | 状态 | 说明 |
|----|------|------|
| Engineering | E1 | Python/PyQt governance path documented |
| User | U0 | no runtime-visible behavior |
| Device | D0 | no serial or hardware verification |
