# QuillForge

> Windows-first、可扩展的文本与代码编辑器。采用分层架构，使编辑器引擎、文件服务、命令与插件能够独立演进。当前扩展目录为「仅元数据」：它只检查有界的本地 JSON 描述符，不会导入或执行外部代码。

![Platform](https://img.shields.io/badge/platform-Windows-0078D4)
![Language](https://img.shields.io/badge/language-Python%203.12-3776AB)
![UI](https://img.shields.io/badge/ui-PyQt6%20%2F%20QScintilla-41CD52)
![License](https://img.shields.io/badge/license-Proprietary-yellow)

---

## 1. 项目简介

QuillForge 是一个面向 Windows 的轻量但可扩展编辑器，设计目标是把「编辑器引擎」「文件服务」「命令系统」「插件」解耦到各自可独立演进的层。其核心特性：

- **分层架构**：Presentation / Application / Domain 严格单向依赖，插件只能通过 Public Plugin API 扩展。
- **可扩展但安全**：扩展目录为元数据驱动，仅读取本地有界 JSON 描述符，不导入或执行外部代码。
- **诊断与恢复**：内置启动诊断（`--diagnose-startup`）与文件打开诊断（`--diagnose-file-open`），以及安全模式（`--safe-mode`）用于绕过被污染的会话/恢复/插件状态。
- **测量优先**：Replace All 受 10,000 次字面匹配上限约束并以可取消的 UI 协作切片运行。

## 2. 目录结构

```text
QuillForge/
├─ src/quillforge/        # 应用源码
│  ├─ application/        # 用例（use cases）与 ports
│  ├─ domain/             # 领域值与不变式（invariants）
│  ├─ infrastructure/      # 适配器（文件服务、命令等）
│  ├─ plugins/            # 插件契约与生命周期
│  ├─ presentation/       # Qt 表现层（主题、窗口、控件）
│  ├─ resources/          # 图标与资源
│  └─ __main__.py         # 入口（main）
├─ docs/                  # 架构、约束、工作流、发布策略、ADR、Codex 评审钩子
├─ packaging/             # PyInstaller 配置（quillforge.spec）
├─ scripts/               # 可复现的开发与发布命令（check.ps1 / package.ps1 / run.ps1）
├─ skills/                # 项目级技能（企业架构 / UI 视觉质量）
├─ assets/                # 图标（quillforge-icon.png / quillforge.ico）
├─ .codex/                # Codex 代理配置（agents / config.toml）
├─ pyproject.toml         # 依赖与工具配置（ruff / hatchling / uv）
├─ uv.lock                # 锁定依赖
├─ .python-version        # Python 3.12
└─ README.md              # 本文件
```

## 3. 架构说明

依赖方向始终指向稳定契约（向内），基础设施与 Qt 特定代码不得泄漏到领域模型：

```text
Presentation ──> Application ──> Domain
      │               │
      └──────> Ports <┘
                    ▲
             Infrastructure

Plugins ──> Public plugin API ──> Application commands and events
```

关键约束：

- **内聚与耦合**：每个模块只有一种主要变更理由（领域值/用例/适配器/表现投影/插件契约与生命周期）。跨层行为必须通过显式 port 或 event/command 契约穿越，不得穿透 widget、服务容器或具体适配器。
- **扩展接缝**：插件只能消费 Public Plugin API；扩展目录仅描述元数据，执行隔离由契约保证。
- **质量门禁**：`scripts/check.ps1` 强制校验稳定内层的导入边界、ruff 规则与可接受性闸门（acceptance.json）。

## 4. 环境依赖

| 类别 | 依赖 | 版本约束 |
|---|---|---|
| 运行时 | Python | `>=3.12,<3.13`（见 `.python-version`） |
| UI 运行时 | PyQt6 | `>=6.10,<7` |
| 编辑控件 | PyQt6-QScintilla | `>=2.14,<3` |
| 开发工具 | pyinstaller / ruff | `pyinstaller>=6.21,<7` · `ruff>=0.14,<1` |
| 依赖与构建 | uv / hatchling | `uv` 管理虚拟环境与锁文件；`hatchling` 构建 |
| 操作系统 | Windows | Windows-first（Qt 原生窗口与 DPI 行为为验证边界） |

> 依赖统一置于 `pyproject.toml` 与 `uv.lock`，禁止安装到全局 Python 环境。

## 5. 部署方式（构建与打包）

### 5.1 开发运行

```powershell
uv sync            # 创建虚拟环境并安装依赖
uv run quillforge  # 启动编辑器
```

### 5.2 质量门禁

```powershell
.\scripts\check.ps1   # ruff + 导入边界 + 可接受性闸门
```

### 5.3 打包为 EXE

```powershell
.\scripts\package.ps1 # 经 PyInstaller 产出 QuillForge.exe
```

打包产物 `QuillForge.exe` 已被 `.gitignore` 排除，不纳入版本库。

## 6. 使用说明

### 6.1 常规启动

```powershell
uv run quillforge
# 或直接使用打包产物
.\QuillForge.exe
```

### 6.2 安全模式（恢复）

当正常启动被持久化会话、恢复决策、用户外观设置或内置插件激活阻塞时：

```powershell
.\QuillForge.exe --safe-mode
```

安全模式使用产品默认外观、跳过会话/恢复还原与内置插件激活，但仍接受 `--safe-mode` 后的显式文件路径；不会改写设置或会话数据。

### 6.3 启动诊断

若打包窗口未出现，运行无窗口预检并附上 JSON 报告：

```powershell
.\QuillForge.exe --diagnose-startup --report .\startup-report.json
```

针对特定文件打开的报告：

```powershell
.\QuillForge.exe --diagnose-file-open .\README.md --report .\file-open-report.json
```

> 诊断仅为「诊断边界」，不等同于原生 Windows 窗口创建、渲染、洁净机器启动、签名或发布验收的证明。

## 7. 贡献规范

- **提交规范**：遵循 Conventional Commits；提交信息应标明受影响的模块与导入边界。
- **代码风格**：ruff 规则集为 `E, F, I, UP, B`；`line-length = 100`；`quote-style = double`；`indent-style = space`。
- **依赖方向**：禁止表现层/基础设施层向领域层反向依赖；稳定内层的导入边界由 `scripts/check.ps1` 强制。
- **扩展性**：新能力只能通过 Public Plugin API 与显式 port/event 契约暴露；禁止在领域层引入 Qt 或 UI 依赖。
- **扩展目录**：仅提供有界本地 JSON 描述符，不得导入或执行外部代码。
- **规模与可验证性**：功能完成前须在执行评审中记录依赖方向、归属与扩展接缝；不宣称洁净机器启动、签名或发布验收已完成（须说明实际验证范围）。

---

> 详细文档入口：
> - 架构约束：[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)
> - 依赖与约束：[`docs/DEPENDENCIES.md`](docs/DEPENDENCIES.md) · [`docs/CONSTRAINTS.md`](docs/CONSTRAINTS.md)
> - 工作流与发布：[`docs/WORKFLOW.md`](docs/WORKFLOW.md) · [`docs/RELEASE.md`](docs/RELEASE.md)
> - 代理团队与闸门：[`docs/agent-team/TEAM.md`](docs/agent-team/TEAM.md)
