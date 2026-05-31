#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QTimer>
#include "TerminalModel.h"
#include "Constants.h"

// 自绘制终端控件 - 使用QPainter直接绘制文本
// 比QTextEdit/QPlainTextEdit性能更好，适合大流量数据显示
class TerminalWidget : public QWidget {
    Q_OBJECT

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

    // 自动滚动开关
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;

    // 清空显示
    void clear();

    // 获取选中的文本
    QString selectedText() const;

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

    // 将数据按显示模式格式化为文本行
    QString formatLine(const TerminalLine& line) const;

    // 将QByteArray格式化为HEX字符串
    QString toHexString(const QByteArray& data) const;

    TerminalModel* m_model = nullptr;
    DisplayMode m_displayMode = DisplayMode::Text;
    bool m_showTimestamp = false;
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
    bool m_isSelecting = false;

    // 缓存格式化后的文本行，避免每帧都重新计算
    mutable QVector<QString> m_cachedLines;
    mutable int m_cachedLineCount = 0;
};

#endif // TERMINALWIDGET_H
