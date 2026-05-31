# PRD-030: PanelManager提取 + TerminalWidget方向过滤拆分 + 头文件行数治理

## 背景

commit #29 已完成注释增强、TerminalLayoutManager分栏显示、QuickCommandBar等功能，当前评分 31 分。

当前存在四个文件严重超出 CLAUDE.md 4.6 节规定的行数上限:

| 文件 | 当前行数 | 上限 | 超出量 | 风险等级 |
|------|---------|------|--------|---------|
| MainWindow.cpp | 637 | 500 | +137 | **严重** -- 违反"MainWindow嵌入式main哲学"，上帝对象膨胀趋势 |
| TerminalWidget.cpp | 713 | 500 | +213 | **严重** -- 单一职责被破坏，渲染/搜索/方向过滤混杂 |
| FrameDefinition.h | 262 | 200 | +62 | 中等 -- 内联方法定义过多 |
| MainWindow.h | 270 | 200 | +70 | 中等 -- 成员变量过度暴露 |

此外，用户反馈了三个UI bug（commit #29 已做初步修复），需要在本次迭代中验证效果:

1. 下拉箭头样式 -- 已修复样式，需确认暗色主题下三角箭头清晰可见
2. 连接后面板闪烁 -- 已缩短动画到150ms，需评估是否仍有明显延迟
3. 背景图初始显示 -- 已加WA_StyledBackground，需确认启动时背景图完整覆盖

**审查基准**: commit #29, score 31。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 提取PanelManager -- 将MainWindow::setupUI()中的面板创建代码(约60行)提取到独立类 | P0 | core/MainWindow, core/PanelManager(新增) |
| R2 | TerminalWidget方向过滤拆分 -- 将方向过滤逻辑提取到DirectionFilter类 | P0 | terminal/TerminalWidget, terminal/DirectionFilter(新增) |
| R3 | FrameDefinition拆分 -- 将内联方法定义移到.cpp文件 | P1 | protocol/FrameDefinition |
| R4 | MainWindow.h精简 -- 减少成员变量暴露，使用前置声明替代include | P1 | core/MainWindow.h |

---

## 需求详细说明

---

### R1: 提取PanelManager (P0)

#### 问题分析

MainWindow.cpp 当前 637 行，其中 setupUI() 方法(第133-244行)包含约 60 行纯面板创建代码:

- 第179-201行: 创建 SerialConfigPanel / DataStatistics / ProtocolView / FrameVisualEditor / ChartWidget / OtaWidget，设置不可见，添加到布局
- 第205-213行: 创建 TerminalWidget / TerminalSearchBar / TerminalLayoutManager
- 第217-223行: 创建 QuickCommandBar 并设置默认指令
- 第226-227行: 创建 SendBar

这段代码的职责是"创建面板并堆叠到布局中"，与 MainWindow 的中介者角色无关。按照 CLAUDE.md 5.1.1 节"MainWindow嵌入式main哲学"，面板创建应委托给专门的类。

此外，MainWindow.h 中有 11 个面板相关的成员指针(m_serialConfig, m_terminal, m_searchBar, m_quickCmdBar, m_dataStats, m_protocolView, m_frameParser, m_protocolBridgeMgr, m_frameEditor, m_chartWidget, m_otaWidget)，它们的存在是为了让 connectSignals() 能够连接信号。这些指针应该由 PanelManager 持有，MainWindow 通过 getter 获取。

#### 方案设计

##### 新增类: PanelManager

**职责**: 集中管理所有功能面板的创建、生命周期和指针访问。

```
文件: src/core/PanelManager.h / src/core/PanelManager.cpp
层级: 表现层 (QObject)
父对象: MainWindow
```

PanelManager 不创建布局骨架(BackgroundWidget/QSplitter/QTreeView/右侧容器)，这些仍由 MainWindow 的 setupUI() 负责。PanelManager 只负责在给定的父容器中创建面板 widget 并管理指针。

