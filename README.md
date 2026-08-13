# GS_Tool · 企业级工具集架构总览

> **GS_Tool** 是一个面向研发与调试场景的**工具集合仓库**，按「索引 + 每工具一分支」的治理模型组织：
> `master` 作为默认分支，仅承载本架构总览与目录索引；**每个工具独占一个独立分支**，源码与索引严格分离，互不耦合。
>
> 本文档为仓库级企业架构说明，覆盖项目简介、治理模型、整体架构、子项目矩阵、目录结构、环境依赖、部署方式、使用说明与贡献规范。

---

## 目录

1. [项目简介](#1-项目简介)
2. [仓库治理模型](#2-仓库治理模型)
3. [整体架构](#3-整体架构)
4. [子项目矩阵](#4-子项目矩阵)
5. [目录结构](#5-目录结构)
6. [环境依赖](#6-环境依赖)
7. [部署方式](#7-部署方式)
8. [使用说明](#8-使用说明)
9. [贡献规范](#9-贡献规范)
10. [许可证与说明](#10-许可证与说明)

---

## 1. 项目简介

GS_Tool 收纳多个**相互独立、可单独交付**的研发/调试工具，当前包含三大核心工具与若干历史分支：

- **ai-token-tracker** —— 本地优先的 AI Token 用量追踪器（Windows 桌面端 + Android 客户端）。
- **QuillForge** —— 基于分层架构的桌面富文本/写作编辑器（PyQt6 / PySide6）。
- **SerialForge** —— 嵌入式设备调试控制台（PyQt6 / PySide6，含 OTA 与调试协议栈）。

仓库采用「单默认分支做索引、多工具分支存源码」的方式，使每个工具都能独立演进、独立发布、独立做版本管理，而不污染彼此的历史与构建产物。

---

## 2. 仓库治理模型

| 分支 | 角色 | 内容 |
|---|---|---|
| `master` | **架构总览（默认分支）** | 仅本 README 索引与目录结构说明，**不含任何工具源码** |
| `ai-token-tracker` | 工具分支：AI Token 追踪器 | 完整源码：Windows 服务（Flask + SQLite + CLI + WebUI + EXE）与 Android（Kotlin/Compose）客户端 |
| `QuillForge` | 工具分支：写作编辑器 | 完整源码：分层编辑器（`application` / `domain` / `infrastructure` / `plugins` / `presentation`）、`docs/ADR`、打包与脚本 |
| `SerialForge` | 工具分支：嵌入式调试控制台 | 完整源码：调试控制台（`application` / `domain` / `infrastructure` / `presentation` / `debug` / `ota`）、`docs/ADR`、配置画像、打包与 CI |
| `serial-debugger` | 历史分支：串口工站 | PyQt 上位机 EmbedDebug / SeriLink（`python/` `tools/` `tests/` `docs/` `resources/` 及工程配置） |
| `feat/embed-debug` | 历史分支 | 串口工站早期开发分支，内容已并入 `serial-debugger`，保留备查 |

> **约定**：新增工具时，从 `master` 新建一个以工具命名的独立分支（如 `embedded-diag`、`log-analyzer`），源码只进工具分支、不进 `master`。`master` 始终只维护本架构文档与各工具的目录索引。

---

## 3. 整体架构

### 3.1 架构原则

- **索引与源码分离**：`master` 永不含工具源码，仅作导航；工具源码封闭在各自分支。
- **独立可交付**：每个工具分支具备独立的 `README.md`、依赖声明（`pyproject.toml` / Gradle）、打包与 CI 流程。
- **分层与端口隔离**（桌面工具通用）：表现层（`presentation`）仅依赖应用层（`application`），领域层（`domain`）不依赖任何框架；外部依赖（UI 框架、硬件、网络）经 `infrastructure` / `ports` 适配，保证核心逻辑可测试、可替换。
- **本地优先与隐私**：用量追踪类工具默认本地存储（SQLite），不强制上云；凭据与密钥一律走环境变量，禁止硬编码。

### 3.2 工具关系

```text
                         ┌─────────────────────────────┐
                         │   GS_Tool (master 索引)      │
                         └──────────────┬──────────────┘
                  ┌────────────────────┼────────────────────┐
                  │                    │                    │
         ┌────────▼───────┐   ┌────────▼───────┐   ┌────────▼────────┐
         │ ai-token-      │   │  QuillForge    │   │  SerialForge    │
         │ tracker        │   │  (编辑器)      │   │  (调试控制台)    │
         │ Windows+Android│   │  PyQt/PySide   │   │  PyQt/PySide    │
         └────────┬───────┘   └────────┬───────┘   └────────┬────────┘
                  │                    │                    │
           分层: domain/        分层: domain/          分层: domain/
           app/infra/ui       app/infra/plugins       app/infra/debug/ota
                  │                    │                    │
            SQLite 本地存储     本地文档/工程          串口/OTA 设备通信
```

---

## 4. 子项目矩阵

| 工具 | 分支 | 技术栈 | 角色定位 | 主要入口 |
|---|---|---|---|---|
| AI Token 追踪器 | `ai-token-tracker` | Python(Flask/SQLite)、Kotlin/Compose | 本地优先的 AI 调用 Token 用量统计与可视化 | Windows `run.py` / WebUI；Android App |
| 写作编辑器 | `QuillForge` | Python、PyQt6 / PySide6、`uv` | 分层架构的桌面富文本/写作工具 | `pyproject.toml` 中 `uv` 脚本 |
| 嵌入式调试控制台 | `SerialForge` | Python、PyQt6 / PySide6、`uv`、CI | 嵌入式设备调试、协议分析与 OTA | `pyproject.toml` 中 `uv` 脚本 |

> 三个核心工具的详细能力、目录结构与质量门禁见各自分支的 `README.md`（如 `ai-token-tracker/README.md`）。

---

## 5. 目录结构

### 5.1 `ai-token-tracker` 分支（摘要）

```text
ai-token-tracker/
├── windows/            # Windows 桌面端：Flask 服务 + SQLite + CLI + WebUI + EXE 打包
│   ├── app/            # 后端服务、API、调度
│   ├── web/            # WebUI 静态资源与前端
│   └── docs/           # 归档的索引文档
├── android/            # Android 客户端（Kotlin / Jetpack Compose）
└── README.md           # 企业级工具 README
```

### 5.2 `QuillForge` 分支（摘要）

```text
QuillForge/
├── src/
│   └── quillforge/
│       ├── application/    # 用例编排、服务门面
│       ├── domain/         # 领域模型与业务规则（不依赖框架）
│       ├── infrastructure/ # 持久化、文件 IO 等外部适配
│       ├── plugins/        # 可插拔功能模块
│       └── presentation/   # PyQt6/PySide6 UI 层
├── docs/ADR/           # 架构决策记录
├── packaging/          # PyInstaller 打包配置
├── scripts/            # 开发/构建辅助脚本
├── pyproject.toml      # uv 依赖与脚本声明
└── README.md
```

### 5.3 `SerialForge` 分支（摘要）

```text
SerialForge/
├── src/
│   └── serialforge/
│       ├── application/    # 用例编排
│       ├── domain/         # 领域模型（协议/设备抽象）
│       ├── infrastructure/ # 串口/网络/存储适配
│       ├── presentation/   # PyQt6/PySide6 UI
│       ├── debug/          # 调试会话与日志
│       └── ota/            # 固件 OTA 升级
├── docs/ADR/           # 架构决策记录
├── profiles/           # 设备/环境配置画像
├── packaging/          # PyInstaller 打包配置
├── .github/workflows/  # CI 流水线
├── pyproject.toml
└── README.md
```

---

## 6. 环境依赖

### 6.1 通用

- **Git**：任意较新版本，用于分支管理与远程同步。
- **GitHub CLI（可选）**：`gh` 用于 PR / Issue / CI 管理；纯 `git push`/`pull` 由系统凭据管理器透明认证，无需额外登录。

### 6.2 各工具依赖

| 工具 | 运行环境 | 包管理 | 关键依赖 |
|---|---|---|---|
| ai-token-tracker | Windows（Python 3.11+）、Android（API 级别见 `android/`） | pip / Gradle | Flask、SQLite；AndroidX、Compose |
| QuillForge | Windows / Linux（Python 3.11+） | `uv` | PyQt6 或 PySide6 |
| SerialForge | Windows / Linux（Python 3.11+） | `uv` | PyQt6 或 PySide6、pyserial |

> 各工具分支的 `pyproject.toml` / `uv.lock` / Gradle 文件为依赖与版本的唯一事实来源。

---

## 7. 部署方式

### 7.1 获取某个工具的源码

以 AI Token 追踪器（`ai-token-tracker`）为例：

```bash
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout ai-token-tracker   # 切换至目标工具分支
```

如需其余工具，将分支名替换为 `QuillForge`、`SerialForge`、`serial-debugger` 等即可。

### 7.2 桌面工具（QuillForge / SerialForge）

```bash
uv sync                 # 依据 pyproject.toml 安装依赖
uv run <tool-script>    # 启动对应工具（脚本名见各分支 README）
```

打包为独立可执行文件（Windows）由 `packaging/` 下的 PyInstaller 配置产出，构建产物（`.exe`）不纳入版本库。

### 7.3 AI Token 追踪器

- **Windows**：运行 `windows/run.py` 启动本地服务与 WebUI；EXE 为预打包分发形态。
- **Android**：通过 `android/` 工程构建安装包（Gradle / Android Studio）。

---

## 8. 使用说明

1. 克隆仓库并切换到目标工具分支（见 7.1）。
2. 按各工具分支 `README.md` 的「环境依赖 / 部署方式 / 使用说明」章节操作。
3. 桌面工具统一使用 `uv` 进行依赖安装与启动；追踪器使用本地服务 + 客户端形态。
4. 所有工具的本地数据与凭据默认存于本地或环境变量，注意按各分支说明配置。

---

## 9. 贡献规范

### 9.1 分支与提交流程

1. 从 `master` 新建工具分支：`git checkout -b <tool-name>`。
2. 在分支内维护该工具源码与独立 `README.md`。
3. 回到 `master`，在本档案的「分支模型表」与「子项目矩阵」中追加/更新对应条目。
4. 提交并推送两个分支；`master` 保持默认分支。

### 9.2 提交规范

采用 **Conventional Commits**：

```text
feat:     新功能        fix:      缺陷修复
test:     测试增加      docs:     文档变更
chore:    构建/工程     refactor: 重构（无功能变化）
perf:     性能优化      ci:       CI 配置
```

示例：`feat(quillforge): add table export to markdown`

### 9.3 质量门禁

- 每个工具分支应自带测试（单元 / 集成 / UI smoke），合并前需通过。
- 依赖锁定文件（`uv.lock` / `gradle` 锁定）随 `pyproject.toml` / 构建文件一并提交。
- 不提交构建产物（`.exe`、`build/`、`dist/`、`.venv/`、`.ruff_cache/`、`.optimize_backup/`）与任何密钥。

### 9.4 代码评审

- 通过 `gh` 或 GitHub Web 提交 Pull Request，至少一次评审通过后合并。
- 重大架构变更须在 `docs/ADR/` 留下决策记录。

---

## 10. 许可证与说明

- **License**：MIT。
- 详细产品能力、架构原则与打包交付流程，见各工具分支自带的 `README.md`。
- `master` 仅维护本架构总览与各工具索引，不含任何工具源码。
