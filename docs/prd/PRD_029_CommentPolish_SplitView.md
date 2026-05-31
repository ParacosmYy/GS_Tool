# PRD-029: 核心模块详细中文注释 + MainWindow面板自管理 + TX/RX分栏显示

## 背景

commit #27 已完成 ToolbarController/SettingsController 提取、lineAt 悬空引用修复、objectName 补全和内联样式迁移，当前评分 29 分。MainWindow.cpp 已降至 497 行，架构拆分效果显著。

用户在最新指示中明确提出了四个方向性要求:

1. **MainWindow 应像嵌入式 main 一样简洁** -- 只做初始化和任务调度，不持有任何业务逻辑或UI创建细节
2. **代码注释必须详细** -- 每个类、方法、成员变量都要有中文注释，开发者是 C++/Qt 新手
3. **串口功能要"做精做细"** -- TX/RX 分栏显示是串口工具的核心差异化功能
4. **UI 要美观现代化** -- 分栏模式下的视觉设计需符合 CLAUDE.md 第六章标准

经审查发现以下现状问题:

**注释质量审查结果**:

| 模块 | 文件 | 行数 | 当前注释质量 | 缺陷 |
|------|------|------|------------|------|
| core/MainWindow.h | 147行 | 有类注释，成员有分组注释 | 缺少成员变量用途说明 |
| core/MainWindow.cpp | 497行 | 有行内注释 | 方法缺少详细文档注释 |
| core/ConnectionController | 266行 | 有类职责注释 | 成员变量缺少注释 |
| core/SendController | 246行 | 有类职责注释 | 成员变量缺少注释 |
| core/SettingsController | 162行 | 有类职责注释 | 方法缺少参数说明 |
| core/ToolbarController | 244行 | 有类职责注释 | 成员变量缺少注释 |
| core/NavigationController | 334行 | 有类职责注释 | 成员变量缺少注释 |
| core/BackgroundWidget | 236行 | 有行内注释 | 类职责注释不完整 |
| terminal/TerminalWidget | 686行 | 较完善 | 部分方法缺少注释 |
| terminal/TerminalModel | 229行 | 较完善 | 成员变量注释可加强 |
| terminal/TerminalSearchBar | 272行 | 一般 | 多处缺注释 |
| serial/SerialConfigPanel | 327行 | 有行内注释 | 方法缺少文档 |
| utils/* | 多文件 | 参差不齐 | 部分纯函数无注释 |

**MainWindow 面板管理审查结果**:

当前 `MainWindow::setupUI()` 仍然承担了大量面板创建和布局代码(约110行，第86-196行)，包括:
- 创建所有面板 widget (SerialConfigPanel、DataStatistics、ProtocolView、FrameVisualEditor、ChartWidget、OtaWidget)
- 创建终端容器 (TerminalSearchBar + TerminalWidget)
- 创建 QuickCommandBar 和 SendBar
- 将所有面板堆叠到 serialLayout 中

这些面板创建逻辑应该由各面板自身的工厂方法或面板管理器来负责，MainWindow 只做顶层布局骨架和信号连接。

**串口分栏可行性**:

TerminalWidget 已具备 `setDirectionFilter(DataDirection)` 方法，TerminalModel 已有 `DataDirection::Rx/Tx` 枚举，Constants.h 已预定义 `TerminalLayout` 枚举(Mixed/SplitHorizontal/SplitVertical)。基础设施已就绪，但缺少:
1. 分栏布局管理器 -- 负责创建两个 TerminalWidget 实例并设置方向过滤器
2. 布局模式切换UI -- 在工具栏或终端右键菜单中提供切换入口
3. 分栏同步滚动 -- 两个终端的滚动位置联动
4. 搜索高亮联动 -- 搜索在一个终端命中时另一个也同步高亮

**审查基准**: commit #27, score 29。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 核心模块详细中文注释 -- 所有核心模块的类/方法/成员变量补全完整中文注释 | P0 | src/下所有.h/.cpp文件 |
| R2 | MainWindow面板创建提取 -- setupUI中的面板创建逻辑提取到PanelManager或各面板自身管理 | P1 | core/MainWindow, core/PanelManager(新增) |
| R3 | TX/RX分栏显示 -- 左右分栏模式，左侧RX右侧TX，支持配置切换 | P1 | core/TerminalLayoutManager(新增), terminal/TerminalWidget, core/MainWindow |
| R4 | CLAUDE.md约束文档更新 -- 新增"MainWindow嵌入式main哲学"和"注释必须详细"两条规则 | P2 | CLAUDE.md |

---

## 需求详细说明

---

### R1: 核心模块详细中文注释 (P0)

#### 问题分析

CLAUDE.md 5.1 节明确规定"详细中文注释（开发者是C++/Qt新手）"，但当前代码中大量模块的注释水平未达到该标准。主要问题:

1. **头文件类注释不完整** -- 只有简单的一句话描述，缺少职责边界说明、线程安全性说明、生命周期说明
2. **方法缺少文档注释** -- 只有方法名，没有参数含义说明、返回值说明、调用时机说明、副作用说明
3. **成员变量缺少用途注释** -- 只有类型和名称，没有说明该变量的用途、初始值含义、生命周期归属
4. **信号缺少触发条件注释** -- 只声明了信号签名，没有说明何时触发、携带什么数据

#### 注释标准定义

所有核心模块(.h和.cpp)必须达到以下注释标准:

**类级注释模板**:
```cpp
/**
 * @brief 类的简短中文描述（一句话）
 *
 * 详细职责说明:
 *   1. 职责A -- 说明
 *   2. 职责B -- 说明
 *   3. 职责C -- 说明
 *
 * 线程安全性: 是/否，说明哪些方法需要在主线程调用
 * 生命周期: 由谁创建、由谁销毁、父对象是谁
 * 依赖: 列出主要依赖的其他类
 */