```cpp
/**
 * @brief 面板管理器 - 集中管理所有功能面板的创建和指针访问
 *
 * 详细职责:
 *   1. 创建所有功能面板(SerialConfigPanel, DataStatistics, ProtocolView等)
 *   2. 创建终端子系统(TerminalWidget, TerminalSearchBar, TerminalLayoutManager)
 *   3. 创建辅助面板(QuickCommandBar, SendBar)
 *   4. 管理面板的初始可见性(默认全部隐藏，由NavigationController控制显示)
 *   5. 提供面板指针的getter接口
 *   6. 提供面板映射表供NavigationController构建导航树
 *
 * 线程安全性: 否，所有方法必须在主线程(GUI线程)调用
 * 生命周期: 由MainWindow在构造函数中创建，作为MainWindow的子QObject，
 *           随MainWindow销毁而自动销毁
 *
 * 协作关系:
 *   - MainWindow: 创建PanelManager，通过getter获取面板指针进行信号连接
 *   - NavigationController: 通过panelMap()获取面板名称-指针映射构建导航树
 *   - SendController: PanelManager调用其createSendBar()创建发送区域
 *   - OtaManager: 传入OtaWidget构造
 */
class PanelManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造面板管理器
     *
     * @param sendController 发送控制器，用于创建SendBar
     * @param otaManager OTA管理器，用于创建OtaWidget
     * @param parent 父对象，通常为MainWindow
     */
    explicit PanelManager(SendController* sendController,
                          OtaManager* otaManager,
                          QObject* parent = nullptr);

    ~PanelManager() override;

    /**
     * @brief 创建所有面板并堆叠到指定容器
     *
     * 在container的布局中按顺序添加:
     *   SerialConfigPanel, DataStatistics, ProtocolView, FrameVisualEditor,
     *   ChartWidget, OtaWidget, 终端容器(由TerminalLayoutManager管理),
     *   QuickCommandBar, SendBar。
     * 除终端容器外，所有面板初始设为不可见。
     *
     * @param container 面板堆叠的目标容器widget，必须有QVBoxLayout
     * @param terminalModel 终端数据模型，用于TerminalWidget::setModel()
     */
    void createPanels(QWidget* container, TerminalModel* terminalModel);

    /**
     * @brief 获取面板映射表，供NavigationController构建导航树
     *
     * 返回有序的(面板名称, QWidget指针对)列表，顺序决定导航树中的显示顺序。
     * 面板名称使用QT_TRANSLATE_NOOP宏包裹，支持多语言翻译。
     *
     * @return 面板映射表: {翻译键, QWidget指针}
     */
    QVector<QPair<QString, QWidget*>> panelMap() const;

    // ---- 各面板的getter接口 ----
    // MainWindow通过这些getter获取面板指针，用于connectSignals()中的信号连接

    SerialConfigPanel* serialConfigPanel() const;
    TerminalWidget* terminalWidget() const;
    TerminalSearchBar* searchBar() const;
    QuickCommandBar* quickCommandBar() const;
    DataStatistics* dataStatistics() const;
    ProtocolView* protocolView() const;
    FrameVisualEditor* frameVisualEditor() const;
    ChartWidget* chartWidget() const;
    OtaWidget* otaWidget() const;
    FrameParser* frameParser() const;
    ProtocolBridgeManager* protocolBridgeManager() const;
    TerminalLayoutManager* layoutManager() const;

private:
    // ---- 面板指针 -- PanelManager拥有所有权，面板作为本对象的子widget ----

    SerialConfigPanel* m_serialConfig;
    DataStatistics* m_dataStats;
    ProtocolView* m_protocolView;
    FrameVisualEditor* m_frameEditor;
    ChartWidget* m_chartWidget;
    OtaWidget* m_otaWidget;
    TerminalWidget* m_terminal;
    TerminalSearchBar* m_searchBar;
    QuickCommandBar* m_quickCmdBar;
    TerminalLayoutManager* m_layoutManager;

    // ---- 业务对象 -- PanelManager拥有所有权 ----

    FrameParser* m_frameParser;
    ProtocolBridgeManager* m_protocolBridgeMgr;

    // ---- 外部依赖（不拥有所有权，由MainWindow注入） ----

    SendController* m_sendController;   ///< 发送控制器，用于创建SendBar
    OtaManager* m_otaManager;           ///< OTA管理器，用于创建OtaWidget
};
```

##### MainWindow 适配

**构造函数变化**:

PanelManager 的创建在 MainWindow 构造函数初始化列表中完成。FrameParser 和 ProtocolBridgeManager 从 MainWindow 转移到 PanelManager。

```cpp
// MainWindow 构造函数中:
// 删除: m_frameParser(new FrameParser(this))
// 删除: m_protocolBridgeMgr(new ProtocolBridgeManager(m_frameParser, this))
// 新增:
m_panelManager = new PanelManager(m_sendController, m_otaManager, this);
```

**setupUI() 变化**:

```cpp
void MainWindow::setupUI()
{
    // 1. 背景层 (不变)
    m_backgroundWidget = new BackgroundWidget(this);
    setCentralWidget(m_backgroundWidget);
    // ... 布局设置 ...

    // 2. 分割器 (不变)
    m_mainSplitter = new QSplitter(Qt::Horizontal, m_backgroundWidget);

    // 3. 左侧导航树 (不变)
    m_navTree = new QTreeView;
    // ... 树属性设置 ...
    m_mainSplitter->addWidget(m_navTree);

    // 4. 右侧容器骨架 (不变)
    // ... 创建 rightWidget / rightLayout / m_rightPanel / rightPanelLayout ...

    // 5. 面板栈容器 (简化 -- 只创建serialPanel壳子)
    auto* serialPanel = new QWidget;
    serialPanel->setObjectName("serialPanel");
    serialPanel->setAttribute(Qt::WA_StyledBackground, true);
    auto* serialLayout = new QVBoxLayout(serialPanel);
    serialLayout->setContentsMargins(0, 0, 0, 0);
    serialLayout->setSpacing(0);

    // 6. 委托PanelManager创建所有面板并填入serialPanel
    m_panelManager->createPanels(serialPanel, m_terminalModel);

    rightPanelLayout->addWidget(serialPanel);

    // 7. 分割器设置 + Ctrl+F (不变)
    m_mainSplitter->addWidget(rightWidget);
    m_mainSplitter->setSizes({200, 1000});
    // ...
}
```

**connectSignals() 变化**:

所有面板指针访问改为通过 m_panelManager->xxxPanel():

```cpp
// 之前:
connect(m_serialConfig, &SerialConfigPanel::connectRequested, ...);
connect(m_protocolView, &ProtocolView::onFrameParsed, ...);

// 之后:
connect(m_panelManager->serialConfigPanel(), &SerialConfigPanel::connectRequested, ...);
connect(m_panelManager->protocolView(), &ProtocolView::onFrameParsed, ...);
```

为了减少 connectSignals() 中反复调用 getter 的冗余，可在 connectSignals() 开头缓存指针到局部变量:

