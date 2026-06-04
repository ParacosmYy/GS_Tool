/**
 * @file NumericDisplayWidgetPaint.cpp
 * @brief 数值显示控件 — paintEvent 绘制实现
 *
 * 从 NumericDisplayWidget.cpp 拆分而来，包含背景矩形、
 * 标签、大号数值和单位后缀的完整绘制逻辑。
 */

#include "dashboard/NumericDisplayWidget.h"

#include <QPainter>

#include "core/theme/ThemeManager.h"

/**
 * @brief 绘制数值显示区域
 *
 * 绘制流程：背景矩形 → 标签 → 大号数值 → 单位后缀
 * @param event 绘制事件参数
 */
void NumericDisplayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 8;
    QRectF widgetRect = this->rect().adjusted(margin, margin, -margin, -margin);
    if (widgetRect.width() <= 0 || widgetRect.height() <= 0) {
        painter.end();
        return;
    }

    /* --- 1. 背景 --- */
    painter.setPen(Qt::NoPen);
    auto& theme = ThemeManager::instance();
    painter.setBrush(theme.color(ThemeManager::SemanticColor::BgSecondary));
    painter.drawRoundedRect(widgetRect, 8.0, 8.0);

    /* --- 2. 标签（顶部） --- */
    qreal labelHeight = widgetRect.height() * 0.25;

    /* 从 channelName 或留空 — 标签在上方 */
    QFont labelFont = font();
    labelFont.setPointSize(qMax(8, static_cast<int>(widgetRect.height() / 8)));
    painter.setFont(labelFont);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextSecondary));

    QRectF labelRect(widgetRect.left() + 6, widgetRect.top() + 2,
                     widgetRect.width() - 12, labelHeight);

    /* 优先显示 channelName 作为标签，否则显示对象名 */
    QString displayLabel = m_channelName.isEmpty()
                               ? QString() : m_channelName;
    painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignTop, displayLabel);

    /* --- 3. 大号数值文本 --- */
    qreal valueAreaTop = widgetRect.top() + labelHeight;
    qreal valueAreaHeight = widgetRect.height() - labelHeight;
    QRectF valueRect(widgetRect.left() + 6, valueAreaTop,
                     widgetRect.width() - 12, valueAreaHeight);

    QFont valueFont = font();
    int fontSize = qMax(14, static_cast<int>(valueAreaHeight * 0.65));
    valueFont.setPointSize(fontSize);
    valueFont.setBold(true);
    painter.setFont(valueFont);
    painter.setPen(theme.color(ThemeManager::SemanticColor::Accent));

    QString valueText = QString::number(m_value, 'f', m_precision);
    painter.drawText(valueRect, Qt::AlignCenter, valueText);

    /* --- 4. 单位后缀（绘制在数值右侧） --- */
    if (!m_unit.isEmpty()) {
        QFont unitFont = font();
        unitFont.setPointSize(qMax(9, fontSize / 2));
        painter.setFont(unitFont);
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));

        /* 计算数值文本宽度，在其右侧绘制单位 */
        QFontMetrics fm(valueFont);
        qreal textWidth = fm.horizontalAdvance(valueText);

        QRectF unitRect(valueRect.center().x() + textWidth / 2 + 4,
                        valueRect.center().y() - fontSize / 2,
                        widgetRect.width() - textWidth / 2 - 20,
                        fontSize);
        painter.drawText(unitRect, Qt::AlignLeft | Qt::AlignVCenter, m_unit);
    }

    painter.end();
}

/**
 * @brief 重置所有统计计数器为零
 */
void NumericDisplayWidget::resetStatistics()
{
    m_stats = Stats{};
}
