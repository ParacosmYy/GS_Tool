/**
 * @file WaterfallWidget.cpp
 * @brief 瀑布图控件实现 — 频谱数据的时频二维可视化
 *
 * 将连续的频谱数据按时间轴纵向滚动显示，
 * 使用冷暖色渐变映射幅度值，支持暂停/恢复和滚动速度调节。
 */

#include "chart/waterfall/WaterfallWidget.h"
#include "core/theme/ThemeManager.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QLinearGradient>

/** @brief 构造瀑布图控件，初始化鼠标追踪和滚动定时器 @param parent 父控件指针 */
WaterfallWidget::WaterfallWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("WaterfallWidget");
    setMouseTracking(true);
    setMinimumSize(200, 150);
    connect(&m_scrollTimer, &QTimer::timeout, this, &WaterfallWidget::scrollImage);
    m_scrollTimer.setInterval(m_scrollSpeed);
}

/** @brief 析构函数，使用默认实现 */
WaterfallWidget::~WaterfallWidget() = default;

/** @brief 添加一帧频谱数据到历史记录并绘制新行 @param spectrum 频谱幅度值向量 */
void WaterfallWidget::addSpectrum(const QVector<double> &spectrum)
{
    if (m_paused) { m_totalSpectrumsDropped++; return; }
    m_totalSpectrumsAdded++;
    m_totalUpdates++;
    ++m_totalFrameUpdates;
    if (static_cast<quint64>(spectrum.size()) > m_peakDataPoints) m_peakDataPoints = spectrum.size();
    if (spectrum.size() > m_peakSpectrumWidth) m_peakSpectrumWidth = spectrum.size();
    m_history.append(spectrum);
    if (m_history.size() > m_maxLines) {
        m_history.removeFirst();
    }
    m_currentLine = m_history.size() - 1;

    // Draw new line at bottom
    if (m_waterfall.isNull()) {
        m_waterfall = QPixmap(width(), m_maxLines);
        m_waterfall.fill(ThemeManager::instance().color(ThemeManager::SemanticColor::BgPrimary));
    }
    QPainter p(&m_waterfall);
    int y = m_currentLine % m_maxLines;
    double range = m_maxValue - m_minValue;
    if (range <= 0) range = 1.0;
    int binW = qMax(1, width() / spectrum.size());
    for (int i = 0; i < spectrum.size(); ++i) {
        QColor c = valueToColor(spectrum[i]);
        p.setPen(c);
        p.drawLine(i * binW, y, (i + 1) * binW, y);
    }
    emit spectrumAdded(m_history.size());
    update();
}

/** @brief 设置瀑布图最大显示行数 @param lines 行数(最小10) */
void WaterfallWidget::setMaxLines(int lines)
{
    m_maxLines = qMax(10, lines);
    while (m_history.size() > m_maxLines) m_history.removeFirst();
    m_waterfall = QPixmap();
    update();
}

/** @brief 设置颜色映射的值域范围 @param min 值域下界 @param max 值域上界 */
void WaterfallWidget::setColorRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
    m_totalColorMapChanges++;
}

/** @brief 设置滚动刷新间隔 @param ms 间隔毫秒数(最小10ms) */
void WaterfallWidget::setScrollSpeed(int ms)
{
    m_scrollSpeed = qMax(10, ms);
    m_scrollTimer.setInterval(m_scrollSpeed);
}

/** @brief 清除所有历史数据和缓存像素图 */
void WaterfallWidget::clear()
{
    ++m_totalClears;
    m_history.clear();
    m_currentLine = 0;
    m_waterfall = QPixmap();
    update();
}

/** @brief 暂停瀑布图数据接收和滚动 */
void WaterfallWidget::pause() { ++m_totalPauses; m_paused = true; m_scrollTimer.stop(); }

/** @brief 恢复瀑布图数据接收和滚动 */
void WaterfallWidget::resume() { ++m_totalResumes; m_paused = false; m_scrollTimer.start(); }

/** @brief 绘制事件处理，将缓存瀑布图缩放绘制到控件上 @param event 绘制事件参数(未使用) */
void WaterfallWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    m_totalRepaints++;
    ++m_totalRenders;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    if (m_waterfall.isNull()) {
        p.fillRect(rect(), ThemeManager::instance().color(ThemeManager::SemanticColor::BgPrimary));
        p.setPen(ThemeManager::instance().color(ThemeManager::SemanticColor::TextMuted));
        p.drawText(rect(), Qt::AlignCenter, tr("无数据"));
        return;
    }
    p.drawPixmap(0, 0, m_waterfall.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

/** @brief 窗口大小变更事件处理，清除缓存以触发重绘 @param event 大小变更事件参数 */
void WaterfallWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_waterfall = QPixmap();
}

/** @brief 鼠标移动事件处理，计算光标处的频率索引和幅度值并发送信号 @param event 鼠标事件参数 */
void WaterfallWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    int col = x * (m_history.isEmpty() ? 1 : m_history[0].size()) / width();
    int row = y * m_maxLines / height();
    if (row >= 0 && row < m_history.size() && col >= 0 && col < m_history[row].size()) {
        m_totalCursorQueries++;
        emit valueAtCursor(col, m_history[row][col]);
    }
}

/** @brief 滚动定时器回调，触发控件重绘 */
void WaterfallWidget::scrollImage()
{
    m_totalScrolls++;
    ++m_totalScrollEvents;
    update();
}

/** @brief 将数值映射为颜色，黑→蓝→青→绿→黄→红渐变 @param value 待映射的数值 @return 对应的QColor颜色 */
QColor WaterfallWidget::valueToColor(double value) const
{
    double t = (value - m_minValue) / (m_maxValue - m_minValue);
    t = qBound(0.0, t, 1.0);
    // Cool-to-hot: black -> blue -> cyan -> green -> yellow -> red
    int r, g, b;
    if (t < 0.25) {
        r = 0; g = 0; b = static_cast<int>(t * 4 * 255);
    } else if (t < 0.5) {
        r = 0; g = static_cast<int>((t - 0.25) * 4 * 255); b = static_cast<int>((0.5 - t) * 4 * 255);
    } else if (t < 0.75) {
        r = static_cast<int>((t - 0.5) * 4 * 255); g = 255; b = 0;
    } else {
        r = 255; g = static_cast<int>((1.0 - t) * 4 * 255); b = 0;
    }
    return QColor(r, g, b);
}

/** @brief 重置所有统计计数器 */
void WaterfallWidget::resetStats()
{
    m_totalSpectrumsAdded = 0;
    m_totalSpectrumsDropped = 0;
    m_peakSpectrumWidth = 0;
    m_totalCursorQueries = 0;
    m_totalRepaints = 0;
    m_totalUpdates = 0;
    m_totalScrolls = 0;
    m_totalColorMapChanges = 0;
    m_peakDataPoints = 0;
    m_totalFrameUpdates = 0;
    m_totalScrollEvents = 0;
    m_totalRenders = 0;
    m_totalPauses = 0;
    m_totalResumes = 0;
    m_totalClears = 0;
}