```cpp
void MainWindow::connectSignals()
{
    auto* serialConfig = m_panelManager->serialConfigPanel();
    auto* terminal = m_panelManager->terminalWidget();
    auto* protocolView = m_panelManager->protocolView();
    // ... 使用局部变量连接信号 ...
}
```

**MainWindow.h 成员变量变化**:

删除 11 个面板成员指针，新增 1 个 PanelManager 指针:

```
删除: m_serialConfig, m_terminal, m_searchBar, m_quickCmdBar, m_dataStats,
      m_protocolView, m_frameParser, m_protocolBridgeMgr, m_frameEditor,
      m_chartWidget, m_otaWidget, m_layoutManager
新增: PanelManager* m_panelManager
```

**预计效果**:
- MainWindow.cpp: 637 - 60(面板创建) + 15(getter缓存) = 约 592 行 (仍超限，需R4进一步精简)
- MainWindow.h: 270 - 70(成员+include) + 10(PanelManager) = 约 210 行

---

### R2: TerminalWidget方向过滤拆分 (P0)

#### 问题分析

TerminalWidget.cpp 当前 713 行，其中方向过滤相关逻辑分布在多个方法中:

1. **成员变量** (TerminalWidget.h 第139-142行):
   - `m_directionFiltered` -- 是否启用方向过滤
   - `m_filterDirection` -- 过滤方向
   - `m_filteredIndices` -- 过滤后的模型行号索引表

2. **setDirectionFilter()** (第59-68行, 10行) -- 启用方向过滤并重置缓存

3. **clearDirectionFilter()** (第70-78行, 9行) -- 清除方向过滤

4. **paintEvent() 过滤分支** (第283-334行, 约52行) -- 方向过滤模式下的增量缓存构建和绘制

5. **selectedText() 过滤分支** (第147-159行, 13行) -- 过滤模式下的文本选择

6. **setSearchHighlight() 过滤分支** (第523-567行, 约45行) -- 过滤模式下的搜索匹配

7. **updateVisibleRange() 过滤分支** (第496-498行, 3行) -- 过滤模式下的滚动范围计算

8. **onDataAppended() 过滤分支** (第464-466行, 3行) -- 过滤模式下跳过滚动偏移更新

9. **onDataCleared()** (第484行) -- 清除过滤索引

10. **setModel()** (第55行) -- 清除过滤索引

方向过滤相关代码总计约 135 行，分散在 10 个方法中。这种散布式设计使得过滤逻辑与渲染逻辑交织，违反单一职责原则。

#### 方案设计

##### 新增类: DirectionFilter

**职责**: 封装终端方向过滤的状态和逻辑，作为TerminalWidget的组合成员。

```
文件: src/terminal/DirectionFilter.h / src/terminal/DirectionFilter.cpp
层级: 数据层 (纯C++类，非QObject)
```

```cpp
/**
 * @brief 方向过滤器 - 管理终端数据的方向过滤状态和索引
 *
 * 详细职责:
 *   1. 维护过滤开关状态(m_enabled)和过滤方向(m_direction)
 *   2. 维护过滤索引表 m_filteredIndices: 显示行号 → 模型行号的映射
 *   3. 提供增量索引构建接口: 当新数据到达时只处理新增行
 *   4. 提供索引查询接口: 获取过滤后总行数、根据显示行号获取模型行号
 *   5. 提供环形缓冲区回绕检测: 模型行数减少时自动触发全量重建
 *
 * 设计模式: 值对象(非QObject)，由TerminalWidget按值持有作为组合成员。
 * 不使用QObject以避免不必要的信号/槽开销。
 *
 * 线程安全性: 否，由TerminalWidget保证调用线程安全
 * 生命周期: 随TerminalWidget创建和销毁，无需手动管理
 */
class DirectionFilter {
public:
    DirectionFilter() = default;

    /**
     * @brief 启用方向过滤
     *
     * 设置过滤方向并清空所有现有索引，强制下次paintEvent全量重建。
     *
     * @param direction 过滤方向: Rx=只显示接收数据, Tx=只显示发送数据
     */
    void setFilter(DataDirection direction);

    /**
     * @brief 清除方向过滤
     *
     * 重置过滤状态，清空索引表。之后TerminalWidget恢复显示所有方向数据。
     */
    void clearFilter();

    /** @return 是否启用了方向过滤 */
    bool isEnabled() const;

    /** @return 当前过滤方向(仅当isEnabled()为true时有效) */
    DataDirection direction() const;

    /**
     * @brief 增量更新过滤索引
     *
     * 根据缓存行信息(CachedLine)更新过滤索引表。
     * 只处理从 cachedLineCount 开始的新增行。
     * 支持环形缓冲区回绕检测: 当 modelLineCount < cachedLineCount 时触发全量重建。
     *
     * @param cachedLines 缓存行数组
     * @param cachedLineCount 已缓存的行数（上次处理的行数）
     * @param modelLineCount 模型当前总行数
     * @return 更新后的 cachedLineCount（调用方需保存）
     */
    int rebuildIndices(const QVector<CachedLine>& cachedLines,
                       int cachedLineCount,
                       int modelLineCount);

    /**
     * @brief 获取过滤后的总行数
     *
     * @return 过滤索引表中的行数
     */
    int filteredLineCount() const;

    /**
     * @brief 根据显示行号获取模型行号
     *
     * @param displayLine 过滤后的显示行号 (0-based)
     * @return 对应的模型行号，越界时返回-1
     */
    int modelLineIndex(int displayLine) const;

    /**
     * @brief 获取完整的过滤索引表
     *
     * 用于搜索功能中需要遍历所有过滤后行的场景。
     *
     * @return 过滤索引表的const引用
     */
    const QVector<int>& filteredIndices() const;

    /**
     * @brief 清空所有索引（用于数据清除/模型重置场景）
     */
    void clearIndices();

private:
    bool m_enabled = false;                     ///< 是否启用方向过滤
    DataDirection m_direction = DataDirection::Rx; ///< 过滤方向(仅当m_enabled为true时有效)
    QVector<int> m_filteredIndices;             ///< 过滤索引表: 显示行号 → 模型行号
};
```

