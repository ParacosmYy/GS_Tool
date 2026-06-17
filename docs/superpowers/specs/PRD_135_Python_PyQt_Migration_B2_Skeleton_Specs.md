# PRD-135 B2 - Python/PyQt Skeleton Specs

> 用途：建立最小 Python/PyQt 可运行骨架和 uv lane。B2 只证明 Python/PyQt 主线可启动和可测，不实现 Serial Station 功能 parity。

## 1. 目标

- 在 `python/embeddebug/` 下创建最小 Python/PyQt 应用包。
- 在 `python/embeddebug/serial_station/` 下创建最小 Serial Station PyQt window。
- 在 `pyproject.toml` 中加入 Python lane 脚本：
  - `start-embeddebug-py`
  - `test-embeddebug-py`
- 加入 PyQt6 运行依赖和 pytest/pytest-qt dev 依赖。
- 加入 pytest-qt smoke test。
- 保持 C++ baseline：`EmbedDebug.bat -> build/EmbedDebug.exe` 不变。
- 本轮三轴状态：工程 `E2骨架`，用户 `U1可见不可用`，设备 `D0未验证`。

## 2. License Decision

B2 选择 PyQt6 GPLv3-compatible development lane：

- 允许本地开发和自动化 smoke 使用 `PyQt6`。
- 禁止把 Python/PyQt app 作为 MIT-only 公共二进制发布。
- 公开发布前必须补 `LICENSE`、third-party notices、SBOM 和依赖版本清单。
- 若未来需要闭源或 MIT-only 二进制，必须另行记录 Riverbank commercial PyQt/Qt 授权或 PySide6/LGPL 改道决策。

## 3. 非目标

- 不实现串口连接。
- 不实现 RawData/FireWater/JustFloat parser。
- 不实现 pyqtgraph、NumPy ring buffer 或 VOFA+ 可视化。
- 不新增 PyInstaller spec。
- 不修改 C++ 源码、CMake、README 或 `EmbedDebug.bat`。
- 不把 `uv run start-embeddebug` 改为 Python。
- 不声称 Python 迁移完成。

## 4. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| 主要文件 | `pyproject.toml` | 更新 |
| 主要文件 | `.gitignore` | 更新 |
| 主要文件 | `python/embeddebug/` | 新增 |
| 主要文件 | `tests/python/` | 新增 |
| 主要文件 | `docs/prd/PRD_135_Python_PyQt_Migration.md` | 更新 license decision |
| 主要文件 | `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B2_Skeleton_Specs.md` | 新增 |
| 禁止修改 | `src/**` | 禁止 |
| 禁止修改 | `cmake/**` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 5. 验收标准

- [ ] `uv run start-embeddebug-py --smoke` 返回 0。
- [ ] `uv run test-embeddebug-py` 通过。
- [ ] `uv run test-embeddebug-tools` 仍通过。
- [ ] Python 源码只在 `python/embeddebug/` 下。
- [ ] Python 测试只在 `tests/python/` 下。
- [ ] `EmbedDebug.bat` 未修改。
- [ ] CMake 未修改。
- [ ] 不新增第二构建目录。

## 6. GO 配置

```powershell
uv run start-embeddebug-py --smoke
uv run test-embeddebug-py
uv run test-embeddebug-tools
git diff --check -- pyproject.toml .gitignore python tests/python docs/prd/PRD_135_Python_PyQt_Migration.md docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B2_Skeleton_Specs.md
```

本轮不执行 C++ build/start smoke，因为没有修改 C++ 源码、CMake、资源、启动脚本或 C++ 执行路径。

## 7. 收口状态

| 轴 | 状态 | 说明 |
|----|------|------|
| Engineering | E2 | PyQt skeleton package and uv scripts exist |
| User | U1 | Python window can be created in smoke, no workflow yet |
| Device | D0 | no serial transport or device validation |
