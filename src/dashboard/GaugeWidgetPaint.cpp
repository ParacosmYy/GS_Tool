/**
 * @file GaugeWidgetPaint.cpp
 * @brief 仪表盘量表控件 - 绘制逻辑实现
 *
 * 从 GaugeWidget.cpp 拆分而来，包含 paintEvent 绘制方法:
 * 外圆弧、刻度线、指针三角形、数值文本和标签。
 */

#include "dashboard/GaugeWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include "core/theme/ThemeManager.h"

/**
 * @brief 绘制仪表盘
 *
 * 绘制流程：外圆弧 → 刻度线 → 指针 → 数值文本 → 标签
 * 弧度范围 135° ~ 405°（共 270°）
 * @param event 绘制事件参数
 */
void GaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 20;
    QRectF rect = this->rect().adjusted(margin, margin, -margin, -margin);
    if (rect.width() <= 0 || rect.height() <= 0) {
        painter.end();
        return;
    }

    /* 取正方形区域的中心与半径 */
    qreal side = qMin(rect.width(), rect.height());
    QPointF center = rect.center();
    qreal radius = side / 2.0;

    /* --- 颜色定义 --- */
    auto& theme = ThemeManager::instance();
    QPen arcPen(theme.color(ThemeManager::SemanticColor::Border), 2.0);
    QPen tickPen(theme.color(ThemeManager::SemanticColor::TextPrimary), 1.5);
    QPen minorTickPen(theme.color(ThemeManager::SemanticColor::Border), 1.0);
    QPen needlePen(theme.color(ThemeManager::SemanticColor::Error), 2.0);
    QPen textPen(theme.color(ThemeManager::SemanticColor::TextPrimary));

    /* --- 1. 外圆弧 (135° → 405°, Qt 角度 = 1/16°) --- */
    QRectF arcRect(center.x() - radius, center.y() - radius,
                   radius * 2, radius * 2);
    painter.setPen(arcPen);
    painter.drawArc(arcRect, 135 * 16, 270 * 16);

    /* --- 2. 刻度线 --- */
    qreal arcSpanDeg = 270.0;
    qreal startAngleDeg = 135.0;
    int majorSteps = 5;   /* 每 20% 画大刻度 */
    int minorSteps = 10;  /* 每 10% 画小刻度 */

    for (int i = 0; i <= minorSteps; ++i) {
        qreal fraction = static_cast<qreal>(i) / minorSteps;
        qreal angleDeg = startAngleDeg + fraction * arcSpanDeg;
        qreal angleRad = qDegreesToRadians(angleDeg);

        bool isMajor = (i % (minorSteps / majorSteps) == 0);
        qreal outerR = radius;
        qreal innerR = isMajor ? radius - 15 : radius - 8;

        qreal x1 = center.x() + outerR * qCos(angleRad);
        qreal y1 = center.y() - outerR * qSin(angleRad);
        qreal x2 = center.x() + innerR * qCos(angleRad);
        qreal y2 = center.y() - innerR * qSin(angleRad);

        painter.setPen(isMajor ? tickPen : minorTickPen);
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }

    /* --- 3. 最小/最大标签 --- */
    QFont smallFont = font();
    smallFont.setPointSize(qMax(7, static_cast<int>(side / 18)));
    painter.setFont(smallFont);
    painter.setPen(textPen);

    qreal range = m_max - m_min;
    if (qFuzzyIsNull(range)) {
        range = 1.0;
    }

    /* min 标签 (135° 位置) */
    qreal minAngleRad = qDegreesToRadians(startAngleDeg);
    qreal labelR = radius + 12;
    QPointF minPos(center.x() + labelR * qCos(minAngleRad),
                   center.y() - labelR * qSin(minAngleRad));
    painter.drawText(minPos, QString::number(m_min, 'f', 0));

    /* max 标签 (405° 位置) */
    qreal maxAngleRad = qDegreesToRadians(startAngleDeg + arcSpanDeg);
    QPointF maxPos(center.x() + labelR * qCos(maxAngleRad),
                   center.y() - labelR * qSin(maxAngleRad));
    painter.drawText(maxPos, QString::number(m_max, 'f', 0));

    /* --- 4. 指针 --- */
    qreal clamped = qBound(m_min, m_value, m_max);
    qreal valueFraction = (clamped - m_min) / range;
    qreal needleAngleDeg = startAngleDeg + valueFraction * arcSpanDeg;
    qreal needleAngleRad = qDegreesToRadians(needleAngleDeg);

    qreal needleLen = radius * 0.75;
    qreal needleBase = 6.0;

    /* 指针三角形：顶点 + 两个底部角 */
    QPointF tip(center.x() + needleLen * qCos(needleAngleRad),
                center.y() - needleLen * qSin(needleAngleRad));

    qreal perpRad = needleAngleRad + M_PI / 2.0;
    QPointF base1(center.x() + needleBase * qCos(perpRad),
                  center.y() - needleBase * qSin(perpRad));
    QPointF base2(center.x() - needleBase * qCos(perpRad),
                  center.y() + needleBase * qSin(perpRad));

    QPainterPath needlePath;
    needlePath.moveTo(tip);
    needlePath.lineTo(base1);
    needlePath.lineTo(base2);
    needlePath.closeSubpath();

    painter.setPen(needlePen);
    painter.setBrush(theme.color(ThemeManager::SemanticColor::Error));
    painter.drawPath(needlePath);

    /* 中心圆点 */
    painter.setBrush(theme.color(ThemeManager::SemanticColor::TextPrimary));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, 4.0, 4.0);

    /* --- 5. 数值文本 --- */
    QFont valueFont = font();
    valueFont.setPointSize(qMax(10, static_cast<int>(side / 10)));
    valueFont.setBold(true);
    painter.setFont(valueFont);
    painter.setPen(textPen);

    QString valueText = QString::number(m_value, 'f', 1);
    QRectF valueRect(center.x() - 40, center.y() + radius * 0.3, 80, 30);
    painter.drawText(valueRect, Qt::AlignCenter, valueText);

    /* --- 6. 标签 --- */
    if (!m_label.isEmpty()) {
        QFont labelFont = font();
        labelFont.setPointSize(qMax(8, static_cast<int>(side / 16)));
        painter.setFont(labelFont);
        painter.setPen(textPen);
        QRectF labelRect(center.x() - 50, center.y() + radius * 0.6, 100, 20);
        painter.drawText(labelRect, Qt::AlignCenter, m_label);
    }

    painter.end();
}