##### TerminalWidget 适配

TerminalWidget 将 DirectionFilter 作为组合成员持有:

```cpp
// TerminalWidget.h 新增:
#include "DirectionFilter.h"

// private成员:
DirectionFilter m_directionFilter;  ///< 方向过滤器 -- 管理方向过滤状态和索引
```

**方法变更对照**:

| 原方法 | 变更 | 说明 |
|--------|------|------|
| setDirectionFilter() | 改为调用 m_directionFilter.setFilter() + 清缓存 + update() | 逻辑不变 |
| clearDirectionFilter() | 改为调用 m_directionFilter.clearFilter() + 清缓存 + update() | 逻辑不变 |
| paintEvent() 过滤分支 | 改为调用 m_directionFilter.rebuildIndices() | 索引构建逻辑提取 |
| selectedText() 过滤分支 | 改为调用 m_directionFilter.filteredIndices() | 索引查询不变 |
| setSearchHighlight() 过滤分支 | 改为调用 m_directionFilter 相关接口 | 搜索逻辑不变 |
| updateVisibleRange() 过滤分支 | 改为调用 m_directionFilter.filteredLineCount() | 计算逻辑不变 |
| onDataAppended() 过滤分支 | 改为调用 m_directionFilter.isEnabled() | 判断逻辑不变 |
| onDataCleared() | 改为调用 m_directionFilter.clearIndices() | 清除逻辑不变 |
| setModel() | 改为调用 m_directionFilter.clearIndices() | 清除逻辑不变 |

TerminalWidget 删除 3 个成员变量:
```
删除: m_directionFiltered, m_filterDirection, m_filteredIndices
新增: DirectionFilter m_directionFilter (组合成员)
```

**预计效果**:
- TerminalWidget.h: 149 - 3(成员) - 4(方法声明简化) = 约 145 行
- TerminalWidget.cpp: 713 - 100(过滤逻辑内聚) + 30(DirectionFilter调用) = 约 643 行

TerminalWidget.cpp 仍超 500 行限制，但方向过滤逻辑已完成内聚化，后续可继续提取搜索逻辑(SearchHighlighter)来进一步降低行数。本次迭代优先解决最紧迫的架构问题。

---

### R3: FrameDefinition拆分 (P1)

#### 问题分析

FrameDefinition.h 当前 262 行，其中包含两个结构体的内联方法定义:

1. **FieldDef::extractValue()** -- 约 63 行，从字节流中提取并转换字段值
2. **FieldDef::formatValue()** -- 约 27 行，格式化显示值
3. **FieldDef::toJson()** -- 约 13 行，JSON序列化
4. **FieldDef::fromJson()** -- 约 12 行，JSON反序列化
5. **FrameDefinition::maxFrameLength()** -- 约 8 行，最大帧长度计算
6. **FrameDefinition::toJson()** -- 约 28 行，JSON序列化
7. **FrameDefinition::fromJson()** -- 约 30 行，JSON反序列化

这 7 个方法总计约 181 行内联代码，加上结构体声明和注释约 80 行，总计 262 行。

根据 CLAUDE.md 4.6 节，头文件行数上限 200 行。将内联方法移到 .cpp 文件可以显著降低头文件体积，同时保持接口不变。

#### 方案设计

##### 文件拆分

**FrameDefinition.h** (目标约 80 行):

- 保留: include、struct声明、成员变量声明、方法声明(不含函数体)
- 移除: 所有方法的函数体实现
- 注意: extractValue() 和 formatValue() 在帧解析高频调用路径上，但它们不是 const 内联热点(每次解析只调用有限次)，移到 .cpp 不影响性能

**FrameDefinition.cpp** (新增，约 180 行):

- 包含: 所有方法的实现代码
- 包含: 需要的 Qt JSON 头文件 (QJsonDocument, QJsonObject, QJsonArray)
- 头文件中可移除这些 include，改为前置声明或移到 .cpp

##### 迁移方法清单

| 方法 | 原位置 | 迁移到 | 行数 |
|------|--------|--------|------|
| FieldDef::extractValue() | .h 第34-97行 | .cpp | 63 |
| FieldDef::formatValue() | .h 第100-127行 | .cpp | 27 |
| FieldDef::toJson() | .h 第130-141行 | .cpp | 12 |
| FieldDef::fromJson() | .h 第143-155行 | .cpp | 12 |
| FrameDefinition::maxFrameLength() | .h 第184-193行 | .cpp | 8 |
| FrameDefinition::toJson() | .h 第196-226行 | .cpp | 28 |
| FrameDefinition::fromJson() | .h 第228-259行 | .cpp | 30 |

##### 头文件精简后结构

