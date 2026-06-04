/**
 * @file MiniChartWidgetPaint.cpp
 * @brief 仪表盘迷你折线图控件 — paintEvent 绘制逻辑实现
 *
 * 从 MiniChartWidget.cpp 拆分而来，包含:
 *   1. 圆角背景矩形
 *   2. 水平网格线（3条辅助线）
 *   3. 平滑折线曲线（QPainterPath quadTo）
 *   4. 折线下方渐变填充
 *   5. 右上角当前值 + 单位文本
 *   6. 底部标签文本
 * 所有颜色通过 ThemeManager 语义色获取，禁止硬编码。
 */

#include "dashboard/MiniChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QtMath>

#include "core/theme/ThemeManager.h"

/**
 * @brief 绘制迷你折线图
 *
 * 绘制流程：背景 → 网格线 → 渐变填充 → 折线 → 当前值 → 标签
 * @param event 绘制事件参数
 */
void MiniChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    ++m_stats.totalRenders;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 6;
    QRectF widgetRect = rect().adjusted(margin, margin, -margin, -margin);
    if (widgetRect.width() <= 0 || widgetRect.height() <= 0) {
        painter.end();
        return;
    }

    auto &theme = ThemeManager::instance();

    /* --- 1. 圆角背景矩形 --- */
    painter.setPen(Qt::NoPen);
    painter.setBrush(theme.color(ThemeManager::SemanticColor::BgSecondary));
    painter.drawRoundedRect(widgetRect, 8.0, 8.0);

    /* --- 计算绘制区域与Y轴范围 --- */
    qreal labelHeight = widgetRect.height() * 0.20;
    QRectF chartArea(widgetRect.left() + 4,
                     widgetRect.top() + 4,
                     widgetRect.width() - 8,
                     widgetRect.height() - labelHeight - 8);

    QPair<double, double> yRange = m_autoRange
        ? computeVisibleRange()
        : qMakePair(m_min, m_max);
    double yMin = yRange.first;
    double yMax = yRange.second;
    double yRangeSpan = yMax - yMin;
    if (qFuzzyIsNull(yRangeSpan)) {
        yRangeSpan = 1.0;
    }

    /* --- 2. 水平网格线（3条辅助线） --- */
    QPen gridPen(theme.color(ThemeManager::SemanticColor::Border));
    gridPen.setStyle(Qt::DotLine);
    gridPen.setWidthF(0.8);
    painter.setPen(gridPen);

    int gridLines = 3;
    for (int i = 1; i <= gridLines; ++i) {
        qreal fraction = static_cast<qreal>(i) / (gridLines + 1);
        qreal y = chartArea.bottom() - fraction * chartArea.height();
        painter.drawLine(QPointF(chartArea.left(), y),
                         QPointF(chartArea.right(), y));
    }

    /* 无数据时仅绘制背景和网格 */
    if (m_count < 2) {
        painter.end();
        return;
    }

    /* --- 3. 计算数据点的屏幕坐标 --- */
    QVector<QPointF> points;
    points.reserve(m_count);

    qreal stepX = chartArea.width() / static_cast<qreal>(kMaxPoints - 1);

    for (int i = 0; i < m_count; ++i) {
        int idx = (m_head - m_count + i + kMaxPoints) % kMaxPoints;
        qreal x = chartArea.left() + i * stepX;
        qreal normalizedY = (m_data[idx] - yMin) / yRangeSpan;
        qreal y = chartArea.bottom() - normalizedY * chartArea.height();
        /* 将Y坐标限制在图表区域内 */
        y = qBound(chartArea.top(), y, chartArea.bottom());
        points.append(QPointF(x, y));
    }

    /* --- 4. 平滑折线 + 渐变填充 --- */
    QPainterPath linePath;
    linePath.moveTo(points[0]);

    for (int i = 1; i < points.size(); ++i) {
        /* 使用二次贝塞尔曲线实现平滑过渡 */
        QPointF prev = points[i - 1];
        QPointF curr = points[i];
        QPointF ctrl((prev.x() + curr.x()) / 2.0,
                     (prev.y() + curr.y()) / 2.0);
        linePath.quadTo(prev, ctrl);
    }
    /* 最后一段直接连接到末尾点 */
    linePath.lineTo(points.last());

    /* 渐变填充区域：折线路径 → 底边闭合 */
    QPainterPath fillPath = linePath;
    fillPath.lineTo(points.last().x(), chartArea.bottom());
    fillPath.lineTo(points.first().x(), chartArea.bottom());
    fillPath.closeSubpath();

    QLinearGradient gradient(0, chartArea.top(), 0, chartArea.bottom());
    QColor accentColor = theme.color(ThemeManager::SemanticColor::Accent);
    QColor fillTop = QColor(accentColor.red(), accentColor.green(),
                            accentColor.blue(), 80);
    QColor fillBottom = QColor(accentColor.red(), accentColor.green(),
                               accentColor.blue(), 10);
    gradient.setColorAt(0.0, fillTop);
    gradient.setColorAt(1.0, fillBottom);

    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawPath(fillPath);

    /* 折线描边 */
    QPen linePen(accentColor, 1.8);
    linePen.setCapStyle(Qt::RoundCap);
    linePen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(linePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(linePath);

    /* --- 5. 末端数据点圆点 --- */
    painter.setPen(Qt::NoPen);
    painter.setBrush(accentColor);
    painter.drawEllipse(points.last(), 3.0, 3.0);

    /* --- 6. 右上角当前值 + 单位 --- */
    QFont valueFont = font();
    int fontSize = qMax(9, static_cast<int>(chartArea.height() / 6));
    valueFont.setPointSize(fontSize);
    valueFont.setBold(true);
    painter.setFont(valueFont);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextPrimary));

    QString valueText = QString::number(m_value, 'f', 2);
    if (!m_unit.isEmpty()) {
        valueText += QStringLiteral(" ") + m_unit;
    }
    QRectF valueRect(chartArea.right() - 80, chartArea.top(),
                     76, fontSize + 6);
    painter.drawText(valueRect, Qt::AlignRight | Qt::AlignTop, valueText);

    /* --- 7. 底部标签 --- */
    if (!m_label.isEmpty()) {
        QFont labelFont = font();
        labelFont.setPointSize(qMax(7, static_cast<int>(labelHeight * 0.55)));
        painter.setFont(labelFont);
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextSecondary));

        QRectF labelRect(widgetRect.left() + 6,
                         widgetRect.bottom() - labelHeight,
                         widgetRect.width() - 12,
                         labelHeight);
        painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, m_label);
    }

    painter.end();
}
