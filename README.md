# GS_Tool · 工具集仓库索引

> **GS_Tool 是一个工具集合仓库**，未来会陆续收纳很多独立的研发/调试工具。
> 本仓库采用 **索引 + 每工具一分支** 的治理方式：`master` 仅作为总览索引（本 README），**每个工具独立占用一个分支**，源码与索引严格分离。

## 分支模型

| 分支 | 角色 | 内容 |
|---|---|---|
| `master` | **总览索引（默认分支）** | 仅本 README 索引与目录结构说明，**不含任何工具源码** |
| `serial-debugger` | 工具分支：串口工站 | 完整源码：`python/`、`tools/`、`tests/`、`docs/`、`resources/` 及工程配置（PyQt 上位机 EmbedDebug / SeriLink） |
| `feat/embed-debug` | 历史分支 | 串口工站的早期开发分支，内容已并入 `serial-debugger`，保留备查 |

> **约定**：新增工具时，从 `master` 新建一个以工具命名的独立分支（如 `embedded-diag`、`log-analyzer`），源码只进工具分支，不进 `master`。`master` 始终只维护本索引与各工具的目录说明。

## 获取某个工具的源码

以串口工站（`serial-debugger`）为例：

```bash
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout serial-debugger   # 切到对应工具的分支
```

未来新增工具后，把 `serial-debugger` 换成对应工具的分支名即可。

## 目录结构（以 `serial-debugger` 分支为例）

```text
GS_Tool/
├── python/embeddebug/   # PyQt 应用入口与 Serial Station 核心工作台
│   ├── app/             # QApplication、启动参数、smoke
│   ├── serial_station/  # 上位机核心：ui / controllers / core / protocols / services / drivers / workers
│   ├── devtools/        # 测试、打包、验证命令
│   └── shared/          # 共享值对象与基础能力
├── tools/               # 工程辅助工具
├── tests/               # 单测 / 集成测试 / UI smoke
├── docs/                # PRD、工程约束、跟踪文档
├── resources/           # 图标等静态资源（SVG）
├── EmbedDebug.bat       # 唯一 Windows 用户启动入口
├── pyproject.toml       # uv scripts 与依赖声明
└── uv.lock              # 锁定依赖图
```

## 串口工站快速启动（`serial-debugger` 分支）

```powershell
uv sync
uv run start-embeddebug
```

## 新增工具的标准流程

1. 在 GitHub 上从 `master` 新建工具分支：`git checkout -b <tool-name>`。
2. 在分支内放入该工具的源码与独立的 `README.md`（工具自身说明）。
3. 回到 `master`，在本文档的分支模型表中追加一行，更新「目录结构」示例。
4. 提交并推送两个分支；`master` 保持默认分支。

## 说明

- 详细的产品能力、架构原则、质量门禁与打包交付流程，见各工具分支自带的 `README.md`（如 `serial-debugger` 分支）。
- 提交规范：采用 Conventional Commits（如 `feat:`、`fix:`、`test:`、`docs:`、`chore:`）。
- License：MIT。