```cpp
#ifndef FRAMEDEFINITION_H
#define FRAMEDEFINITION_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QVariant>

class QJsonObject;      // 前置声明替代 #include <QJsonObject>
class QJsonArray;        // 前置声明替代 #include <QJsonArray>

// 单个字段定义
struct FieldDef {
    QString name;
    int offset = 0;
    int size = 1;
    double scale = 1.0;
    double offsetVal = 0.0;
    QString unit;

    enum Type { UInt8, UInt16LE, UInt16BE, UInt32LE, UInt32BE,
                Int8, Int16LE, Int16BE, Float, Raw };
    Type type = UInt8;

    // 方法声明 -- 实现移到 FrameDefinition.cpp
    QVariant extractValue(const QByteArray& payload) const;
    QString formatValue(const QByteArray& payload) const;
    QJsonObject toJson() const;
    static FieldDef fromJson(const QJsonObject& obj);
};

enum class ChecksumType { None, Sum8, CRC8, CRC16CCITT, CRC16Modbus, CRC32 };

// 完整帧格式定义
struct FrameDefinition {
    QByteArray header;
    QByteArray footer;
    int lengthFieldOffset = -1;
    int lengthFieldSize = 1;
    bool lengthBigEndian = false;
    int lengthAdjust = 0;
    int checksumOffset = -1;
    ChecksumType checksumType = ChecksumType::None;
    int checksumSize = 0;
    int checksumStart = 0;
    int checksumEnd = -1;
    QVector<FieldDef> fields;

    // 方法声明 -- 实现移到 FrameDefinition.cpp
    int maxFrameLength() const;
    QJsonObject toJson() const;
    static FrameDefinition fromJson(const QJsonObject& obj);
};

#endif // FRAMEDEFINITION_H
```

**预计效果**: FrameDefinition.h 从 262 行降至约 55 行（远低于 200 行上限）。

---

### R4: MainWindow.h精简 (P1)

#### 问题分析

MainWindow.h 当前 270 行，超出 200 行上限。主要膨胀来源:

1. **include 指令** (第4-38行): 35 行 include，其中大部分可以替换为前置声明
2. **成员变量** (第140-268行): 约 30 个成员指针，其中 11 个面板指针将在 R1 中转移到 PanelManager

R1 完成后，MainWindow.h 将删除 11 个面板成员及其注释(约 70 行)，新增 PanelManager 指针(约 5 行)，净减少约 65 行，降至约 205 行。仍略超上限。

进一步精简策略:

1. **前置声明替代 include**: 将 PanelManager、TerminalLayoutManager 等类型的 include 替换为前置声明。只要 MainWindow.h 中只使用指针类型而不需要知道类的完整定义，就可以使用前置声明。
2. **合并注释**: 将每个成员变量的一行式 @brief 注释压缩为行内 ///< 注释，减少注释占用行数。

#### 可替换的前置声明

| 原 include | 替换为 | 条件 |
|-----------|--------|------|
| `terminal/TerminalWidget.h` | `class TerminalWidget;` | 只使用指针 |
| `terminal/TerminalModel.h` | `class TerminalModel;` | 只使用指针 |
| `serial/SerialConfigPanel.h` | R1删除 | 转移到PanelManager |
| `serial/QuickCommandBar.h` | R1删除 | 转移到PanelManager |
| `serial/SendHistory.h` | `class SendHistory;` | 只使用指针 |
| `serial/DataStatistics.h` | R1删除 | 转移到PanelManager |
| `utils/DataExporter.h` | `class DataExporter;` | 只使用指针 |
| `utils/DataLogger.h` | `class DataLogger;` | 只使用指针 |
| `core/ConnectionController.h` | `class ConnectionController;` | 只使用指针 |
| `core/RecordingController.h` | `class RecordingController;` | 只使用指针 |
| `core/SendController.h` | `class SendController;` | 只使用指针 |
| `core/ToolbarController.h` | `class ToolbarController;` | 只使用指针 |
| `core/SettingsController.h` | `class SettingsController;` | 只使用指针 |
| `core/BackgroundWidget.h` | `class BackgroundWidget;` | 只使用指针 |
| `core/BackgroundSettingsPopup.h` | `class BackgroundSettingsPopup;` | 只使用指针 |
| `utils/SettingsManager.h` | 删除(R1中PanelManager使用) | 不再直接使用 |
| `terminal/TerminalSearchBar.h` | R1删除 | 转移到PanelManager |
| `terminal/TerminalLayoutManager.h` | R1删除 | 转移到PanelManager |
| `protocol/FrameParser.h` | R1删除 | 转移到PanelManager |
| `protocol/ProtocolView.h` | R1删除 | 转移到PanelManager |
| `protocol/FrameVisualEditor.h` | R1删除 | 转移到PanelManager |
| `protocol/ProtocolBridgeManager.h` | R1删除 | 转移到PanelManager |
| `chart/ChartWidget.h` | R1删除 | 转移到PanelManager |
| `ota/OtaManager.h` | `class OtaManager;` | 只使用指针 |
| `ota/OtaWidget.h` | R1删除 | 转移到PanelManager |

R1 + R4 后，MainWindow.h 只需保留以下 include:

```cpp
#include <QMainWindow>
#include <QTreeView>        // setupUI()中创建QTreeView需要完整类型
#include <QSplitter>        // setupUI()中创建QSplitter需要完整类型
#include <QStatusBar>       // setupStatusBar()需要
#include <QLabel>           // setupStatusBar()中创建QLabel需要
#include <QTimer>           // m_statsTimer类型需要
#include <functional>       // 导出回调使用
#include "ConnectionManager.h"  // 构造函数中new ConnectionManager需要完整类型
#include "ThemeManager.h"       // 可选，如只使用指针可前置声明
```

