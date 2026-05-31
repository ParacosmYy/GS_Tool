# ARCH-044: TerminalWidget 拆分与 BookmarkWidget 接口设计

> **迭代**: #44 | **当前评分**: 43/1000
> **架构师**: System Architect
> **日期**: 2026-06-01

---

## 一、TerminalWidget 行数分析与拆分方案

### 1.1 当前状态

TerminalWidget.cpp 当前 539 行，超出 500 行上限 39 行。需要通过提取独立类将其降至 500 行以下。

**文件行数分布**（按逻辑分区统计）:

| 分区 | 行范围 | 行数 | 职责 |
|------|--------|------|------|
| 构造与基本配置 | 22-138 | 117 | 初始化、属性setter、模型绑定 |
| 搜索委托方法 | 142-195 | 54 | setSearchHighlight/clearSearchHighlight/refreshSearch 等委托调用 |
| paintLine 单行绘制 | 199-280 | 82 | 选区背景、方向前缀、搜索高亮、文本绘制 |
| paintEvent 核心渲染 | 284-346 | 63 | 主渲染循环（普通模式 + 方向过滤模式） |
| 事件处理 | 350-448 | 99 | wheel/mouse/key/resize 事件 + 模型数据回调 |
| 缓存格式化 formatToCache | 452-475 | 24 | TerminalLine -> CachedLine 转换 |
| 右键菜单 | 480-539 | 60 | 菜单创建、contextMenuEvent、selectAll |

### 1.2 拆分方案: 提取 TerminalContextMenuManager

**决策**: 不提取搜索高亮渲染逻辑（已由 TerminalSearchManager 管理），而是提取 **右键菜单管理** 为独立类。

**理由**:
- 搜索高亮渲染（paintLine 中第 237-260 行）深度耦合 `paintLine` 的局部变量（xOffset、textXOffset、prefixLen），提取会引入复杂的参数传递，破坏 paintLine 的内聚性
- TerminalSearchManager 已经封装了搜索数据和状态，paintLine 中仅是消费搜索结果的渲染代码，属于合理的展示逻辑
- 右键菜单（60 行）是完全独立的职责：菜单创建、动作响应、上下文显示，与终端渲染零耦合
- 右键菜单提取后，TerminalWidget.cpp 预计减少约 50-55 行（createContextMenu + contextMenuEvent + 5 个 QAction 成员变量），降至约 484-489 行

**提取内容**:

```
从 TerminalWidget 提取到 TerminalContextMenuManager:

方法:
  - createContextMenu()           -> TerminalContextMenuManager::setup()
  - contextMenuEvent()            -> TerminalContextMenuManager::show(event, selectedText)

成员变量:
  - QMenu*   m_contextMenu        -> 移入
  - QAction* m_copyAction         -> 移入
  - QAction* m_pasteAction        -> 移入
  - QAction* m_clearAction        -> 移入
  - QAction* m_selectAllAction    -> 移入
  - QAction* m_searchAction       -> 移入
```

**新类: TerminalContextMenuManager**

```
文件: src/terminal/TerminalContextMenuManager.h
      src/terminal/TerminalContextMenuManager.cpp

层级: 表现层 (Presentation) — 纯 UI 交互组件

依赖:
  -> QWidget (Qt)
  -> QMenu, QAction (Qt)
  -> TerminalWidget 的信号 (通过构造时传入 parent 连接)

被依赖:
  <- TerminalWidget (组合持有)
```

**类接口设计**:

```cpp
class TerminalContextMenuManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造右键菜单管理器
     * @param parent 终端控件，作为菜单的父 widget 和信号源
     */
    explicit TerminalContextMenuManager(QWidget* parent);

    /**
     * @brief 在指定位置显示右键菜单
     * 根据当前选区状态动态设置"复制"动作的可用性
     * @param globalPos 全局坐标
     * @param hasSelection 当前是否有选中文本
     */
    void show(const QPoint& globalPos, bool hasSelection);

signals:
    /** @brief 复制请求 — 调用者应获取选中文本并写入剪贴板 */
    void copyRequested();

    /** @brief 粘贴请求 — 调用者应读取剪贴板文本 @param text */
    void pasteRequested(const QString& text);

    /** @brief 清屏请求 */
    void clearRequested();

    /** @brief 全选请求 */
    void selectAllRequested();

    /** @brief 搜索请求(Ctrl+F) */
    void searchRequested();

private:
    void setup();  ///< 创建菜单项和信号连接

    QWidget* m_parent;          ///< 宿主终端控件
    QMenu*   m_contextMenu;     ///< 右键菜单
    QAction* m_copyAction;      ///< 复制
    QAction* m_pasteAction;     ///< 粘贴
    QAction* m_clearAction;     ///< 清屏
    QAction* m_selectAllAction; ///< 全选
    QAction* m_searchAction;    ///< 搜索
};
```

