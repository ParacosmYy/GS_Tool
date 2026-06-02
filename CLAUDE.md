# EmbedDebug - 项目开发约束文档

> 本文档是所有开发行为的最高约束入口。
> 详细约束已拆分为模块化文档，分布在 `docs/constraints/` 目录下。
> 修改本文档或约束模块需要用户审查通过后方可生效。

---

## 约束模块索引

| 模块 | 文件 | 何时加载 |
|------|------|---------|
| 项目概况 + 构建环境 | [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md) | 每次开发 |
| 开发工作流 + Agent | [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md) | 每次迭代 |
| 架构原则 + 设计模式 | [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md) | 涉及架构/新增类 |
| 编码规范 | [docs/constraints/04-coding-standard.md](docs/constraints/04-coding-standard.md) | 每次编码 |
| UI执行标准 | [docs/constraints/05-ui-standard.md](docs/constraints/05-ui-standard.md) | 涉及UI改动 |
| Git + Commit规则 | [docs/constraints/06-git-commit.md](docs/constraints/06-git-commit.md) | 每次提交 |
| 目录结构 | [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md) | 涉及文件创建/移动 |

---

## 铁律（所有开发都必须遵守，违反不允许commit）

### 工作流铁律
1. **禁止不经PRD直接写代码** — 每个功能必须有PRD
2. **禁止不经架构审查直接加新类** — 新类必须通过检查清单
3. **每次commit ≥ 300行代码变更** — 不足300行不允许commit
4. **零编译错误才能commit** — 编译不过必须先修
5. **每次commit后必须验证 EmbedDebug.bat 能正常启动**

### 架构铁律
6. **分层单向依赖**: 表现层→业务层→数据层→基础设施层，**禁止反向**
7. **禁止在MainWindow中写业务逻辑** — 委托给Controller/Manager
8. **MainWindow.cpp ≤ 500行** — 超过必须拆分
9. **公共组件只写一次** — CRC/HexConverter/RingBuffer/SettingsManager等已验证组件不得重写

### 编码铁律
10. **C++17标准** — 头文件引用: Qt→STL→项目，使用相对src路径
11. **Qt信号/槽用新式connect语法** — 禁止SIGNAL/SLOT宏
12. **详细中文注释** — Doxygen格式，每个公开方法/成员变量必须有注释
13. **禁止裸new不配对delete** — QObject父子树或智能指针

### UI铁律
14. **禁止C++中硬编码颜色到setStyleSheet()** — 颜色从QSS主题获取
15. **所有QWidget必须设置objectName** — QSS依赖
16. **按钮必须有hover/pressed/disabled三种状态**
17. **面板切换必须有过渡动画** — 禁止突然出现/消失
18. **所有用户可见文字必须用tr()包裹**

### 文件体积铁律
19. **.cpp ≤ 500行** — 超过说明职责过多
20. **.h ≤ 200行** — 超过说明成员/方法过多
21. **单个方法 ≤ 80行** — 超过说明逻辑过于复杂

---

## 快速参考

| 项 | 值 |
|----|-----|
| 应用名称 | EmbedDebug |
| 项目路径 | `E:\Embedded\Tool\Serial_tool\User_Serial` |
| 当前版本 | 0.1.0 |
| 评分 | 见 [docs/tracking/SCORE_TRACKING.md](docs/tracking/SCORE_TRACKING.md) |
| Git分支 | `feat/embed-debug` |
| Git远程 | `https://github.com/ParacosmYy/GS_Tool.git` |

### 构建命令
```bash
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64
cmake --build build
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/windeployqt.exe build/EmbedDebug.exe
```

### Commit Message格式
```
<模块名>: <简述改了什么>

<详细说明为什么这样改>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```