其余全部替换为前置声明，在 MainWindow.cpp 中 include 完整头文件。

**预计效果**: MainWindow.h 从 270 行降至约 140 行（含注释）。

---

## 接口设计

### 新增接口

#### PanelManager

| 接口 | 文件 | 签名 | 说明 |
|------|------|------|------|
| PanelManager() | core/PanelManager.h | `explicit PanelManager(SendController*, OtaManager*, QObject*)` | 构造函数 |
| ~PanelManager() | core/PanelManager.h | `~PanelManager() override` | 析构函数 |
| createPanels() | core/PanelManager.h | `void createPanels(QWidget* container, TerminalModel* model)` | 创建所有面板 |
| panelMap() | core/PanelManager.h | `QVector<QPair<QString, QWidget*>> panelMap() const` | 面板映射表 |
| serialConfigPanel() | core/PanelManager.h | `SerialConfigPanel* serialConfigPanel() const` | getter |
| terminalWidget() | core/PanelManager.h | `TerminalWidget* terminalWidget() const` | getter |
| searchBar() | core/PanelManager.h | `TerminalSearchBar* searchBar() const` | getter |
| quickCommandBar() | core/PanelManager.h | `QuickCommandBar* quickCommandBar() const` | getter |
| dataStatistics() | core/PanelManager.h | `DataStatistics* dataStatistics() const` | getter |
| protocolView() | core/PanelManager.h | `ProtocolView* protocolView() const` | getter |
| frameVisualEditor() | core/PanelManager.h | `FrameVisualEditor* frameVisualEditor() const` | getter |
| chartWidget() | core/PanelManager.h | `ChartWidget* chartWidget() const` | getter |
| otaWidget() | core/PanelManager.h | `OtaWidget* otaWidget() const` | getter |
| frameParser() | core/PanelManager.h | `FrameParser* frameParser() const` | getter |
| protocolBridgeManager() | core/PanelManager.h | `ProtocolBridgeManager* protocolBridgeManager() const` | getter |
| layoutManager() | core/PanelManager.h | `TerminalLayoutManager* layoutManager() const` | getter |

#### DirectionFilter

| 接口 | 文件 | 签名 | 说明 |
|------|------|------|------|
| setFilter() | terminal/DirectionFilter.h | `void setFilter(DataDirection direction)` | 启用方向过滤 |
| clearFilter() | terminal/DirectionFilter.h | `void clearFilter()` | 清除方向过滤 |
| isEnabled() | terminal/DirectionFilter.h | `bool isEnabled() const` | 查询过滤状态 |
| direction() | terminal/DirectionFilter.h | `DataDirection direction() const` | 查询过滤方向 |
| rebuildIndices() | terminal/DirectionFilter.h | `int rebuildIndices(const QVector<CachedLine>&, int cachedLineCount, int modelLineCount)` | 增量重建索引 |
| filteredLineCount() | terminal/DirectionFilter.h | `int filteredLineCount() const` | 过滤后行数 |
| modelLineIndex() | terminal/DirectionFilter.h | `int modelLineIndex(int displayLine) const` | 显示行号→模型行号 |
| filteredIndices() | terminal/DirectionFilter.h | `const QVector<int>& filteredIndices() const` | 索引表引用 |
| clearIndices() | terminal/DirectionFilter.h | `void clearIndices()` | 清空索引 |

#### FrameDefinition.cpp

