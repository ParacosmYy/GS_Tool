# PRD-044: TerminalWidget拆分 + DataBookmark Phase2 + DataLogger Seek

## 背景

迭代#43完成后，TerminalWidget.cpp 达到539行，突破 CLAUDE.md 4.6 节规定的500行上限。
该文件混合了三种职责：(1) 终端渲染核心、(2) 搜索高亮渲染逻辑、(3) 事件处理与用户交互。
按单一职责原则，搜索高亮渲染（paintLine 中约60行搜索矩形绘制 + refreshSearch/refreshSearchAfterCacheUpdate 约15行）
应提取到独立的渲染辅助类中。

DataBookmark Phase1（迭代#43）已建立 DataBookmark 数据结构和 DataLogger CRUD API，但缺少用户可见的 UI 面板。
Phase2 需要实现 BookmarkWidget：一个嵌入导航面板体系的可切换面板，显示书签列表、支持增删操作、
点击书签跳转到对应时间戳位置。为此 DataLogger 需要新增 seek 支持——从指定时间戳位置开始回放。

本迭代三大目标相互独立，可并行开发：
1. TerminalWidget.cpp 拆分（架构整洁度强制要求）
2. BookmarkWidget UI 面板（用户可交互的书签管理）
3. DataLogger seek 支持（回放定位基础设施）

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 提取 TerminalWidget 中搜索高亮渲染逻辑到独立的 TerminalHighlightRenderer 辅助类，使 TerminalWidget.cpp 降至500行以下 | P0 | terminal |
| R2 | 新增 BookmarkWidget 面板类：书签列表（QListWidget）、添加按钮、删除按钮、清空按钮 | P0 | core |
| R3 | BookmarkWidget 注册到 PanelManager 面板体系，可通过导航树切换显示 | P0 | core |
| R4 | BookmarkWidget 与 DataLogger 的 bookmarksChanged 信号联动，实时同步书签列表 | P1 | core |
| R5 | BookmarkWidget 添加书签时弹出输入对话框（QInputDialog），用户填写标签文本 | P1 | core |
| R6 | BookmarkWidget 点击书签项时发射 bookmarkClicked(qint64 timestamp) 信号 | P1 | core |
| R7 | DataLogger 新增 seekToTimestamp(qint64 targetTs) 方法，回放时定位到指定时间戳位置 | P1 | utils |
| R8 | DataLogger 新增 seekToRecord(int recordIndex) 方法，回放时定位到指定记录索引 | P2 | utils |
| R9 | BookmarkWidget 中书签项显示格式："HH:mm:ss.zzz - 标签文本"，时间戳从 DataBookmark.timestamp 格式化 | P1 | core |
| R10 | BookmarkWidget 在录制状态时显示"添加书签"按钮，非录制状态隐藏 | P2 | core |
| R11 | 三主题 QSS 中添加 BookmarkWidget 对应样式 | P0 | resources |

## 接口设计

### 1. TerminalHighlightRenderer（新类）

文件位置: `src/terminal/TerminalHighlightRenderer.h/cpp`

```cpp
/**
 * @brief 终端高亮渲染器 - 负责搜索匹配高亮矩形的绘制计算
 *
 * 从 TerminalWidget::paintLine() 中提取的搜索高亮渲染逻辑。
 * 接收当前行的缓存数据、行号、文本起始偏移量，
 * 计算并绘制所有匹配该行的搜索高亮矩形。
 *
 * 协作关系:
 *   - TerminalWidget: 在 paintLine() 中调用本类的 renderSearchHighlights()
 *   - TerminalSearchManager: 从中获取搜索匹配列表和当前匹配索引
 */
class TerminalHighlightRenderer {
public:
    explicit TerminalHighlightRenderer(
        const TerminalSearchManager* searchManager,
        const QFontMetrics& fontMetrics);

    /**
     * @brief 绘制单行的搜索高亮矩形
     * @param painter QPainter 引用
     * @param cached 当前行的缓存数据
     * @param displayLine 显示行号（用于匹配 SearchMatch.line）
     * @param y 行的像素 Y 坐标
     * @param lineHeight 行高（像素）
     * @param textXOffset 文本内容起始 X 偏移（已含时间戳宽度）
     * @param showDirectionPrefix 是否显示方向前缀
     */
    void renderSearchHighlights(QPainter& painter,
                                const CachedLine& cached,
                                int displayLine,
                                int y,
                                int lineHeight,
                                int textXOffset,
                                bool showDirectionPrefix) const;

private:
    const TerminalSearchManager* m_searchManager;   ///< 搜索管理器（只读引用）
    QFontMetrics m_fontMetrics;                     ///< 字体度量（用于计算文本宽度）
};
```

### 2. BookmarkWidget（新类）