**TerminalWidget 改动**:
- 移除 5 个 QAction* 成员和 QMenu* 成员，替换为 `TerminalContextMenuManager* m_contextMenuManager`
- `contextMenuEvent` 委托给 `m_contextMenuManager->show(event->globalPos(), !selectedText().isEmpty())`
- 连接 `m_contextMenuManager` 的信号到现有信号/行为

### 1.3 拆分方案备选: 为什么不提取 TerminalHighlightRenderer

用户建议了两种拆分方向，以下是分析结论:

| 方案 | 预计减少行数 | 耦合度影响 | 内聚性影响 | 推荐度 |
|------|-------------|-----------|-----------|--------|
| 提取搜索高亮渲染到 TerminalHighlightRenderer | ~25 行（仅 paintLine 内搜索高亮段） | 高 — 需传入 painter/cached/displayLine/fontMetrics/xOffset/textXOffset 等大量上下文 | 低 — 搜索高亮是 paintLine 渲染流水线的一环，不应独立 | 不推荐 |
| 提取搜索方法到 TerminalSearchHelper | ~54 行 | 中 — 搜索方法已经是薄委托层 | 低 — 仅是 5 个 delegate 方法 | 不推荐 |
| **提取右键菜单到 TerminalContextMenuManager** | **~55 行** | **低 — 仅依赖 QWidget 信号** | **高 — 菜单是完全独立的交互职责** | **推荐** |

**结论**: 搜索相关逻辑已经良好地封装在 TerminalSearchManager 中（数据+状态+搜索算法），paintLine 中仅剩的搜索高亮绘制代码（237-260 行）是渲染流水线的有机组成部分。强行提取会制造一个只有 `paintHighlights()` 单方法的类，反而增加理解和维护成本。右键菜单是完全独立的交互模块，提取收益最高。

---

## 二、BookmarkWidget 类接口设计

### 2.1 层级定位

```
BookmarkWidget -> 表现层 (Presentation)
依赖:
  -> DataBookmark (数据层: utils/DataBookmark.h)
  -> DataLogger (数据层: utils/DataLogger.h, 通过信号槽间接通信)
被依赖:
  <- MainWindow (创建和管理 BookmarkWidget 实例)
```

**分层合规性**: BookmarkWidget 属于表现层，通过信号/槽与数据层的 DataLogger 通信（观察者模式），不直接调用 DataLogger 的非 const 方法。所有数据修改通过信号向上传递，由 MainWindow 编排。

### 2.2 布局设计（复用 TerminalSearchBar 模式）

TerminalSearchBar 的布局模式: 水平排列 `[输入框] [复选框] [结果标签] [关闭按钮]`，可展开/收起嵌入终端顶部。

BookmarkWidget 采用类似的可展开面板模式:

```
+-----------------------------------------------------------------+
| [书签图标] 书签列表                               [添加] [关闭]  |
+-----------------------------------------------------------------+
| [时间]           [标签]                          [跳转] [删除]  |
| 14:30:25.123    异常发生                          [->]   [x]    |
| 14:31:02.456    复位完成                          [->]   [x]    |
| 14:35:11.789    通信超时                          [->]   [x]    |
+-----------------------------------------------------------------+
| [添加新书签] [标签输入框_________________________] [确认添加]    |
+-----------------------------------------------------------------+
```

**设计要点**:
- 顶部标题栏: 图标 + "书签列表" 标签 + 添加按钮 + 关闭按钮
- 中部列表: 使用 QListWidget 或自定义 QListView，每行显示时间/标签/操作按钮
- 底部输入区: 展开式，点击"添加新书签"后展开输入框
- 整体可嵌入右侧面板或作为独立浮层，不占用终端空间

### 2.3 类接口定义