```

**方法级注释模板**:
```cpp
/**
 * @brief 方法的简短中文描述
 *
 * 详细说明（如果方法逻辑复杂需要补充）。
 * 当xxx时调用此方法，会触发yyy。
 *
 * @param paramName 参数的中文含义说明
 * @return 返回值的中文含义说明
 *
 * @note 特殊注意事项（如线程安全、性能影响等）
 * @see 相关的其他方法
 */
```

**成员变量注释模板**:
```cpp
// 该变量的中文用途说明（一句话，说明用途和初始值含义）
// 如果是标志位: 说明true和false分别代表什么状态
// 如果是指针: 说明所有权归属（本类拥有/外部注入/不拥有）
```

**信号注释模板**:
```cpp
// 当xxx事件发生时发出此信号
// 参数说明: paramName - 参数的中文含义
```

#### 方案设计

按模块分批为所有核心文件补全注释。以下列出需要处理的所有文件，按优先级排序:

**第一批: core/ 目录（P0，最高优先级）**

| 文件 | 行数 | 预估新增注释行数 | 说明 |
|------|------|----------------|------|
| MainWindow.h | 147 | +30 | 类注释、所有成员变量补全注释 |
| MainWindow.cpp | 497 | +60 | 所有方法补全详细注释 |
| ConnectionController.h/cpp | 266 | +30 | 类注释增强、成员补全 |
| SendController.h/cpp | 246 | +25 | 类注释增强、成员补全 |
| SettingsController.h/cpp | 162 | +20 | 方法参数说明补全 |
| ToolbarController.h/cpp | 244 | +20 | 成员变量补全 |
| NavigationController.h/cpp | 334 | +25 | 成员变量补全 |
| BackgroundWidget.h/cpp | 236 | +15 | 类注释增强 |
| BackgroundSettingsPopup.h/cpp | -- | +10 | 类注释增强 |
| Constants.h | 80 | +15 | 每个枚举值补全说明 |

**第二批: terminal/ 目录（P0）**

| 文件 | 行数 | 预估新增注释行数 | 说明 |
|------|------|----------------|------|
| TerminalWidget.h | 139 | +25 | CachedLine注释增强、所有方法补全 |
| TerminalWidget.cpp | 547 | +50 | 关键渲染逻辑补全注释 |
| TerminalModel.h/cpp | 229 | +20 | 环形缓冲区逻辑注释增强 |
| TerminalSearchBar.h/cpp | 272 | +20 | 搜索逻辑注释增强 |
| TerminalTypes.h | 18 | +5 | 结构体字段注释增强 |

**第三批: serial/ 目录（P1）**

| 文件 | 行数 | 预估新增注释行数 |
|------|------|----------------|
| SerialConfigPanel.h/cpp | 327 | +25 |
| QuickCommandBar.h/cpp | 149 | +15 |
| SendHistory.h/cpp | 145 | +10 |
| DataStatistics.h/cpp | 254 | +15 |
| TimedSender.h/cpp | 110 | +10 |
| SerialDriverDetector.h/cpp | 142 | +10 |

**第四批: connection/ + utils/ 目录（P1）**

| 文件 | 行数 | 预估新增注释行数 |
|------|------|----------------|
| IConnection.h | -- | +10 |
| SerialConnection.h/cpp | 281 | +20 |
| TcpConnection.h/cpp | -- | +15 |
| UdpConnection.h/cpp | -- | +15 |
| RingBuffer.h | 100 | +15 |
| SettingsManager.h/cpp | 179 | +15 |
| DataExporter.h/cpp | 323 | +20 |
| DataLogger.h/cpp | 425 | +25 |
| CRC.h | 116 | +10 |
| HexConverter.h | 62 | +10 |

**注释行数预估总计**: 约 570 行新增注释。

---

### R2: MainWindow面板创建提取 (P1)

#### 问题分析

当前 `MainWindow::setupUI()` 包含约 110 行面板创建代码（第122-181行），职责包括:
- 创建 SerialConfigPanel 并设为不可见
- 创建 DataStatistics 并设为不可见
- 创建 ProtocolView 并设为不可见
- 创建 FrameVisualEditor 并设为不可见
- 创建 ChartWidget 并设为不可见
- 创建 OtaWidget 并设为不可见
- 创建 TerminalSearchBar + TerminalWidget 终端容器
- 创建 QuickCommandBar 并设置默认指令
- 调用 SendController::createSendBar() 创建发送区域

这些面板的创建、初始可见性设置和堆叠管理应该由一个专门的面板管理器负责。MainWindow 的 setupUI() 应该只做:
1. 创建背景层 (BackgroundWidget)
2. 创建分割器 (QSplitter)
3. 创建导航树 (QTreeView) 和右侧容器骨架
4. 将面板创建委托给 PanelManager
5. 创建 Ctrl+F 快捷键

#### 方案设计

##### 新增类: PanelManager

**职责**: 管理所有功能面板的创建、堆叠、初始可见性和面板查找。

```
文件: src/core/PanelManager.h / src/core/PanelManager.cpp
层级: 表现层 (QObject)
```

```cpp
/**
 * @brief 面板管理器 - 集中管理所有功能面板的创建和生命周期
 *
 * 详细职责:
 *   1. 创建所有功能面板(SerialConfigPanel, DataStatistics, ProtocolView等)
 *   2. 将面板堆叠到指定的父容器中
 *   3. 管理面板的初始可见性(默认只显示终端)
 *   4. 提供面板查找接口(通过面板名称获取widget指针)
 *   5. 提供面板映射表供NavigationController构建导航树
 *
 * 线程安全性: 否，所有方法必须在主线程调用
 * 生命周期: 由MainWindow创建，作为MainWindow的子对象
 * 依赖: SendController(创建SendBar), OtaManager(创建OtaWidget),
 *        FrameParser, ProtocolBridgeManager(面板间信号连接)
 */