文件位置: `src/core/BookmarkWidget.h/cpp`

```cpp
/**
 * @brief 书签面板 - 展示和管理录制数据流中的书签标记
 *
 * 作为导航面板体系中的一个可切换面板，提供:
 *   - 书签列表（QListWidget）：按时间排序显示所有书签
 *   - 添加按钮：录制中弹出输入对话框添加新书签
 *   - 删除按钮：删除选中的书签
 *   - 清空按钮：清除所有书签
 *   - 点击跳转：点击书签项发射 bookmarkClicked 信号
 *
 * 协作关系:
 *   - DataLogger: 通过 bookmarksChanged 信号同步书签列表
 *   - RecordingController: 连接录制状态控制"添加"按钮的可见性
 *   - NavigationController: 通过 PanelManager 注册为可切换面板
 *   - MainWindow: 接收 bookmarkClicked 信号，调用 DataLogger::seekToTimestamp
 */
class BookmarkWidget : public QWidget {
    Q_OBJECT

public:
    explicit BookmarkWidget(QWidget* parent = nullptr);

    /**
     * @brief 设置 DataLogger 实例
     * 连接 bookmarksChanged 信号用于自动刷新列表
     * @param logger 数据日志记录器
     */
    void setDataLogger(DataLogger* logger);

    /** @brief 设置录制状态，控制添加按钮的可见性 */
    void setRecording(bool recording);

signals:
    /** @brief 用户点击书签项，请求跳转到指定时间戳 @param timestamp 书签的时间戳（ms since epoch） */
    void bookmarkClicked(qint64 timestamp);

private slots:
    void onAddBookmark();           ///< 添加书签（弹出输入对话框）
    void onDeleteBookmark();        ///< 删除选中书签
    void onClearBookmarks();        ///< 清空所有书签
    void onBookmarkItemClicked(QListWidgetItem* item);  ///< 书签项点击
    void refreshBookmarkList();     ///< 从 DataLogger 重新加载书签列表

private:
    void setupUI();                 ///< 构建界面布局

    DataLogger* m_logger = nullptr; ///< 数据日志记录器
    QListWidget* m_listWidget = nullptr;    ///< 书签列表控件
    QPushButton* m_addBtn = nullptr;        ///< 添加书签按钮
    QPushButton* m_deleteBtn = nullptr;     ///< 删除书签按钮
    QPushButton* m_clearBtn = nullptr;      ///< 清空书签按钮
    bool m_recording = false;               ///< 当前是否在录制中
};
```

### 3. DataLogger 新增接口

```cpp
// ---- Seek 支持（新增） ----

/**
 * @brief 回放定位到指定时间戳位置
 * 仅在回放状态下有效。扫描 EDL 文件中的 RecordHeader，
 * 找到 timestamp >= targetTs 的第一条记录，从该位置恢复回放。
 * @param targetTs 目标时间戳（距录制开始的毫秒偏移）
 * @return true=定位成功，false=回放未启动或文件读取失败
 */
bool seekToTimestamp(qint64 targetTs);

/**
 * @brief 回放定位到指定记录索引位置
 * 仅在回放状态下有效。跳过前 recordIndex 条记录，
 * 从第 recordIndex 条记录开始恢复回放。
 * @param recordIndex 目标记录索引（从0开始）
 * @return true=定位成功，false=回放未启动或索引越界
 */
bool seekToRecord(int recordIndex);
```

### 4. PanelManager 新增 Getter

```cpp
/** @brief 获取书签面板（录制数据流书签管理和跳转） */
BookmarkWidget* bookmarkWidget() const;
```

### 5. NavigationController 导航树新增项

在导航树中"数据工具"分组下添加"书签"入口，与"数据统计""数据导出"同级。

## 依赖的公共组件

| 组件 | 复用方式 |
|------|---------|
| DataLogger | 扩展（新增 seekToTimestamp/seekToRecord） |
| DataBookmark | 直接使用（BookmarkWidget 显示和操作书签数据） |
| TerminalSearchManager | 只读引用（TerminalHighlightRenderer 从中获取搜索匹配） |
| PanelManager | 扩展（新增 BookmarkWidget 面板注册和 Getter） |
| NavigationController | 扩展（导航树添加"书签"项） |
| ThemeManager | 复用（BookmarkWidget 颜色从语义色板获取） |
| RecordingController | 扩展（录制状态联动 BookmarkWidget 添加按钮） |

## 设计模式

- **组合模式**: TerminalHighlightRenderer 作为 TerminalWidget 的组合成员，持有搜索管理器的只读引用。
  搜索高亮绘制委托给渲染器，TerminalWidget 保留核心渲染（背景、文本）职责。
