# GS_Tool · 仓库索引

> 面向嵌入式研发、协议调试、硬件联调与产测排障的 Python/PyQt 桌面上位机（EmbedDebug）。
> 本仓库采用 **双分支结构**：`master` 仅作为项目总览索引，`develop` 承载全部源码。

## 分支模型

| 分支 | 角色 | 内容 |
|---|---|---|
| `master` | 主索引（默认分支） | 仅本 README 索引与目录说明，**不含源码** |
| `develop` | 开发主线 | 完整源码：`python/`、`tools/`、`tests/`、`docs/`、`resources/` 及工程配置 |

> 源码不提交到 `master`；所有开发在 `develop` 上进行，打包产物（二进制）不入库。

## 获取源码

```bash
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout develop
```

## 目录结构（`develop` 分支）

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

## 快速启动（`develop` 分支）

```powershell
uv sync
uv run start-embeddebug
```

## 说明

- 详细的产品能力、架构原则、质量门禁与打包交付流程见 `develop` 分支的 `README.md`。
- 提交规范：采用 Conventional Commits（如 `feat:`、`fix:`、`test:`、`docs:`、`chore:`）。
- License：MIT。