class PanelManager : public QObject {
    Q_OBJECT

public:
    explicit PanelManager(SendController* sendController,
                          OtaManager* otaManager,
                          QObject* parent = nullptr);

    /**
     * @brief 创建所有面板并堆叠到指定容器中
     *
     * 创建SerialConfigPanel、DataStatistics、ProtocolView、FrameVisualEditor、
     * ChartWidget、OtaWidget、终端容器(TerminalSearchBar+TerminalWidget)、
     * QuickCommandBar、SendBar，全部添加到container的布局中。
     * 除终端容器外，所有面板初始设为不可见。
     *
     * @param container 面板堆叠的目标容器widget，必须有QVBoxLayout
     * @param terminalModel 终端数据模型，用于TerminalWidget的setModel
     */
    void createPanels(QWidget* container, TerminalModel* terminalModel);

    /**
     * @brief 获取面板映射表，供NavigationController构建导航树
     *
     * 返回一个有序的(面板名称, 面板widget指针对)列表，
     * 顺序决定导航树中的显示顺序。
     * 面板名称使用QT_TRANSLATE_NOOP宏包裹，支持翻译。
     *
     * @return 面板映射表: {翻译键, QWidget指针}
     */
    QVector<QPair<QString, QWidget*>> panelMap() const;

    // ---- 各面板的getter接口 ----

    /** @return 串口配置面板 */
    SerialConfigPanel* serialConfigPanel() const;

    /** @return 终端显示控件 */
    TerminalWidget* terminalWidget() const;

    /** @return 终端搜索栏 */
    TerminalSearchBar* searchBar() const;

    /** @return 快捷指令栏 */
    QuickCommandBar* quickCommandBar() const;

    /** @return 数据统计面板 */
    DataStatistics* dataStatistics() const;

    /** @return 协议解析视图 */
    ProtocolView* protocolView() const;

    /** @return 帧可视化编辑器 */
    FrameVisualEditor* frameVisualEditor() const;

    /** @return 波形图控件 */
    ChartWidget* chartWidget() const;

    /** @return OTA升级面板 */
    OtaWidget* otaWidget() const;

    /** @return 帧解析器 */
    FrameParser* frameParser() const;

    /** @return 协议桥管理器 */
    ProtocolBridgeManager* protocolBridgeManager() const;

private:
    // 各面板指针 -- PanelManager拥有所有权，作为本对象的子对象
    SerialConfigPanel* m_serialConfig;
    DataStatistics* m_dataStats;
    ProtocolView* m_protocolView;
    FrameVisualEditor* m_frameEditor;
    ChartWidget* m_chartWidget;
    OtaWidget* m_otaWidget;
    TerminalWidget* m_terminal;
    TerminalSearchBar* m_searchBar;
    QuickCommandBar* m_quickCmdBar;
    FrameParser* m_frameParser;
    ProtocolBridgeManager* m_protocolBridgeMgr;

    // 外部依赖（不拥有所有权）
    SendController* m_sendController;
    OtaManager* m_otaManager;
};
```

##### MainWindow 适配

MainWindow::setupUI() 改造后的伪代码:

```cpp
void MainWindow::setupUI()
{
    // 1. 背景层
    m_backgroundWidget = new BackgroundWidget(this);
    setCentralWidget(m_backgroundWidget);
    // ... 布局设置 ...

    // 2. 分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, m_backgroundWidget);

    // 3. 左侧导航树
    m_navTree = new QTreeView;
    m_navTree->setObjectName("navTree");
    // ... 树属性设置 ...
    m_mainSplitter->addWidget(m_navTree);

    // 4. 右侧容器骨架
    auto* rightWidget = new QWidget;
    rightWidget->setObjectName("rightWidget");
    auto* rightLayout = new QVBoxLayout(rightWidget);
    // ... 布局设置 ...
    m_rightPanel = new QWidget;
    m_rightPanel->setObjectName("rightPanel");
    auto* rightPanelLayout = new QVBoxLayout(m_rightPanel);
    // ... 布局设置 ...

    // 5. 委托PanelManager创建所有面板
    m_panelManager->createPanels(m_rightPanel, m_terminalModel);

    m_mainSplitter->addWidget(rightWidget);
    m_mainSplitter->setSizes({200, 1000});

    // 6. Ctrl+F快捷键
    auto* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated,
            m_panelManager->searchBar(), &TerminalSearchBar::activate);
}
```

MainWindow 成员变量大幅减少:
- 删除: m_serialConfig, m_dataStats, m_protocolView, m_frameEditor, m_chartWidget, m_otaWidget, m_terminal, m_searchBar, m_quickCmdBar, m_frameParser, m_protocolBridgeMgr
- 新增: m_panelManager
- 对外通过 m_panelManager->xxxPanel() 获取面板指针

**预计 MainWindow.cpp 行数**: 497 - 70(面板创建) + 10(PanelManager初始化) = 约 437 行。

---

### R3: TX/RX 分栏显示 (P1)

#### 问题分析

当前终端只支持混合模式(Mixed) -- TX 和 RX 数据在同一终端中按时间顺序交替显示。对于嵌入式调试场景，分栏模式是高频需求:
- 调试AT指令时，需要清楚看到发送了什么命令、收到了什么响应
- 调试通信协议时，需要对比请求帧和应答帧的时序
- 大流量数据场景下，TX/RX混在一起难以阅读

TerminalWidget 已具备 `setDirectionFilter(DataDirection)` 方法，可以直接复用。Constants.h 已预定义 `TerminalLayout` 枚举。基础设施完备，需要构建分栏布局管理器。

#### 方案设计

##### 新增类: TerminalLayoutManager

**职责**: 管理终端的布局模式切换（混合模式/左右分栏/上下分栏）。

```
文件: src/core/TerminalLayoutManager.h / src/core/TerminalLayoutManager.cpp
层级: 表现层 (QObject)
```

```cpp
/**
 * @brief 终端布局管理器 - 控制TX/RX数据的显示布局方式
 *
 * 详细职责:
 *   1. 管理三种布局模式: 混合(Mixed)、左右分栏(SplitHorizontal)、上下分栏(SplitVertical)
 *   2. 混合模式: 使用单个TerminalWidget显示所有数据
 *   3. 分栏模式: 使用两个TerminalWidget实例，分别过滤RX和TX数据
 *   4. 分栏模式下提供可选的分隔线和标签("RX"/"TX")
 *   5. 搜索高亮在两个终端间联动
 *   6. 支持运行时切换布局模式
 *
 * 线程安全性: 否，所有方法必须在主线程调用
 * 生命周期: 由PanelManager或MainWindow创建
 * 依赖: TerminalModel(数据源), TerminalWidget(显示)
 */
