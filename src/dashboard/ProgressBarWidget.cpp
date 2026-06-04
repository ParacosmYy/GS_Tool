/**
 * @file ProgressBarWidget.cpp
 * @brief 进度条仪表盘控件实现
 *
 * 自定义 paintEvent 绘制水平圆角进度条、填充区域、数值与标签文本。
 * 所有颜色通过 QPen/QBrush 设置，不使用 setStyleSheet 硬编码颜色。
 */

#include "dashboard/ProgressBarWidget.h"

#include <QPainter>
#include "core/theme/ThemeManager.h"
#include <QtMath>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
ProgressBarWidget::ProgressBarWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("ProgressBarWidget");
}

/**
 * @brief 设置当前显示值
 * @param value 新值
 */
void ProgressBarWidget::setValue(double value)
{
    m_value = value;
    ++m_stats.totalValueUpdates;
    m_stats.cumulativeValue += value;
    if (m_max > m_min && value >= m_max) ++m_stats.totalCompleteEvents;
    update();
}

/**
 * @brief 设置量程范围
 * @param min 最小值
 * @param max 最大值
 */
void ProgressBarWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    ++m_stats.totalRangeChanges;
    update();
}

/**
 * @brief 设置标签文本
 * @param label 标签
 */
void ProgressBarWidget::setLabel(const QString &label)
{
    m_label = label;
    update();
}

/**
 * @brief 取消当前进度，值重置为起始值并递增取消计数
 */
void ProgressBarWidget::cancel()
{
    ++m_stats.totalCancelledEvents;
    m_value = m_min;
    update();
}

/**
 * @brief 绑定数据通道名称
 * @param channelName 通道名称
 */
void ProgressBarWidget::bindChannel(const QString &channelName)
{
    m_channelName = channelName;
}

/**
 * @brief 建议最小尺寸
 * @return 最小尺寸 150×50
 */
QSize ProgressBarWidget::minimumSizeHint() const
{
    return QSize(150, 50);
}

/**
 * @brief 绘制水平进度条
 *
 * 绘制流程：背景圆角矩形 → 填充区域 → 数值文本 → 标签
 * @param event 绘制事件参数
 */
void ProgressBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 8;
    int labelHeight = 18;
    int barTop = margin + labelHeight;
    int barHeight = height() - barTop - margin;
    if (barHeight < 8) {
        barHeight = 8;
    }
    qreal barRadius = barHeight / 2.0;

    QRectF barRect(margin, barTop, width() - 2 * margin, barHeight);

    /* --- 1. 背景圆角矩形 --- */
    painter.setPen(Qt::NoPen);
    auto& theme = ThemeManager::instance();
    painter.setBrush(theme.color(ThemeManager::SemanticColor::BgTertiary));
    painter.drawRoundedRect(barRect, barRadius, barRadius);

    /* --- 2. 填充区域 --- */
    qreal range = m_max - m_min;
    if (qFuzzyIsNull(range)) {
        range = 1.0;
    }
    qreal clamped = qBound(m_min, m_value, m_max);
    qreal fillRatio = (clamped - m_min) / range;
    qreal fillWidth = barRect.width() * fillRatio;

    if (fillWidth > 0) {
        QRectF fillRect(barRect.left(), barRect.top(),
                        fillWidth, barRect.height());
        painter.setBrush(theme.color(ThemeManager::SemanticColor::Success));
        painter.drawRoundedRect(fillRect, barRadius, barRadius);
    }

    /* --- 3. 数值文本（居中于 bar） --- */
    QFont valueFont = font();
    valueFont.setPointSize(qMax(9, barHeight / 3));
    valueFont.setBold(true);
    painter.setFont(valueFont);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextPrimary));

    QString valueText = QString::number(m_value, 'f', 1);
    painter.drawText(barRect, Qt::AlignCenter, valueText);

    /* --- 4. 标签（顶部左侧） --- */
    if (!m_label.isEmpty()) {
        QFont labelFont = font();
        labelFont.setPointSize(qMax(8, barHeight / 4));
        painter.setFont(labelFont);
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextPrimary));
        QRectF labelRect(margin, margin, width() - 2 * margin, labelHeight);
        painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, m_label);
    }

    painter.end();
}

/**
 * @brief 重置所有统计计数器为零
 */
void ProgressBarWidget::resetStatistics()
{
    m_stats = Stats{};
}

/** @brief 获取历史平均值 @return 平均值，无更新时返回0.0 */
double ProgressBarWidget::avgValue() const
{
    if (m_stats.totalValueUpdates == 0) return 0.0;
    return m_stats.cumulativeValue / static_cast<double>(m_stats.totalValueUpdates);
}
