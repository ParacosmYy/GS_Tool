/**
 * @file TerminalWidget.h
 * @brief auto-draw terminal widget - QPainter high-performance rendering
 */
#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include "terminal/TerminalContextMenuManager.h"
#include "terminal/TerminalModel.h"
#include "terminal/DirectionFilter.h"
#include "terminal/TerminalSelectionManager.h"
#include "terminal/TerminalSearchManager.h"
#include "terminal/TerminalSearchRenderer.h"
#include "core/Constants.h"
#include "terminal/TerminalTypes.h"

class QContextMenuEvent;

/**
 * @brief 自绘制终端控件 — 基于QPainter的高性能终端显示
 *
 * 自绘引擎实现文本/HEX/混合/十进制四种显示模式，支持搜索高亮、F3导航、
 * 方向前缀、时间戳、自动滚屏等特性。分栏模式下通过DirectionFilter过滤RX/TX数据行。
 *
 * 协作关系:
 *   - TerminalModel: 数据源，通过 dataAppended 信号驱动增量渲染
 *   - TerminalSearchManager: 搜索匹配和高亮导航
 *   - TerminalSelectionManager: 文本选择和复制
 *   - TerminalContextMenuManager: 右键菜单（复制/粘贴/清屏/搜索/导出）
 *   - DirectionFilter: 分栏模式下的RX/TX方向过滤
 *   - TerminalLayoutManager: 管理终端的布局切换
 *
 * 所属层级: 表现层（只负责渲染，不包含业务逻辑）
 */
class TerminalWidget : public QWidget {
    Q_OBJECT

signals:
    /** @brief 搜索匹配数变化 @param total 匹配总数 @param current 当前高亮索引 */
    void searchMatchesChanged(int total, int current);
    /** @brief 用户触发搜索(Ctrl+F) */
    void searchRequested();
    /** @brief 右键菜单粘贴请求 @param text 待粘贴文本 */
    void pasteRequested(const QString& text);
    /** @brief 用户请求清屏 */
    void clearRequested();

public:
    explicit TerminalWidget(QWidget* parent = nullptr);
    void setModel(TerminalModel* model);
    void setDirectionFilter(DataDirection direction);
    void clearDirectionFilter();
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;
    void setShowTimestamp(bool show);
    bool showTimestamp() const;
    void setShowDirectionPrefix(bool show);
    bool showDirectionPrefix() const;
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;
    void clear();
    QString selectedText() const;
    void setSearchHighlight(const QString& pattern, bool regex, bool hex);
    void clearSearchHighlight();
    int searchMatchCount() const;
    int currentMatchIndex() const;
    void gotoNextMatch();
    void gotoPrevMatch();
    void selectAll();
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onDataAppended(int firstNewLine, int count);
    void onDataCleared();

private:
    void updateVisibleRange();
    void scrollToMatch(int line);
    void refreshSearchAfterCacheUpdate();
    CachedLine formatToCache(const TerminalLine& line) const;
    int paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine);

    TerminalModel* m_model = nullptr;
    DisplayMode m_displayMode = DisplayMode::Text;
    bool m_showTimestamp = false;
    bool m_showDirectionPrefix = false;
    bool m_autoScroll = true;
    int m_scrollOffset = 0;
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
};

#endif // TERMINALWIDGET_H