```cpp
/**
 * @file BookmarkWidget.h
 * @brief 数据书签面板 - 书签列表展示、添加/删除、跳转定位
 *
 * 展示 DataLogger 中的书签集合，提供:
 *   - 书签列表浏览（时间 + 标签）
 *   - 添加新书签（输入标签文本）
 *   - 删除书签
 *   - 跳转到指定书签的时间位置
 *
 * 展开收起动画复用 TerminalSearchBar 的 QPropertyAnimation 模式:
 *   - 展开: maximumHeight 0 -> 280, 250ms, OutCubic
 *   - 收起: maximumHeight 280 -> 0, 200ms, InCubic
 *
 * 协作关系:
 *   - DataLogger: 通过信号槽获取书签数据（bookmarksChanged -> refreshList）
 *   - MainWindow: 创建实例，连接跳转信号到回放控制
 *   - TerminalSearchBar: 复用展开/收起动画模式
 */

#ifndef BOOKMARKWIDGET_H
#define BOOKMARKWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "utils/DataBookmark.h"

class DataLogger;

class BookmarkWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造书签面板
     * @param parent 父控件
     */
    explicit BookmarkWidget(QWidget* parent = nullptr);

    /**
     * @brief 设置数据源
     * 连接 DataLogger::bookmarksChanged 信号到 refreshList
     * @param logger 数据日志记录器，传入 nullptr 断开旧连接
     */
    void setDataLogger(DataLogger* logger);

    /**
     * @brief 激活面板（展开 + 聚焦列表）
     */
    void activate();

    /**
     * @brief 关闭面板（收起动画后隐藏）
     */
    void deactivate();

    /**
     * @brief 刷新书签列表内容
     * 从 DataLogger 获取最新书签集合，重建列表项
     */
    void refreshList();

signals:
    /**
     * @brief 跳转到指定书签的时间位置
     * MainWindow 应将此信号连接到回放引擎的 seek 操作
     * @param timestamp 书签时间戳（ms since epoch）
     */
    void jumpToBookmark(qint64 timestamp);

    /**
     * @brief 添加书签请求
     * @param label 用户输入的书签标签
     */
    void addBookmarkRequested(const QString& label);

    /**
     * @brief 删除书签请求
     * @param index 书签在列表中的索引
     */
    void removeBookmarkRequested(int index);

    /** @brief 面板关闭完成（收起动画结束后发射） */
    void closed();

private slots:
    /** @brief 列表项双击 — 触发跳转 */
    void onItemDoubleClicked(QListWidgetItem* item);

    /** @brief 添加按钮点击 — 展开输入区或直接添加 */
    void onAddClicked();

    /** @brief 确认添加 — 发射 addBookmarkRequested */
    void onConfirmAdd();

    /** @brief 删除按钮点击 — 发射 removeBookmarkRequested */
    void onDeleteClicked();

    /** @brief 关闭按钮点击 — 执行收起动画 */
    void onCloseClicked();

private:
    /** @brief 构建界面布局、创建子控件、连接信号 */
    void setupUI();

    /** @brief 创建单条书签的列表项控件 */
    QWidget* createBookmarkItem(const DataBookmark& bookmark, int index);

    // ---- 子控件 ----
    QLabel*      m_titleLabel;       ///< 标题 "书签列表"
    QPushButton* m_addBtn;           ///< 添加按钮
    QPushButton* m_closeBtn;         ///< 关闭按钮
    QListWidget* m_listWidget;       ///< 书签列表
    QLineEdit*   m_labelInput;       ///< 标签输入框
    QPushButton* m_confirmAddBtn;    ///< 确认添加按钮
    QWidget*     m_inputArea;        ///< 输入区域（可展开/收起）

    // ---- 数据 ----
    DataLogger*           m_logger = nullptr;   ///< 数据源
    QVector<DataBookmark> m_bookmarks;          ///< 当前书签数据缓存
};

#endif // BOOKMARKWIDGET_H
```

### 2.4 信号流设计

```
BookmarkWidget                    MainWindow                    DataLogger
     |                                |                             |
     |-- addBookmarkRequested ------> | --addBookmark()-----------> |
     |                                |                             |
     |                                | <--bookmarksChanged-------- |
     | <-- (setDataLogger连接) -------|                             |
     |                                |                             |
     |-- removeBookmarkRequested ---> | --removeBookmark()--------> |
     |                                |                             |
     |-- jumpToBookmark(ts) --------> | --seekPlayback(ts)--------> |
     |                                |                             |
```

**关键设计决策**:
- BookmarkWidget 不直接调用 DataLogger 的修改方法，而是通过信号向上传递请求
- MainWindow 作为编排层，负责连接 BookmarkWidget 的请求信号到 DataLogger 的方法
- DataLogger 的 `bookmarksChanged` 信号直接连接到 BookmarkWidget 的 `refreshList` 槽（通过 setDataLogger 建立连接）
- 这种单向数据流确保表现层不直接修改数据层，符合分层规则

---

## 三、DataLogger seek 方法设计

### 3.1 需求背景

BookmarkWidget 的跳转功能需要 DataLogger 支持按时间戳定位回放位置。当前 DataLogger 的回放是顺序读取，没有 seek 能力。