class TerminalLayoutManager : public QObject {
    Q_OBJECT

public:
    explicit TerminalLayoutManager(QObject* parent = nullptr);
    ~TerminalLayoutManager() override;

    /**
     * @brief 初始化布局管理器
     *
     * 创建主终端(Mixed模式使用)和分栏终端(RX/TX两个实例)。
     * 初始状态为Mixed模式，只有主终端可见。
     *
     * @param terminalModel 终端数据模型，所有TerminalWidget共享同一个模型
     */
    void initialize(TerminalModel* terminalModel);

    /**
     * @brief 获取当前布局模式
     */
    TerminalLayout currentLayout() const;

    /**
     * @brief 切换布局模式
     *
     * 切换时执行以下操作:
     *   1. 保存当前布局的滚动位置
     *   2. 隐藏当前布局的widget
     *   3. 显示目标布局的widget
     *   4. 恢复目标布局的滚动位置(如果之前有保存)
     *
     * @param layout 目标布局模式
     */
    void setLayout(TerminalLayout layout);

    /**
     * @brief 获取主终端widget（混合模式使用的终端）
     *
     * 主终端始终存在，不设方向过滤器，显示所有数据。
     * 用于搜索、复制、导出等操作的主入口。
     */
    TerminalWidget* primaryTerminal() const;

    /**
     * @brief 获取当前可见的终端widget列表
     *
     * Mixed模式返回{primaryTerminal}，
     * SplitHorizontal/SplitVertical返回{rxTerminal, txTerminal}。
     * 用于搜索、快捷键等需要作用于当前可见终端的场景。
     */
    QVector<TerminalWidget*> activeTerminals() const;

    /**
     * @brief 获取包含所有终端的容器widget
     *
     * 容器widget内部使用QStackedWidget管理Mixed和Split两种布局。
     * 调用方将此容器添加到面板布局中。
     */
    QWidget* container() const;

    /**
     * @brief 获取终端搜索栏
     *
     * 搜索栏只有一个实例，搜索操作作用于所有活动终端。
     */
    TerminalSearchBar* searchBar() const;

    /**
     * @brief 将当前布局设置应用到指定配置映射（用于持久化）
     */
    QVariantMap saveLayoutConfig() const;

    /**
     * @brief 从配置映射恢复布局设置
     */
    void restoreLayoutConfig(const QVariantMap& config);

signals:
    /**
     * @brief 布局模式变更信号
     *
     * 当布局模式切换时发出，用于通知工具栏下拉框更新选中项。
     * @param layout 新的布局模式
     */
    void layoutChanged(TerminalLayout layout);

private slots:
    /**
     * @brief 主终端搜索结果变化时，同步到分栏终端
     */
    void onSearchMatchesChanged(int total, int current);

private:
    /**
     * @brief 创建分栏布局容器
     *
     * 创建RX终端和TX终端，分别设置方向过滤器。
     * 添加标签("RX"/"TX")和可选分隔线。
     */
    void createSplitLayout();

    TerminalModel* m_terminalModel;     // 数据模型（共享，不拥有）

    TerminalLayout m_currentLayout;     // 当前布局模式

    // 主终端（Mixed模式使用，显示所有数据）
    TerminalWidget* m_primaryTerminal;

    // 分栏终端（Split模式使用，分别过滤RX和TX）
    TerminalWidget* m_rxTerminal;       // 接收数据终端
    TerminalWidget* m_txTerminal;       // 发送数据终端

    // 搜索栏（共享，只有一个实例）
    TerminalSearchBar* m_searchBar;

    // 容器布局
    QWidget* m_container;               // 顶层容器
    QStackedWidget* m_stackWidget;      // Mixed页面和Split页面的切换容器
    QWidget* m_mixedPage;               // Mixed模式页面（只包含主终端）
    QWidget* m_splitPage;               // Split模式页面（包含RX和TX终端）