- **观察者模式**: DataLogger::bookmarksChanged() 信号驱动 BookmarkWidget::refreshBookmarkList() 槽函数，
  实现数据层到表现层的单向通知。
- **委托模式**: BookmarkWidget 不直接操作 DataLogger 的内部数据，通过 DataLogger 的公开 API 进行增删查操作。
- **策略模式**: DataLogger 的 seekToTimestamp 和 seekToRecord 提供两种不同的定位策略（按时间 vs 按索引），
  上层可根据场景选择合适的定位方式。

## 影响范围

| 文件 | 变更类型 | 影响 |
|------|---------|------|
| src/terminal/TerminalHighlightRenderer.h | 新增 | 搜索高亮渲染辅助类头文件 |
| src/terminal/TerminalHighlightRenderer.cpp | 新增 | 搜索高亮渲染辅助类实现 |
| src/terminal/TerminalWidget.h | 修改 | 新增 TerminalHighlightRenderer* 成员，移除 paintLine 中的搜索渲染内联代码 |
| src/terminal/TerminalWidget.cpp | 修改 | 重构 paintLine() 委托搜索高亮到渲染器，目标降至480行以内 |
| src/core/BookmarkWidget.h | 新增 | 书签面板头文件 |
| src/core/BookmarkWidget.cpp | 新增 | 书签面板实现 |
| src/utils/DataLogger.h | 修改 | 新增 seekToTimestamp/seekToRecord 声明 |
| src/utils/DataLogger.cpp | 修改 | 新增 seekToTimestamp/seekToRecord 实现 |
| src/core/PanelManager.h | 修改 | 新增 BookmarkWidget* 成员和 getter |
| src/core/PanelManager.cpp | 修改 | createPanels() 中创建 BookmarkWidget 并注册 |
| src/core/MainWindow.cpp | 修改 | 连接 BookmarkWidget 信号（bookmarkClicked → seekToTimestamp） |
| src/core/RecordingController.cpp | 修改 | 录制状态联动 BookmarkWidget::setRecording() |
| src/core/NavigationController.cpp | 修改 | 导航树添加"书签"项 |
| resources/themes/dark_terminal.qss | 修改 | 添加 BookmarkWidget 样式 |
| resources/themes/modern_dark.qss | 修改 | 添加 BookmarkWidget 样式 |
| resources/themes/light.qss | 修改 | 添加 BookmarkWidget 样式 |
| CMakeLists.txt | 修改 | 添加 TerminalHighlightRenderer.h/cpp 和 BookmarkWidget.h/cpp |

## 验收标准

1. **TerminalWidget.cpp 行数 <= 500行**：提取搜索高亮渲染逻辑后，TerminalWidget.cpp 总行数不超过500行
2. **TerminalHighlightRenderer 独立编译**：新类可独立编译，不影响终端渲染的视觉效果
3. **搜索功能回归通过**：文本搜索、正则搜索、HEX搜索、F3/Shift+F3导航功能与拆分前完全一致
4. **搜索高亮位置正确**：时间戳显示时搜索高亮矩形位置与拆分前一致，无水平偏移
5. **BookmarkWidget 编译通过**：新面板可正常创建、显示、隐藏
6. **导航树切换正常**：点击导航树"书签"项可切换到 BookmarkWidget 面板
7. **书签增删查功能正常**：
   - 点击"添加"弹出输入对话框，输入标签后书签出现在列表中
   - 选中书签后点击"删除"，书签从列表中消失
   - 点击"清空"，所有书签被清除
8. **书签列表显示格式正确**：每项显示 "HH:mm:ss.zzz - 标签文本"
9. **DataLogger seekToTimestamp 正常**：回放状态下调用 seekToTimestamp 后回放从目标位置继续
10. **DataLogger seekToRecord 正常**：回放状态下调用 seekToRecord 后回放从指定记录继续
11. **三主题 QSS 样式完整**：BookmarkWidget 在 dark_terminal/modern_dark/light 三个主题下样式正确
12. **零编译错误**：cmake --build build 零错误
13. **所有修改文件行数未突破 CLAUDE.md 4.6 上限**（.h <= 200行, .cpp <= 500行, 单方法 <= 80行）
14. **EmbedDebug.bat 启动正常**：双击 bat 文件应用可正常启动

## 实现优先级

```
Phase A（并行启动）:
  ├── 核心开发: DataLogger seekToTimestamp/seekToRecord 实现
  ├── UI开发 A: TerminalHighlightRenderer 提取 + TerminalWidget.cpp 行数瘦身
  └── UI开发 B: BookmarkWidget UI 面板实现

Phase B（依赖 Phase A）:
  └── 集成开发: PanelManager 注册 + NavigationController 导航项 + MainWindow 信号连接
      + RecordingController 状态联动 + 三主题 QSS 补全
```
