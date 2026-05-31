#include "TerminalWidget.h"
#include "utils/HexConverter.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
{
    // 设置等宽字体
    m_font = QFont("Consolas", 10);
    m_font.setStyleHint(QFont::Monospace);

    // 暗色主题配色
    m_bgColor = QColor(30, 30, 46);         // #1e1e2e
    m_rxColor = QColor(205, 214, 244);       // #cdd6f4 接收数据(白色)
    m_txColor = QColor(166, 227, 161);       // #a6e3a1 发送数据(绿色)
    m_timestampColor = QColor(147, 153, 178); // #9399b2 时间戳(灰色)
    m_selectionBg = QColor(69, 71, 90);      // #45475a 选中背景

    // 基础设置
    setFont(m_font);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // 计算行高
    QFontMetrics fm(m_font);
    m_lineHeight = fm.height() + 2;

    setMinimumSize(400, 200);
}

void TerminalWidget::setModel(TerminalModel* model)
{
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }
    m_model = model;
    if (m_model) {
        connect(m_model, &TerminalModel::dataAppended,
                this, &TerminalWidget::onDataAppended);
        connect(m_model, &TerminalModel::dataCleared,
                this, &TerminalWidget::onDataCleared);
    }
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    update();
}

void TerminalWidget::setDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
    m_cachedLineCount = 0;  // 清缓存，强制重新格式化
    update();
}

DisplayMode TerminalWidget::displayMode() const
{
    return m_displayMode;
}

void TerminalWidget::setShowTimestamp(bool show)
{
    m_showTimestamp = show;
    m_cachedLineCount = 0;
    update();
}

bool TerminalWidget::showTimestamp() const
{
    return m_showTimestamp;
}

void TerminalWidget::setAutoScroll(bool autoScroll)
{
    m_autoScroll = autoScroll;
    if (m_autoScroll) {
        // 跳到最新
        m_scrollOffset = m_maxScrollOffset;
        update();
    }
}

bool TerminalWidget::autoScroll() const
{
    return m_autoScroll;
}

void TerminalWidget::clear()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    update();
}

QString TerminalWidget::selectedText() const
{
    // TODO: 实现文本选择
    return QString();
}

QSize TerminalWidget::sizeHint() const
{
    return QSize(800, 600);
}

void TerminalWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 填充背景
    painter.fillRect(rect(), m_bgColor);
    painter.setFont(m_font);

    if (!m_model) {
        // 没有模型时显示提示
        painter.setPen(m_timestampColor);
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("No connection - waiting for data..."));
        return;
    }

    // 获取模型数据
    auto lines = m_model->lines();
    int totalLines = lines.size();

    // 如果缓存失效，重新格式化
    if (m_cachedLineCount != totalLines) {
        m_cachedLines.resize(totalLines);
        for (int i = m_cachedLineCount; i < totalLines; ++i) {
            m_cachedLines[i] = formatLine(lines[i]);
        }
        m_cachedLineCount = totalLines;

        // 更新最大滚动偏移
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }
    }

    // 计算可见范围
    int startLine = m_scrollOffset;
    int endLine = qMin(startLine + m_visibleLines + 1, totalLines);

    // 逐行绘制
    int y = 0;
    for (int i = startLine; i < endLine; ++i) {
        // 选择背景色
        if (i >= m_selectionStartLine && i <= m_selectionEndLine
            && m_selectionStartLine >= 0) {
            painter.fillRect(0, y, width(), m_lineHeight, m_selectionBg);
        }

        // 根据方向设置文字颜色
        if (lines[i].direction == DataDirection::Tx) {
            painter.setPen(m_txColor);
        } else {
            painter.setPen(m_rxColor);
        }

        // 绘制时间戳
        int xOffset = 0;
        if (m_showTimestamp) {
            painter.setPen(m_timestampColor);
            QString ts = lines[i].timestamp.toString("HH:mm:ss.zzz");
            painter.drawText(4, y + m_lineHeight - 4, ts);
            QFontMetrics fm(m_font);
            xOffset = fm.horizontalAdvance(ts) + 12;

            // 恢复数据颜色
            if (lines[i].direction == DataDirection::Tx) {
                painter.setPen(m_txColor);
            } else {
                painter.setPen(m_rxColor);
            }
        }

        // 绘制数据内容
        painter.drawText(xOffset + 4, y + m_lineHeight - 4, m_cachedLines[i]);
        y += m_lineHeight;
    }
}

void TerminalWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateVisibleRange();
}

void TerminalWidget::wheelEvent(QWheelEvent* event)
{
    // 鼠标滚轮滚动
    int delta = event->angleDelta().y();
    int scrollLines = delta / 120 * 3;  // 每次滚轮滚动3行

    m_scrollOffset -= scrollLines;
    m_scrollOffset = qMax(0, qMin(m_scrollOffset, m_maxScrollOffset));

    // 滚动时关闭自动滚动
    if (delta < 0 && m_scrollOffset < m_maxScrollOffset) {
        m_autoScroll = false;
    }

    update();
    event->accept();
}

void TerminalWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelecting = true;
        int line = m_scrollOffset + event->position().y() / m_lineHeight;
        m_selectionStartLine = line;
        m_selectionEndLine = line;
        update();
    }
    QWidget::mousePressEvent(event);
}

void TerminalWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isSelecting) {
        int line = m_scrollOffset + event->position().y() / m_lineHeight;
        m_selectionEndLine = line;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelecting = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void TerminalWidget::keyPressEvent(QKeyEvent* event)
{
    // Ctrl+C 复制选中内容
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        QString text = selectedText();
        if (!text.isEmpty()) {
            QApplication::clipboard()->setText(text);
        }
        return;
    }
    QWidget::keyPressEvent(event);
}

void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    Q_UNUSED(firstNewLine);
    Q_UNUSED(count);

    // 更新最大滚动偏移
    if (m_model) {
        int totalLines = m_model->lineCount();
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }
    }
    update();
}

void TerminalWidget::onDataCleared()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_scrollOffset = 0;
    m_maxScrollOffset = 0;
    m_selectionStartLine = -1;
    m_selectionEndLine = -1;
    update();
}

void TerminalWidget::updateVisibleRange()
{
    m_visibleLines = height() / m_lineHeight;
    if (m_model) {
        m_maxScrollOffset = qMax(0, m_model->lineCount() - m_visibleLines);
        m_scrollOffset = qMin(m_scrollOffset, m_maxScrollOffset);
    }
}

QString TerminalWidget::formatLine(const TerminalLine& line) const
{
    switch (m_displayMode) {
    case DisplayMode::Hex:
        return HexConverter::toHexString(line.data);
    case DisplayMode::Mixed:
        return QString::fromUtf8(line.data) + "  |  " + HexConverter::toHexString(line.data);
    case DisplayMode::Text:
    default:
        return QString::fromUtf8(line.data);
    }
}