无新增接口，仅将现有内联方法实现从头文件移出。

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| MainWindow::setupUI() | 面板创建委托给PanelManager | 外部行为不变，内部实现简化 |
| MainWindow::connectSignals() | 面板指针通过PanelManager getter获取 | 外部行为不变 |
| MainWindow 成员变量 | 删除11个面板指针，新增PanelManager指针 | 对外接口不变 |
| MainWindow.h include列表 | 大部分include替换为前置声明 | 编译依赖减少，不影响功能 |
| TerminalWidget::setDirectionFilter() | 实现改为委托DirectionFilter | 对外接口不变 |
| TerminalWidget::clearDirectionFilter() | 实现改为委托DirectionFilter | 对外接口不变 |
| TerminalWidget 内部方法 | 过滤分支改为使用DirectionFilter接口 | 内部重构，对外接口不变 |
| FrameDefinition.h 方法 | 声明保留，实现移到.cpp | 对外接口不变 |
| NavigationController::buildNavTree() | 面板映射表来源改为PanelManager::panelMap() | 行为不变 |

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| SerialConfigPanel | serial/SerialConfigPanel.h | PanelManager创建并持有 | R1 |
| TerminalWidget | terminal/TerminalWidget.h | PanelManager创建并持有 | R1 |
| TerminalSearchBar | terminal/TerminalSearchBar.h | PanelManager创建并持有 | R1 |
| QuickCommandBar | serial/QuickCommandBar.h | PanelManager创建并持有 | R1 |
| DataStatistics | serial/DataStatistics.h | PanelManager创建并持有 | R1 |
| ProtocolView | protocol/ProtocolView.h | PanelManager创建并持有 | R1 |
| FrameVisualEditor | protocol/FrameVisualEditor.h | PanelManager创建并持有 | R1 |
| ChartWidget | chart/ChartWidget.h | PanelManager创建并持有 | R1 |
| OtaWidget | ota/OtaWidget.h | PanelManager创建并持有 | R1 |
| TerminalLayoutManager | terminal/TerminalLayoutManager.h | PanelManager创建并持有 | R1 |
| SendController | core/SendController.h | PanelManager调用createSendBar() | R1 |
| OtaManager | ota/OtaManager.h | PanelManager创建OtaWidget时注入 | R1 |
| FrameParser | protocol/FrameParser.h | PanelManager创建并持有 | R1 |
| ProtocolBridgeManager | protocol/ProtocolBridgeManager.h | PanelManager创建并持有 | R1 |
| NavigationController | core/NavigationController.h | 使用panelMap()构建导航树 | R1 |
| TerminalModel | terminal/TerminalModel.h | PanelManager传入TerminalWidget | R1 |
| CachedLine | terminal/TerminalWidget.h | DirectionFilter引用缓存行结构 | R2 |
| Constants::DataDirection | core/Constants.h | DirectionFilter使用方向枚举 | R2 |
| CRC | utils/CRC.h | FrameDefinition.cpp中校验计算可能引用 | R3 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **外观模式 (Facade)** | PanelManager封装所有面板的创建和管理 | R1 | 对外暴露简单的createPanels()和getter接口，隐藏面板创建和布局细节 |
| **组合模式 (Composition)** | DirectionFilter作为TerminalWidget的值成员 | R2 | TerminalWidget通过组合DirectionFilter获得方向过滤能力，而非继承 |
| **值对象模式 (Value Object)** | DirectionFilter是非QObject的纯C++类 | R2 | 避免QObject开销，按值持有，随TerminalWidget生命周期管理 |
| **Pimpl惯用法 (变体)** | MainWindow.h使用前置声明隐藏实现细节 | R4 | 减少头文件编译依赖，缩短编译时间 |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 |
|------|---------|-----|-----|-----|-----|
| `src/core/MainWindow.h` | 成员减少+前置声明 | +5/-75行 | -- | -- | +5/-30行 |
| `src/core/MainWindow.cpp` | 面板委托+getter缓存 | +15/-60行 | -- | -- | -- |
| `src/core/PanelManager.h` | **新增** | +90行 | -- | -- | -- |
| `src/core/PanelManager.cpp` | **新增** | +130行 | -- | -- | -- |
| `src/terminal/DirectionFilter.h` | **新增** | -- | +80行 | -- | -- |
| `src/terminal/DirectionFilter.cpp` | **新增** | -- | +90行 | -- | -- |
| `src/terminal/TerminalWidget.h` | 成员减少+组合DirectionFilter | -- | +2/-4行 | -- | -- |
| `src/terminal/TerminalWidget.cpp` | 过滤逻辑委托DirectionFilter | -- | +30/-100行 | -- | -- |
| `src/protocol/FrameDefinition.h` | 内联方法移出 | -- | -- | -180行 | -- |
| `src/protocol/FrameDefinition.cpp` | **新增** | -- | -- | +180行 | -- |
| `CMakeLists.txt` | 新增文件注册 | +2行 | +2行 | +1行 | -- |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| R1 PanelManager | +230 行 | +20 行 | -135 行 |
| R2 DirectionFilter | +170 行 | +35 行 | -104 行 |
| R3 FrameDefinition | +180 行 | +5 行 | -180 行 |
| R4 MainWindow.h精简 | +5 行 | +5 行 | -30 行 |
| CMakeLists.txt | +5 行 | -- | -- |
| **合计** | **约 590 行** | **约 65 行** | **约 449 行** |

净变更约 206 行新增代码（不含删除），满足 CLAUDE.md commit 300 行变更要求（总变更 590+65+449 = 1104 行）。

### 跨模块影响评估

- **MainWindow -> PanelManager**: MainWindow 不再直接创建面板，通过 m_panelManager->createPanels() 委托。所有面板指针通过 m_panelManager->xxxPanel() getter 获取。
- **PanelManager -> NavigationController**: PanelManager 提供 panelMap() 替代 MainWindow 中硬编码的映射列表。
- **TerminalWidget -> DirectionFilter**: TerminalWidget 不再直接管理过滤状态和索引，委托给 m_directionFilter 组合成员。
- **FrameDefinition.h -> FrameDefinition.cpp**: 纯文件拆分，对 FrameParser、FrameVisualEditor、ProtocolView 等使用方零影响（它们只需 include 头文件，链接器自动解析 .cpp 实现）。
- **MainWindow.h 前置声明**: 所有被替换为前置声明的头文件将在 MainWindow.cpp 中 include，确保编译时完整类型可用。对其他 include MainWindow.h 的文件无影响（它们本来不需要通过 MainWindow 知道面板类型）。

### 构建系统影响

CMakeLists.txt 需注册三个新文件:

```cmake
# core/
src/core/PanelManager.h
src/core/PanelManager.cpp

# terminal/
src/terminal/DirectionFilter.h
src/terminal/DirectionFilter.cpp

# protocol/
src/protocol/FrameDefinition.cpp   # 之前只有.h
```

---

## 验收标准

