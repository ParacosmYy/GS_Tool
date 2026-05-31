#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QTimer>
#include "TerminalModel.h"
#include "core/Constants.h"

// 缓存行的完整信息，避免paintEvent中调用lineAt()访问环形缓冲区
// 每次缓存更新时一次性填充，渲染时只读缓存即可
struct CachedLine {
    QString text;            // 按当前DisplayMode格式化后的文本
    DataDirection direction; // 收/发方向，用于选择文字颜色
    qint64 timestamp;        // epoch毫秒时间戳，用于时间戳显示
};

// 自绘制终端控件 - 使用QPainter直接绘制文本
// 比QTextEdit/QPlainTextEdit性能更好，适合大流量数据显示
class TerminalWidget : public QWidget {
    Q_OBJECT

signals:
    void searchMatchesChanged(int total, int current);

public:
    explicit TerminalWidget(QWidget* parent = nullptr);

    // 设置数据模型
    void setModel(TerminalModel* model);

    // 显示模式
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

    // 时间戳开关
    void setShowTimestamp(bool show);
    bool showTimestamp() const;

    // 方向前缀开关 [TX:] / [RX:]
    void setShowDirectionPrefix(bool show);
    bool showDirectionPrefix() const;

    // 自动滚动开关
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;

    // 清空显示
    void clear();

    // 获取选中的文本
    QString selectedText() const;

    // 搜索功能
    void setSearchHighlight(const QString& pattern, bool regex, bool hex);
    void clearSearchHighlight();
    int searchMatchCount() const;
    int currentMatchIndex() const;
    void gotoNextMatch();
    void gotoPrevMatch();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // 模型有新数据时，安排重绘
    void onDataAppended(int firstNewLine, int count);
    void onDataCleared();

private:
    // 计算可见区域可以显示多少行
    void updateVisibleRange();

    // 滚动到指定行（搜索匹配导航）
    void scrollToMatch(int line);

    // 重新执行搜索（缓存更新后调用）
    void refreshSearch();

    // 将数据行转换为缓存结构（格式化文本 + 方向 + 时间戳）
    CachedLine formatToCache(const TerminalLine& line) const;

    TerminalModel* m_model = nullptr;
    DisplayMode m_displayMode = DisplayMode::Text;
    bool m_showTimestamp = false;
    bool m_showDirectionPrefix = false;
    bool m_autoScroll = true;

    // 滚动和渲染相关
    int m_scrollOffset = 0;         // 当前滚动偏移(行数)
    int m_lineHeight = 18;          // 每行像素高度
    int m_visibleLines = 0;         // 可见行数
    int m_maxScrollOffset = 0;      // 最大滚动偏移
    QFont m_font;                   // 终端字体(等宽)

    // 颜色配置
    QColor m_bgColor;               // 背景色
    QColor m_rxColor;               // 接收文本颜色
    QColor m_txColor;               // 发送文本颜色
    QColor m_timestampColor;        // 时间戳颜色
    QColor m_selectionBg;           // 选中背景色

    // 鼠标选择
    int m_selectionStartLine = -1;
    int m_selectionEndLine = -1;
    int m_selectionStartCol = -1;
    int m_selectionEndCol = -1;
    bool m_isSelecting = false;

    // 搜索高亮
    struct SearchMatch { int line; int startCol; int length; };
    QVector<SearchMatch> m_searchMatches;
    int m_currentMatchIndex = -1;
    QString m_searchPattern;
    bool m_searchRegex = false;
    bool m_searchHex = false;
    QColor m_searchHighlightColor;
    QColor m_currentMatchColor;

    // 缓存格式化后的行信息（文本+方向+时间戳），避免paintEvent访问环形缓冲区
    mutable QVector<CachedLine> m_cachedLines;
    mutable int m_cachedLineCount = 0;
};

#endif // TERMINALWIDGET_H