### 3.2 EDL 文件格式分析

```
当前 EDL 格式:
[Header: 8 bytes]  magic(3) + version(1) + recordCount(4)
[Record 0]         timestamp(8) + direction(1) + length(4) + data(length)
[Record 1]         timestamp(8) + direction(1) + length(4) + data(length)
...
[Record N]         timestamp(8) + direction(1) + length(4) + data(length)
```

**问题**: EDL 格式的记录长度不固定（data 字段可变长），无法通过简单数学计算 seek 到任意时间戳对应的文件偏移。

### 3.3 seek 方案设计

**方案: 线性扫描 seek（Phase2 适用，后续迭代可优化为索引表）**

```cpp
/**
 * @brief 将回放位置定位到指定时间戳
 *
 * 从文件头开始顺序读取 RecordHeader，找到第一个 timestamp >= targetTs 的记录，
 * 定位文件指针到该记录的数据起始位置，重置回放状态。
 *
 * Phase2 实现说明:
 *   - 线性扫描，时间复杂度 O(N)
 *   - 适用于当前阶段的书签数量（通常 < 1000 条）
 *   - 后续迭代可引入索引表（在录制结束时写入 seek table）优化为 O(logN)
 *
 * @param targetTs 目标时间戳（ms since epoch），录制期间的绝对时间
 * @return true 定位成功，false 定位失败（未在回放状态或文件读取错误）
 */
bool seekToTimestamp(qint64 targetTs);
```

**seekToTimestamp 实现逻辑**:

```
1. 前置检查: 必须处于回放状态且文件有效
2. 停止回放计时器
3. seek 文件到 Header 之后（偏移 8 字节）
4. 循环读取 RecordHeader:
   a. 读取 timestamp(8) + direction(1) + length(4)
   b. 如果 timestamp >= targetTs: 找到目标记录
      - 回退文件指针到该记录起始位置
      - 设置 m_nextRecordTime = timestamp
      - 设置 m_playbackBaseTime = 当前已回放时间偏移
      - 重启回放计时器
      - return true
   c. 否则: seek 跳过 data(length) 字节，继续
5. 文件读完未找到: return false（时间戳超出录制范围）
```

**信号扩展**:

```cpp
signals:
    /** @brief 回放位置跳转完成 @param timestamp 实际跳转到的记录时间戳 */
    void playbackSeeked(qint64 timestamp);
```

### 3.4 后续优化方向（Phase3+）

| 方案 | 复杂度 | 说明 |
|------|--------|------|
| 线性扫描（当前） | O(N) | 简单可靠，适合少量书签 |
| 录制结束写索引表 | O(logN) | 在 EDL 文件尾部追加 `{timestamp, fileOffset}` 排序数组，seek 时二分查找 |
| 边录边建索引 | O(logN) | 录制期间维护内存中的时间戳索引，录制结束时序列化到文件尾部 |

---

## 四、对现有分层架构的影响评估

### 4.1 分层合规性检查

| 新增/修改类 | 层级 | 依赖方向 | 是否合规 |
|------------|------|---------|---------|
| TerminalContextMenuManager | 表现层 | -> QWidget, QMenu (Qt) | 合规 |
| BookmarkWidget | 表现层 | -> DataBookmark (数据层), DataLogger (数据层, 信号槽) | 合规 |
| DataLogger::seekToTimestamp | 数据层 | -> QFile (Qt) | 合规 |

**依赖方向验证**:
- 表现层 -> 数据层: 合规（单向依赖）
- 数据层 -> 基础设施层: 合规（QFile）
- 无反向依赖: 数据层不依赖表现层

### 4.2 公共组件清单更新

新增 2 个组件需登记:

| 组件 | 文件 | 用途 |
|------|------|------|
| `TerminalContextMenuManager` | `terminal/TerminalContextMenuManager.h/cpp` | 终端右键菜单管理（复制/粘贴/清屏/全选/搜索） |
| `BookmarkWidget` | `serial/BookmarkWidget.h/cpp` | 数据书签面板（列表/添加/删除/跳转定位） |

**文件放置位置**:
- `TerminalContextMenuManager` 放在 `src/terminal/` 目录，与 TerminalWidget 同级，因为它是终端控件的协作组件
- `BookmarkWidget` 放在 `src/serial/` 目录，与 SerialConfigPanel 同级，因为它是串口功能的面板级控件（而非终端内部组件）

### 4.3 对现有文件的影响

