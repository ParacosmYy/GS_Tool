/**
 * @file TerminalWidget.h
 * @brief 自绘制终端控件 — QPainter高性能渲染, 支持文本/HEX/混合/十进制四种显示模式
 */
#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include "shared/AppConstants.h"
#include "terminal/menu/TerminalContextMenuManager.h"
#include "terminal/model/TerminalModel.h"
#include "terminal/types/DirectionFilter.h"
#include "terminal/selection/TerminalSelectionManager.h"
#include "terminal/search/TerminalSearchManager.h"
#include "terminal/search/TerminalSearchRenderer.h"
#include "terminal/types/TerminalTypes.h"

class QContextMenuEvent;

/**
 * @brief 自绘制终端控件 — 基于QPainter的高性能终端显示
 *
 * 自绘引擎实现文本/HEX/混合/十进制四种显示模式，支持搜索高亮、F3导航、
 * 方向前缀、时间戳、自动滚屏等特性。分栏模式下通过DirectionFilter过滤RX/TX数据行。
 * 所属层级: 表现层（只负责渲染，不包含业务逻辑）
 */
class TerminalWidget : public QWidget {
    Q_OBJECT

signals:
    void searchMatchesChanged(int total, int current); ///< 搜索匹配数变化 @param total 匹配总数 @param current 当前高亮索引
    void searchRequested();                             ///< 用户触发搜索(Ctrl+F)
    void pasteRequested(const QString& text);           ///< 右键菜单粘贴请求 @param text 待粘贴文本
    void clearRequested();                              ///< 用户请求清屏

public:
    /** @brief 构造终端控件 @param parent 父Widget指针 */
    explicit TerminalWidget(QWidget* parent = nullptr);
    /** @brief 设置数据模型 @param model TerminalModel指针，提供终端数据源 */
    void setModel(TerminalModel* model);
    /** @brief 设置方向过滤器(仅显示指定方向的数据) @param direction 数据方向(RX/TX/Both) */
    void setDirectionFilter(DataDirection direction);
    void clearDirectionFilter();               ///< 清除方向过滤器，恢复显示所有数据
    /** @brief 设置显示模式 @param mode 显示模式(Text/Hex/Mixed/Decimal) */
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;           ///< 获取当前显示模式
    /** @brief 设置是否显示时间戳 @param show true=显示时间戳 */
    void setShowTimestamp(bool show);
    bool showTimestamp() const;                 ///< 查询时间戳显示状态
    /** @brief 设置是否显示方向前缀(RX/TX) @param show true=显示方向前缀 */
    void setShowDirectionPrefix(bool show);
    bool showDirectionPrefix() const;           ///< 查询方向前缀显示状态
    /** @brief 设置自动滚屏 @param autoScroll true=新数据自动滚动到底部 */
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;                    ///< 查询自动滚屏状态
    void clear();                               ///< 清空终端内容
    /** @brief 获取当前选中的文本 @return 选中区域的纯文本，无选中时返回空字符串 */
    QString selectedText() const;
    /** @brief 设置搜索高亮模式 @param pattern 搜索模式串 @param regex true=正则模式 @param hex true=HEX模式 @param caseSensitive true=区分大小写 @param wholeWord true=全词匹配 */
    void setSearchHighlight(const QString& pattern, bool regex, bool hex, bool caseSensitive = false, bool wholeWord = false);
    void clearSearchHighlight();                ///< 清除搜索高亮
    /** @brief 获取搜索匹配总数 @return 匹配数量，无搜索时返回0 */
    int searchMatchCount() const;
    /** @brief 获取当前高亮的匹配索引 @return 当前匹配索引(从0开始)，无搜索时返回-1 */
    int currentMatchIndex() const;
    /** @brief 获取搜索管理器指针 @return TerminalSearchManager指针 */
    TerminalSearchManager* searchManager() const;
    void gotoNextMatch();                       ///< 跳转到下一个搜索匹配
    void gotoPrevMatch();                       ///< 跳转到上一个搜索匹配
    void selectAll();                           ///< 全选终端内容
    QSize sizeHint() const override;            ///< 返回控件推荐尺寸

protected:
    void paintEvent(QPaintEvent* event) override;       ///< 重绘事件
    void resizeEvent(QResizeEvent* event) override;     ///< 尺寸变化事件
    void wheelEvent(QWheelEvent* event) override;       ///< 滚轮事件
    void mousePressEvent(QMouseEvent* event) override;  ///< 鼠标按下事件
    void mouseMoveEvent(QMouseEvent* event) override;   ///< 鼠标移动事件
    void mouseReleaseEvent(QMouseEvent* event) override;///< 鼠标释放事件
    void keyPressEvent(QKeyEvent* event) override;      ///< 键盘按下事件
    void contextMenuEvent(QContextMenuEvent* event) override; ///< 右键菜单事件

private slots:
    void onDataAppended(int firstNewLine, int count); ///< 数据追加处理 @param firstNewLine 首行索引 @param count 新增行数
    void onDataCleared();                              ///< 数据清除处理

private:
    void updateVisibleRange();                         ///< 更新可见行范围
    void scrollToMatch(int line);                      ///< 滚动到匹配行
    void refreshSearchAfterCacheUpdate();              ///< 缓存更新后刷新搜索高亮
    CachedLine formatToCache(const TerminalLine& line) const; ///< 格式化行到缓存
    /** @brief 绘制单行 @param painter 画布 @param cached 缓存行 @param y Y坐标 @param displayLine 显示行号 @return 占用像素高度 */
    int paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine);

    TerminalModel* m_model = nullptr;
    DisplayMode m_displayMode = DisplayMode::Text;
    bool m_showTimestamp = false;
    bool m_showDirectionPrefix = false;
    bool m_autoScroll = true;
    int m_scrollOffset = 0;
    int m_scrollAccumulator = 0;
    int m_lineHeight = 18;
    int m_visibleLines = 0;
    int m_maxScrollOffset = 0;
    QFont m_font{"Consolas", 10};
    QFontMetrics m_fontMetrics{m_font};
    QColor m_bgColor;
    QColor m_rxColor;
    QColor m_txColor;
    QColor m_timestampColor;
    TerminalSelectionManager* m_selectionManager;
    TerminalSearchManager* m_searchManager;
    DirectionFilter* m_directionFilter;
    TerminalContextMenuManager* m_contextMenuManager;
    mutable QVector<CachedLine> m_cachedLines;
    mutable int m_cachedLineCount = 0;
    // ---- 统计计数器 ----
    mutable quint64 m_totalLinesRendered = 0;     ///< 总渲染行数
    quint64 m_totalKeyPresses = 0;                ///< 总按键次数
    quint64 m_totalContextMenuActions = 0;        ///< 总右键菜单操作次数
    quint64 m_totalClears = 0;                    ///< 总清屏次数
    quint64 m_totalDisplayModeChanges = 0;        ///< 总显示模式切换次数(文本/HEX/混合/十进制)
    quint64 m_totalMatchNavigations = 0;          ///< 总搜索匹配导航次数(F3/Shift+F3)

public:
    quint64 totalLinesRendered() const { return m_totalLinesRendered; }         ///< 获取总渲染行数
    quint64 totalKeyPresses() const { return m_totalKeyPresses; }               ///< 获取总按键次数
    quint64 totalContextMenuActions() const { return m_totalContextMenuActions; }///< 获取总右键菜单操作次数
    quint64 totalClears() const { return m_totalClears; }                       ///< 获取总清屏次数
    quint64 totalDisplayModeChanges() const { return m_totalDisplayModeChanges; }///< 获取总显示模式切换次数
    quint64 totalMatchNavigations() const { return m_totalMatchNavigations; }   ///< 获取总搜索匹配导航次数
    void resetTerminalWidgetStatistics(); ///< 重置终端统计计数器
};

#endif // TERMINALWIDGET_H