### 功能验收

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| AC-1 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| AC-2 | EmbedDebug.bat正常启动 | 双击bat文件 | 应用窗口正常显示 |
| AC-3 | 串口连接/断开功能回归 | 手动连接+断开 | 状态切换正确，面板切换到终端 |
| AC-4 | 数据收发功能回归 | 手动发送数据 | TX/RX数据正确显示在终端 |
| AC-5 | 面板切换功能回归 | 手动点击导航树 | 各面板正确显示/隐藏 |
| AC-6 | 搜索高亮功能回归 | Ctrl+F搜索+F3导航 | 搜索匹配高亮正确 |
| AC-7 | 分栏模式功能回归 | 切换左右/上下分栏 | RX/TX终端正确分流数据 |
| AC-8 | 主题切换功能回归 | 切换暗色/亮色/现代暗色 | 三主题正常切换 |
| AC-9 | 数据导出功能回归 | 导出txt/csv/bin | 文件内容正确 |
| AC-10 | 帧编辑器功能回归 | 编辑帧定义+解析 | 帧解析结果正确 |
| AC-11 | OTA升级功能回归 | 模拟OTA流程 | 进度显示正常 |
| AC-12 | 下拉箭头可见性确认 | 暗色主题下检查 | 小三角箭头清晰可见 |
| AC-13 | 连接后面板切换确认 | 点击连接后面板切换 | 切换流畅无明显闪烁 |
| AC-14 | 背景图初始显示确认 | 启动后检查 | 背景图完整覆盖窗口 |

### 代码度量验收

| 编号 | 度量项 | 度量方法 | 当前基线 | 目标值 |
|------|--------|---------|---------|--------|
| M-1 | MainWindow.cpp 行数 | wc -l | 637 行 | <= 580 行(R1贡献) |
| M-2 | MainWindow.h 行数 | wc -l | 270 行 | <= 150 行(R1+R4贡献) |
| M-3 | TerminalWidget.cpp 行数 | wc -l | 713 行 | <= 630 行(R2贡献) |
| M-4 | FrameDefinition.h 行数 | wc -l | 262 行 | <= 60 行(R3贡献) |
| M-5 | PanelManager.h 存在 | 文件检查 | 新增 | 存在且 <= 100 行 |
| M-6 | PanelManager.cpp 存在 | 文件检查 | 新增 | 存在且 <= 150 行 |
| M-7 | DirectionFilter.h 存在 | 文件检查 | 新增 | 存在且 <= 90 行 |
| M-8 | DirectionFilter.cpp 存在 | 文件检查 | 新增 | 存在且 <= 100 行 |
| M-9 | FrameDefinition.cpp 存在 | 文件检查 | 新增 | 存在且 <= 190 行 |
| M-10 | MainWindow 成员指针数 | 代码检查 | 约 30 个 | <= 20 个 |
| M-11 | MainWindow.h include数 | 代码检查 | 约 30 个 | <= 10 个 |
| M-12 | 分层依赖方向 | 代码检查 | -- | 无反向依赖 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 | 预估变更量 |
|------|------|------|-----------|
| 1 | R1: PanelManager 创建 + MainWindow 适配 | 最大收益(P0)，解决 MainWindow 最严重的膨胀问题 | +230/-135 行 |
| 2 | R3: FrameDefinition 拆分 h/cpp | 最简单(P1)，无逻辑变更，纯文件拆分 | +180/-180 行 |
| 3 | R2: DirectionFilter 提取 + TerminalWidget 适配 | P0，解决 TerminalWidget 膨胀 | +170/-104 行 |
| 4 | R4: MainWindow.h 前置声明精简 | P1，依赖R1先完成面板转移 | +5/-30 行 |
| 5 | Bug验证: 下拉箭头/连接闪烁/背景图 | 确认commit #29的修复效果 | 可能 +10 行 |
| 6 | 编译验证 | 确保零错误零警告 | -- |
| 7 | 全功能回归测试 | 验证所有功能正常 | -- |

### 风险评估

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| PanelManager getter缓存导致悬空指针 | 低 | 高 | getter始终返回PanelManager持有的指针，生命周期一致 |
| DirectionFilter重建索引时机不一致 | 中 | 中 | rebuildIndices()接口明确返回更新后的cachedLineCount |
| FrameDefinition.cpp 缺少必要的include | 低 | 低 | .cpp中包含所有需要的JSON头文件 |
| 前置声明导致不完整类型错误 | 中 | 低 | 编译验证时会立即发现，在MainWindow.cpp中补充include |

---

## 验证度量指标

### 代码度量目标

| 度量项 | 当前基线 | R1后 | R2后 | R3后 | R4后(最终) |
|--------|---------|------|------|------|-----------|
| MainWindow.cpp | 637 | ~577 | 577 | 577 | 577 |
| MainWindow.h | 270 | ~205 | 205 | 205 | ~145 |
| TerminalWidget.cpp | 713 | 713 | ~643 | 643 | 643 |
| TerminalWidget.h | 149 | 149 | ~147 | 147 | 147 |
| FrameDefinition.h | 262 | 262 | 262 | ~55 | 55 |
| 新增文件 | -- | +2 | +2 | +1 | +5 |

### 架构度量目标

| 度量项 | 当前基线 | 目标值 | 说明 |
|--------|---------|--------|------|
| MainWindow 成员指针数 | ~30 | <= 20 | 面板指针转移到PanelManager |
| MainWindow.h include数 | ~30 | <= 10 | 前置声明替代 |
| TerminalWidget 过滤相关成员 | 3个 | 1个(组合DirectionFilter) | 内聚化 |
| 超限文件数 | 4个 | 2个(TerminalWidget.cpp仍超限) | 显著改善 |

### 后续迭代建议

本次迭代后仍有两个文件超限:

1. **TerminalWidget.cpp (~643行)** -- 下次迭代可提取 SearchHighlighter 类(搜索匹配逻辑约150行)进一步降至500行以下
2. **MainWindow.cpp (~577行)** -- 下次迭代可将 connectSignals() 中的部分信号连接逻辑转移到各Controller内部，进一步降至500行以下