    // 分栏标签
    QLabel* m_rxLabel;                  // "RX" 标签（分栏模式下显示）
    QLabel* m_txLabel;                  // "TX" 标签（分栏模式下显示）
};
```

##### 分栏布局视觉设计

**左右分栏 (SplitHorizontal)**:

```
+---------------------------+---------------------------+
| RX 接收数据               | TX 发送数据               |
| (蓝色文字，左侧)          | (绿色文字，右侧)          |
|                           |                           |
| > Hello from MCU          | [TX] AT+STATUS?           |
| > OK                      | [TX] AT+RESET             |
| > System ready            | [TX] AA 55 01 00 FE       |
|                           |                           |
+---------------------------+---------------------------+
```

- 左右各占50%宽度，中间有2px分隔线（使用border颜色）
- 左上角显示"RX"标签（接收颜色），右上角显示"TX"标签（发送颜色）
- 标签使用半透明背景，悬浮在终端内容之上
- 分隔线可拖拽调整左右比例（使用QSplitter）

**上下分栏 (SplitVertical)**:

```
+-------------------------------------------------------+
| RX 接收数据                                           |
| > Hello from MCU                                      |
| > OK                                                  |
| > System ready                                        |
+-------------------------------------------------------+
| TX 发送数据                                           |
| [TX] AT+STATUS?                                       |
| [TX] AT+RESET                                         |
| [TX] AA 55 01 00 FE                                   |
+-------------------------------------------------------+
```

- 上下各占50%高度，中间有2px分隔线
- 分隔线可拖拽调整上下比例

##### 工具栏布局切换入口

在 ToolbarController 中新增终端布局下拉框:

```
位置: 工具栏中，紧跟在 displayModeCombo 之后
选项: "混合" | "左右分栏" | "上下分栏"
默认: "混合"
objectName: "layoutCombo"
```

##### 搜索联动机制

分栏模式下搜索联动:
1. 用户在搜索栏输入关键词
2. 搜索同时应用于RX终端和TX终端
3. F3导航在两个终端间切换: 先在RX中跳转，跳完后再跳到TX中的匹配项
4. 搜索栏显示总匹配数 = RX匹配数 + TX匹配数

---

### R4: CLAUDE.md 约束文档更新 (P2)

#### 问题分析

CLAUDE.md 当前缺少两条重要约束:

1. **MainWindow嵌入式main哲学** -- 虽然第四章有"MainWindow特殊规则"，但只提到行数上限和拆分策略，没有明确"MainWindow应该像嵌入式main一样简洁"的哲学原则。需要提升为架构级别的约束。

2. **注释必须详细** -- 虽然第五章5.1节提到"详细中文注释（开发者是C++/Qt新手）"，但这只是一句话提及，没有量化标准和注释模板。需要增加具体的注释规范和注释质量检查项。

#### 方案设计

##### 新增约束 1: MainWindow 嵌入式 main 哲学

在 CLAUDE.md 第四章 "架构原则" 中的 4.6 节之后新增 4.7 节:

```markdown
### 4.7 MainWindow 嵌入式 main 哲学（铁律）

> 核心理念: MainWindow 就像嵌入式系统中的 main() 函数，
> 只负责三件事: 初始化硬件、创建任务、启动调度器。
> 它不应该包含任何业务逻辑或UI创建细节。

**MainWindow 允许做的事情（白名单）**:
1. 创建顶层布局骨架（BackgroundWidget + QSplitter + QTreeView + 右侧容器）
2. 创建并初始化控制器和模型对象
3. 连接控制器之间的信号和槽
4. 加载/保存设置（委托给SettingsController）
5. 处理窗口关闭事件

**MainWindow 禁止做的事情（黑名单）**:
1. 禁止直接创建功能面板（SerialConfigPanel、TerminalWidget等） -- 由PanelManager负责
2. 禁止直接操作面板可见性 -- 由NavigationController负责
3. 禁止包含数据处理逻辑 -- 由各Controller负责
4. 禁止包含QPainter绘制代码 -- 由各Widget负责
5. 禁止直接调用 setStyleSheet -- 由QSS主题文件负责

**判别标准**: 如果MainWindow中的某段代码可以提取到一个专门的类中，那它就不应该留在MainWindow中。

**参考代码量**: MainWindow.cpp 目标控制在 400 行以内（纯初始化+信号连接）。
```

##### 新增约束 2: 详细注释规范

在 CLAUDE.md 第五章 "编码规范" 中的 5.1 节扩展:

```markdown
### 5.1.1 注释详细度标准（铁律）

> 目标读者: C++/Qt 新手开发者。注释要让新手能完全理解每个模块的设计意图和实现细节。

**每个文件必须包含的注释**:
1. 文件头注释: 模块简介（如果.h文件没有类级注释则必须有）
2. 类级注释: @brief + 详细职责列表 + 线程安全性 + 生命周期 + 依赖关系
3. 公开方法注释: @brief + @param + @return + @note(特殊情况)
4. 私有方法注释: @brief（至少一句话说明用途）
5. 成员变量注释: 用途说明 + 所有权归属（对于指针）
6. 信号注释: 触发条件 + 参数含义

**注释密度要求**:
- 头文件(.h): 注释行数 >= 代码行数的 30%
- 实现文件(.cpp): 注释行数 >= 代码行数的 20%
- 关键算法/渲染逻辑: 注释行数 >= 代码行数的 50%

