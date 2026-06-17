# 07 - 目录结构与新增文件约束（重构版）

> 适用范围：新建、移动、删除文件或目录时必须遵守。

---

## 一、Python/PyQt 启动与产物约束

- Python/PyQt 默认入口固定：`EmbedDebug.bat` -> `uv run start-embeddebug` -> Python/PyQt。
- 用户侧启动、测试、打包和验证统一经 `uv run ...` 项目脚本暴露。
- PyInstaller 输出固定到 `dist/EmbedDebugPy-*-windows-x64/`，产物不提交。
- PyInstaller workpath 不得使用仓库 `build/`；必须由包装脚本设置到 `%TEMP%` 或其他非仓库构建目录。
- 禁止创建/引用：
  - `build2/`
  - `build-debug/`
  - `build-release/`
- `native-build-*`
- 固定考核路径：`uv run start-embeddebug --smoke` + `cmd /c EmbedDebug.bat --smoke`。

## 二、Canonical 路径

- `python/embeddebug/`：Python/PyQt 新主线运行时代码，按 `app/`、`ui/`、`controllers/`、`core/`、`protocols/`、`services/`、`workers/`、`drivers/` 分层
- `tests/python/`：Python/PyQt 新主线测试，按 `unit/`、`integration/`、`ui_smoke/` 分层
- `tests/fixtures/serial_station/`：串口协议 golden fixtures、日志样本和回放样本
- `packaging/pyinstaller/`：未来 Python/PyInstaller 打包 spec 与包装脚本；不得输出产物到该目录
- `tests/`：按模块分层编写测试
- `docs/constraints/`：约束与执行规则
- `resources/themes/`、`resources/icons/`：统一资源入口

## 三、新建文件分流规则

### 3.1 串口工站相关
- Python/PyQt 串口工站迁移只允许新增到 `python/embeddebug/serial_station/`，内部按 `ui/`、`controllers/`、`core/`、`protocols/`、`services/`、`workers/`、`drivers/`。
- 旧串口工站已移除；PRD-136/B23 后默认启动、打包和验收入口均进入 Python/PyQt。
- 不得把 Python 产品运行时代码放入 `tools/`；`tools/` 只放工程管理、启动、审计和包装辅助脚本。
- 不得恢复 `src/` native 产品主线。
- 不得把 Python 运行时代码回填到历史源码目录或原生构建链路。

### 3.2 接口与共享
- Python 主线的共享值对象优先放在 `python/embeddebug/shared/` 或对应 app 内 `core/`，不得复制 legacy native 源码到 Python 目录。

### 3.3 工具与文档
- 新工具脚本放 `tools/`，仅工程管理目的，不进入运行时模块。
- 新约束或执行说明放 `docs/constraints/`；专项方案放 `docs/serial_station_architecture.md`。
- 文档改动必须写明目标分更新点与对应约束条目。

### 3.4 Python/PyQt 工具链与打包
- Python GUI 启动、测试、打包、验证统一通过 `uv run ...` 项目脚本收束。
- 预留 Python lane：
  - `uv run start-embeddebug-py`
  - `uv run test-embeddebug-py`
  - `uv run package-embeddebug-py`
  - `uv run verify-package-embeddebug-py`
- `uv run start-embeddebug`、`uv run package-embeddebug`、`uv run verify-package-embeddebug` 是 Python/PyQt 默认 lane。
- PyInstaller 只允许用于 Python lane；优先 `onedir`，输出到 `dist/`，产物不提交。
- PyInstaller workpath 不得使用仓库 `build/`；必须由包装脚本设置到 `%TEMP%` 或其他非仓库构建目录。

## 四、冻结目录（只允许兼容维护）

- `animation2/`
- `widgets2/`
- `loader2/`
- `icons/` 与 `icon/`（现有资源仅兼容）
- `responsive/`
- `font/` 与 `fonts/`
- `shortcut/` 与 `managers/` 的平行能力
- `src/` native 产品目录（已移除，不得恢复）
- native 构建文件（已移除，不得恢复）

冻结规则：
- 不允许在冻结目录新增新业务功能。
- 仅允许兼容修复、历史引用、迁移补丁。

## 五、文件落地检查（工程门禁）

新增或改动 native 源码默认禁止进入活跃产品主线；如确需恢复，必须有单独 PRD 和用户明确批准。

新增或改动的 Python 运行时代码必须满足：
- 已有 PRD/Specs 授权具体批次。
- 生产代码位于 `python/embeddebug/`。
- 测试位于 `tests/python/` 或共享 fixtures 位于 `tests/fixtures/serial_station/`。
- 通过 `pyproject.toml` 的 uv script 暴露入口，不新增未登记的裸脚本工作流。
- PyQt6 依赖必须先完成 GPLv3/商业授权决策记录。

### 5.1 分数门禁补充

- 新建文件无落点映射：本次提交 `+0`。
- 写入冻结目录且不属于兼容修复：本次提交 `+0` 并生成修复单。
- 文件目录与约束分流符合要求：仅可进入 `+1` 计分闭环。

## 六、文档化落点

- 目录变更需同步更新：
  - 本文件
  - 主任务 PRD/Specs
  - 相关架构文档（如串口、平台迁移）
- 每次目录变更需记录在 `01-project-overview.md` 的评分日志中。

## 七、目录稳定性 KPI

- 新建路径重名率为 0。
- 平行目录写入为 0。
- 单次任务新增目录 ≤ 3，超过必须提交拆分理由。

## 八、每轮高效启动核验（目录变更亦适用）

1. 修改文件落点确认后，执行 `uv run test-embeddebug-py`。
2. 启动 `uv run start-embeddebug --smoke`。
3. 启动 `cmd /c EmbedDebug.bat --smoke`。
4. 任一项失败则阻断目录类加分记录。