| 文件 | 变更类型 | 变更内容 | 行数变化 |
|------|---------|---------|---------|
| `TerminalWidget.h` | 修改 | 移除 5 个 QAction* + QMenu* 成员，新增 TerminalContextMenuManager* | -6, +1 |
| `TerminalWidget.cpp` | 修改 | 移除 createContextMenu/contextMenuEvent，contextMenuEvent 改为委托调用 | -55, +5 |
| `TerminalSearchBar.h/cpp` | 不变 | 仅作为布局参考，无需修改 | 0 |
| `DataLogger.h` | 修改 | 新增 seekToTimestamp() 方法声明和 playbackSeeked 信号 | +8 |
| `DataLogger.cpp` | 修改 | 实现 seekToTimestamp() | +40 |
| `DataBookmark.h` | 不变 | 已有完整的序列化和比较接口 | 0 |
| `RecordingController.h` | 不变 | 已有 addBookmarkRequested 信号，无需修改 | 0 |
| `MainWindow.h/cpp` | 修改 | 新增 BookmarkWidget 实例创建和信号编排 | +15 |

### 4.4 设计模式合规性

| 模式 | 应用点 | 说明 |
|------|--------|------|
| 组合模式 | TerminalWidget 持有 TerminalContextMenuManager | 与现有 TerminalSelectionManager/TerminalSearchManager 一致 |
| 观察者模式 | DataLogger::bookmarksChanged -> BookmarkWidget::refreshList | Qt 信号槽，数据变化自动刷新 UI |
| 委托模式 | MainWindow 编排 BookmarkWidget 请求 -> DataLogger 操作 | 表现层不直接调用数据层修改方法 |

### 4.5 CMakeLists.txt 影响

需在 `src/terminal/` 和 `src/serial/` 分别添加新的源文件:

```cmake
# terminal/
terminal/TerminalContextMenuManager.h
terminal/TerminalContextMenuManager.cpp

# serial/
serial/BookmarkWidget.h
serial/BookmarkWidget.cpp
```

---

## 五、实施计划

### 5.1 实施顺序

```
Phase A: TerminalWidget 拆分（优先级 P0）
  Step 1: 创建 TerminalContextMenuManager.h/cpp
  Step 2: 将右键菜单逻辑从 TerminalWidget 迁移到新类
  Step 3: 修改 TerminalWidget 使用新的菜单管理器
  Step 4: 验证 TerminalWidget.cpp < 500 行
  Step 5: 编译验证 + 功能测试（右键菜单所有操作正常）

Phase B: DataLogger seek 方法（优先级 P1）
  Step 1: 在 DataLogger.h 声明 seekToTimestamp() 和 playbackSeeked 信号
  Step 2: 在 DataLogger.cpp 实现线性扫描 seek
  Step 3: 单元测试验证 seek 精度

Phase C: BookmarkWidget（优先级 P1，依赖 Phase B）
  Step 1: 创建 BookmarkWidget.h/cpp
  Step 2: 实现 setupUI + 列表展示
  Step 3: 实现 add/remove/jump 交互
  Step 4: 实现展开/收起动画
  Step 5: 在 MainWindow 中集成（创建实例 + 信号编排）
  Step 6: 在三个主题 QSS 中添加样式
```

### 5.2 风险点

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| seekToTimestamp 线性扫描在长录制文件上耗时 | 回放跳转卡顿 | 限制单次扫描最大记录数，超过则提示用户；Phase3 引入索引表 |
| BookmarkWidget 列表项自定义控件（跳转/删除按钮）复杂度 | 实现时间超预期 | 使用 QListWidget + setItemWidget 简化实现 |
| 右键菜单提取后信号链变长 | 调试复杂度略增 | 保持信号名称与原有一致，减少认知负担 |

---

## 六、总结

本次架构分析的核心决策:

1. **TerminalWidget 拆分方向**: 提取右键菜单管理器（TerminalContextMenuManager），而非搜索高亮渲染器。理由是搜索逻辑已由 TerminalSearchManager 良好封装，paintLine 中的搜索高亮是渲染流水线的有机组成；而右键菜单是完全独立的交互模块，提取后 TerminalWidget.cpp 可降至 ~485 行。

2. **BookmarkWidget 设计**: 遵循 TerminalSearchBar 的展开/收起模式，通过信号向上传递操作请求，由 MainWindow 编排与 DataLogger 的交互，确保表现层不直接修改数据层。

3. **DataLogger seek**: Phase2 采用线性扫描实现，接口预留后续优化空间。seekToTimestamp 不改变 EDL 文件格式，仅在回放时进行文件扫描。

4. **分层合规**: 所有新增类和修改均满足 表现层 -> 数据层 -> 基础设施层 的单向依赖规则。