**注释质量检查项**（每次commit前自检）:
- [ ] 新增类的头文件是否有完整的类级注释？
- [ ] 所有公开方法是否有@brief和@param注释？
- [ ] 所有成员变量是否有用途注释？
- [ ] 所有信号是否有触发条件注释？
- [ ] 复杂逻辑是否有行内注释说明"为什么"这样做？
- [ ] 环形缓冲区/锁/线程相关代码是否有安全性注释？
```

---

## 接口设计

### 新增接口

| 接口 | 文件 | 说明 |
|------|------|------|
| `PanelManager::PanelManager()` | core/PanelManager.h/cpp | 构造函数 |
| `PanelManager::createPanels()` | core/PanelManager.h/cpp | 创建所有面板并堆叠到容器 |
| `PanelManager::panelMap()` | core/PanelManager.h/cpp | 获取面板映射表 |
| `PanelManager::serialConfigPanel()` | core/PanelManager.h/cpp | 获取串口配置面板 |
| `PanelManager::terminalWidget()` | core/PanelManager.h/cpp | 获取主终端 |
| `PanelManager::searchBar()` | core/PanelManager.h/cpp | 获取搜索栏 |
| `PanelManager::quickCommandBar()` | core/PanelManager.h/cpp | 获取快捷指令栏 |
| `PanelManager::dataStatistics()` | core/PanelManager.h/cpp | 获取数据统计面板 |
| `PanelManager::protocolView()` | core/PanelManager.h/cpp | 获取协议视图 |
| `PanelManager::frameVisualEditor()` | core/PanelManager.h/cpp | 获取帧编辑器 |
| `PanelManager::chartWidget()` | core/PanelManager.h/cpp | 获取波形图 |
| `PanelManager::otaWidget()` | core/PanelManager.h/cpp | 获取OTA面板 |
| `PanelManager::frameParser()` | core/PanelManager.h/cpp | 获取帧解析器 |
| `PanelManager::protocolBridgeManager()` | core/PanelManager.h/cpp | 获取协议桥管理器 |
| `TerminalLayoutManager::TerminalLayoutManager()` | core/TerminalLayoutManager.h/cpp | 构造函数 |
| `TerminalLayoutManager::initialize()` | core/TerminalLayoutManager.h/cpp | 初始化终端布局 |
| `TerminalLayoutManager::setLayout()` | core/TerminalLayoutManager.h/cpp | 切换布局模式 |
| `TerminalLayoutManager::currentLayout()` | core/TerminalLayoutManager.h/cpp | 获取当前布局 |
| `TerminalLayoutManager::primaryTerminal()` | core/TerminalLayoutManager.h/cpp | 获取主终端 |
| `TerminalLayoutManager::activeTerminals()` | core/TerminalLayoutManager.h/cpp | 获取活动终端列表 |
| `TerminalLayoutManager::container()` | core/TerminalLayoutManager.h/cpp | 获取容器widget |
| `TerminalLayoutManager::searchBar()` | core/TerminalLayoutManager.h/cpp | 获取搜索栏 |
| `TerminalLayoutManager::layoutChanged` | core/TerminalLayoutManager.h | 布局变更信号 |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| `MainWindow::setupUI()` | 面板创建委托给PanelManager | 外部行为不变 |
| `MainWindow` 成员变量 | 删除11个面板指针，新增PanelManager和TerminalLayoutManager | 对外接口不变 |
| `NavigationController::buildNavTree()` | 面板映射表来源改为PanelManager::panelMap() | 行为不变 |
| `ToolbarController` | 新增layoutCombo下拉框和layoutChanged信号 | 扩展，不影响现有功能 |

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| `TerminalModel` | terminal/TerminalModel.h | TerminalLayoutManager 共享数据模型 | R3 |
| `TerminalWidget` | terminal/TerminalWidget.h | setDirectionFilter() 方向过滤 | R3 |
| `TerminalSearchBar` | terminal/TerminalSearchBar.h | 搜索功能在分栏模式下的联动 | R3 |
| `Constants::TerminalLayout` | core/Constants.h | 布局模式枚举定义 | R3 |
| `Constants::DataDirection` | core/Constants.h | 数据方向枚举 | R3 |
| `SettingsManager` | utils/SettingsManager.h/cpp | 布局偏好持久化 | R3 |
| `SendController` | core/SendController.h/cpp | PanelManager 创建 SendBar | R2 |
| `OtaManager` | ota/OtaManager.h/cpp | PanelManager 创建 OtaWidget | R2 |
| `FrameParser` | protocol/FrameParser.h | PanelManager 创建协议解析面板 | R2 |
| `ProtocolBridgeManager` | protocol/ProtocolBridgeManager.h | PanelManager 创建协议桥面板 | R2 |
| `NavigationController` | core/NavigationController.h/cpp | PanelManager 提供面板映射表 | R2 |
| `BackgroundWidget` | core/BackgroundWidget.h/cpp | MainWindow 顶层背景 | R2 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **外观模式 (Facade)** | PanelManager 封装所有面板的创建和管理 | R2 | 对外暴露简单的 createPanels() 和 getter 接口，隐藏内部面板创建细节 |
| **策略模式 (Strategy)** | TerminalLayoutManager 的布局模式切换 | R3 | Mixed/SplitHorizontal/SplitVertical 三种布局策略，运行时切换 |
| **组合模式 (Composite)** | 分栏模式下两个TerminalWidget组合为统一容器 | R3 | 对外暴露统一的 container() 接口，内部管理多个终端实例 |
| **观察者模式 (Observer)** | 搜索匹配联动 | R3 | 主终端搜索结果变化时通知分栏终端同步高亮 |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 |
|------|---------|-----|-----|-----|-----|
| `src/core/MainWindow.h` | 注释增强+成员减少 | +30行 | +2行/-11行 | +2行 | -- |
| `src/core/MainWindow.cpp` | 注释增强+面板委托 | +60行 | -70行/+10行 | +5行 | -- |
| `src/core/PanelManager.h` | 新增 | -- | +80行 | -- | -- |
| `src/core/PanelManager.cpp` | 新增 | -- | +120行 | -- | -- |
| `src/core/TerminalLayoutManager.h` | 新增 | -- | -- | +90行 | -- |
| `src/core/TerminalLayoutManager.cpp` | 新增 | -- | -- | +180行 | -- |
| `src/core/ConnectionController.h/cpp` | 注释增强 | +30行 | -- | -- | -- |
| `src/core/SendController.h/cpp` | 注释增强 | +25行 | -- | -- | -- |
| `src/core/SettingsController.h/cpp` | 注释增强 | +20行 | -- | -- | -- |
| `src/core/ToolbarController.h/cpp` | 注释增强+新增layoutCombo | +20行 | -- | +15行 | -- |
| `src/core/NavigationController.h/cpp` | 注释增强 | +25行 | -- | -- | -- |
| `src/core/BackgroundWidget.h/cpp` | 注释增强 | +15行 | -- | -- | -- |
| `src/core/Constants.h` | 注释增强 | +15行 | -- | -- | -- |
| `src/terminal/TerminalWidget.h/cpp` | 注释增强 | +75行 | -- | -- | -- |
| `src/terminal/TerminalModel.h/cpp` | 注释增强 | +20行 | -- | -- | -- |
| `src/terminal/TerminalSearchBar.h/cpp` | 注释增强 | +20行 | -- | -- | -- |
| `src/terminal/TerminalTypes.h` | 注释增强 | +5行 | -- | -- | -- |
| `src/serial/SerialConfigPanel.h/cpp` | 注释增强 | +25行 | -- | -- | -- |
| `src/serial/QuickCommandBar.h/cpp` | 注释增强 | +15行 | -- | -- | -- |
| `src/serial/SendHistory.h/cpp` | 注释增强 | +10行 | -- | -- | -- |
| `src/serial/DataStatistics.h/cpp` | 注释增强 | +15行 | -- | -- | -- |
| `src/serial/TimedSender.h/cpp` | 注释增强 | +10行 | -- | -- | -- |
| `src/serial/SerialDriverDetector.h/cpp` | 注释增强 | +10行 | -- | -- | -- |
| `src/connection/SerialConnection.h/cpp` | 注释增强 | +20行 | -- | -- | -- |
| `src/connection/IConnection.h` | 注释增强 | +10行 | -- | -- | -- |
| `src/utils/RingBuffer.h` | 注释增强 | +15行 | -- | -- | -- |
| `src/utils/SettingsManager.h/cpp` | 注释增强 | +15行 | -- | -- | -- |
| `src/utils/DataExporter.h/cpp` | 注释增强 | +20行 | -- | -- | -- |
| `src/utils/DataLogger.h/cpp` | 注释增强 | +25行 | -- | -- | -- |
| `src/utils/CRC.h` | 注释增强 | +10行 | -- | -- | -- |
| `src/utils/HexConverter.h` | 注释增强 | +10行 | -- | -- | -- |
| `resources/themes/dark_terminal.qss` | 新增layoutCombo样式 | -- | -- | +12行 | -- |
| `resources/themes/modern_dark.qss` | 新增layoutCombo样式 | -- | -- | +12行 | -- |
| `resources/themes/light.qss` | 新增layoutCombo样式 | -- | -- | +12行 | -- |
| `CLAUDE.md` | 新增4.7节+扩展5.1.1节 | -- | -- | -- | +50行 |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| R1 注释增强 | 约 570 行 | -- | -- |
| R2 PanelManager | 约 200 行 | 15 行 | 70 行 |
| R3 TerminalLayoutManager | 约 270 行 | 10 行 | -- |
| R3 QSS样式 | 约 36 行 | -- | -- |
| R4 CLAUDE.md | 约 50 行 | -- | -- |
| **合计** | **约 1126 行** | **约 25 行** | **约 70 行** |

### 跨模块影响评估

- **MainWindow -> PanelManager**: MainWindow 不再直接创建面板，通过 m_panelManager->createPanels() 委托创建。面板指针通过 m_panelManager->xxxPanel() getter 获取。
- **MainWindow -> TerminalLayoutManager**: MainWindow 不再直接持有 TerminalWidget 和 TerminalSearchBar，通过 m_terminalLayoutManager 获取。布局切换由 ToolbarController 的 layoutChanged 信号驱动。
- **PanelManager -> NavigationController**: PanelManager 提供 panelMap() 返回面板映射表，替代 MainWindow 中硬编码的映射列表。
- **TerminalLayoutManager -> TerminalWidget**: 分栏模式下创建两个 TerminalWidget 实例，分别调用 setDirectionFilter(Rx) 和 setDirectionFilter(Tx)。
- **QSS 主题文件**: 三个主题文件各新增 layoutCombo 和分栏标签样式，不影响现有样式。

---

## 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | core/ 下所有.h文件类注释包含@brief+职责列表+线程安全性+生命周期 | 代码检查 | 每个类都有完整注释块 |
| R1-AC2 | core/ 下所有公开方法有@brief和@param注释 | 代码检查 | 注释覆盖率100% |
| R1-AC3 | core/ 下所有成员变量有用途注释 | 代码检查 | 注释覆盖率100% |
| R1-AC4 | terminal/ 下所有类和方法注释完备 | 代码检查 | 同R1-AC1/AC2/AC3 |
| R1-AC5 | serial/ 下所有类和方法注释完备 | 代码检查 | 同上 |
| R1-AC6 | connection/ + utils/ 下所有类和方法注释完备 | 代码检查 | 同上 |
| R2-AC1 | PanelManager.h/cpp 文件存在 | 文件检查 | 存在且编译通过 |
| R2-AC2 | MainWindow.cpp 中不再有面板创建代码 | 代码检查 | setupUI()中无new SerialConfigPanel等 |
| R2-AC3 | MainWindow.cpp 行数 <= 450 | wc -l | 不超过450行 |
| R2-AC4 | 面板映射表由PanelManager提供 | 代码检查 | buildNavTree使用panelMap() |
| R2-AC5 | 所有功能回归: 面板切换正常 | 手动: 点击导航树 | 各面板正确显示/隐藏 |
| R3-AC1 | TerminalLayoutManager.h/cpp 文件存在 | 文件检查 | 存在且编译通过 |
| R3-AC2 | 工具栏有布局模式下拉框 | 手动: 查看工具栏 | 有"混合/左右分栏/上下分栏"选项 |
| R3-AC3 | 左右分栏模式显示两个终端 | 手动: 切换到左右分栏 | 左侧RX右侧TX，各有标签 |
| R3-AC4 | 上下分栏模式显示两个终端 | 手动: 切换到上下分栏 | 上方RX下方TX |
| R3-AC5 | 分栏模式下数据正确分流 | 手动: 发送数据后观察 | TX数据只在TX终端，RX数据只在RX终端 |
| R3-AC6 | 切回混合模式后数据完整 | 手动: 分栏->混合 | 所有数据按时间顺序显示 |
| R3-AC7 | 分栏模式下搜索联动 | 手动: Ctrl+F搜索 | 两个终端同步高亮 |
| R3-AC8 | 布局偏好持久化 | 手动: 切换分栏->关闭->重启 | 恢复为上次的布局模式 |
| R3-AC9 | 分栏分隔线可拖拽 | 手动: 拖拽分隔线 | 左右/上下比例可调整 |
| R4-AC1 | CLAUDE.md 新增4.7节 | 文件检查 | 包含"MainWindow嵌入式main哲学"完整内容 |
| R4-AC2 | CLAUDE.md 新增5.1.1节 | 文件检查 | 包含"注释详细度标准"完整内容 |
| R4-AC3 | 4.7节包含白名单和黑名单 | 文件检查 | 明确列出允许和禁止的事项 |
| R4-AC4 | 5.1.1节包含注释模板和密度要求 | 文件检查 | 有具体模板和百分比要求 |
| AC-1 | 编译零错误零警告 | cmake --build build | 0 error, 0 warning |
| AC-2 | EmbedDebug.bat 正常启动 | 双击 EmbedDebug.bat | 应用窗口正常显示 |
| AC-3 | 现有功能回归: 串口连接/断开 | 手动 | 连接状态正确切换 |
| AC-4 | 现有功能回归: 数据收发 | 手动 | TX/RX数据正确显示 |
| AC-5 | 现有功能回归: 搜索高亮(F3导航) | 手动 | 搜索功能正常 |
| AC-6 | 现有功能回归: 主题切换 | 手动 | 三主题正常切换 |
| AC-7 | 现有功能回归: DTR/RTS控制 | 手动 | 连接后可切换 |
| AC-8 | 现有功能回归: 数据导出 | 手动 | 导出功能正常 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 | 预估行数 |
|------|------|------|---------|
| 1 | R4: CLAUDE.md 约束文档更新 | 先确立规则，后续开发遵循 | +50 |
| 2 | R1-part1: core/ 目录注释增强 | 最高优先级模块，后续R2/R3依赖这些注释理解 | +200 |
| 3 | R1-part2: terminal/ 目录注释增强 | 终端模块注释增强 | +120 |
| 4 | R2: PanelManager 创建 + MainWindow 适配 | 面板管理提取，为R3做准备 | +200/-70 |
| 5 | R3: TerminalLayoutManager 创建 | 分栏功能核心实现 | +270 |
| 6 | R3-part2: ToolbarController layoutCombo + QSS | 分栏切换UI入口 | +50 |
| 7 | R1-part3: serial/ 目录注释增强 | 串口模块注释 | +75 |
| 8 | R1-part4: connection/ + utils/ 注释增强 | 基础设施层注释 | +175 |
| 9 | 编译验证 | 确保零错误 | -- |
| 10 | 全功能回归测试 | 验证所有功能正常 | -- |

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| MainWindow.cpp 行数 | wc -l | 497 行 | <= 450 行 |
| MainWindow.h 行数 | wc -l | 147 行 | <= 130 行(成员减少) |
| PanelManager.h 行数 | wc -l | (新增) | <= 90 行 |
| PanelManager.cpp 行数 | wc -l | (新增) | <= 130 行 |
| TerminalLayoutManager.h 行数 | wc -l | (新增) | <= 100 行 |
| TerminalLayoutManager.cpp 行数 | wc -l | (新增) | <= 200 行 |
| 新增文件数 | 文件检查 | -- | 4 个 (.h/.cpp 对) |
| 注释行数占比(core/) | 注释行/总行数 | ~15% | >= 25% |
| 注释行数占比(terminal/) | 注释行/总行数 | ~20% | >= 25% |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| MainWindow 私有方法数 | 代码检查 | setupUI/setupStatusBar/connectSignals/updateStatusBar/updateDataStatistics + 构造/析构/关闭 (<= 8个) |
| MainWindow 成员指针数 | 代码检查 | 从30+减少到15以下 |
| PanelManager 依赖方向 | 代码检查 | 仅依赖表现层组件和注入的控制器 |
| TerminalLayoutManager 依赖方向 | 代码检查 | 仅依赖 TerminalModel/TerminalWidget/TerminalSearchBar |
| 分层依赖方向 | 代码检查 | 无反向依赖 |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
| EmbedDebug.bat 启动 | 双击 bat | 正常启动 |
| 混合模式显示 | 手动 | TX/RX混合显示正确 |
| 左右分栏显示 | 手动 | 左RX右TX，标签清晰 |
| 上下分栏显示 | 手动 | 上RX下TX，标签清晰 |
| 分栏切换流畅 | 手动 | 切换无闪烁，面板正确显示/隐藏 |
| 搜索联动 | 手动 | 分栏模式下两个终端同步高亮 |
| 分隔线拖拽 | 手动 | 可调整比例，最小宽度保护 |
| 布局持久化 | 手动 | 关闭重启后恢复上次布局 |
| 三主题下分栏视觉 | 视觉检查 | 分栏标签、分隔线颜色跟随主题 |
