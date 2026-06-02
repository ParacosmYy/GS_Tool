/**
 * @file LedIndicatorWidget.cpp
 * @brief LED 指示灯控件实现
 *
 * 自定义 paintEvent 绘制圆形 LED 指示灯。
 * 亮时填充指定颜色并带发光效果，灭时填充深灰色。
 * 所有颜色通过 QPen/QBrush 设置。
 */

#include "dashboard/LedIndicatorWidget.h"

#include <QPainter>
#include <QRadialGradient>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
LedIndicatorWidget::LedIndicatorWidget(QWidget *parent)
    : QWidget(parent)
    , m_color(Qt::green)
{
    setObjectName("LedIndicatorWidget");
}

/**
 * @brief 设置开关状态
 * @param on true=亮，false=灭
 */
void LedIndicatorWidget::setOn(bool on)
{
    m_on = on;
    update();
}

/**
 * @brief 设置 LED 颜色
 * @param color 颜色
 */
void LedIndicatorWidget::setColor(const QColor &color)
{
    m_color = color;
    update();
}

/**
 * @brief 绑定数据通道名称
 * @param channelName 通道名称
 */
void LedIndicatorWidget::bindChannel(const QString &channelName)
{
    m_channelName = channelName;
}

/**
 * @brief 建议最小尺寸
 * @return 最小尺寸 40×40
 */
QSize LedIndicatorWidget::minimumSizeHint() const
{
    return QSize(40, 40);
}

/**
 * @brief 绘制 LED 指示灯
 *
 * 绘制流程：外圈边框 → 内部填充（亮色/灰色）→ 高光效果
 * @param event 绘制事件参数
 */
void LedIndicatorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 4;
    QRectF rect = this->rect().adjusted(margin, margin, -margin, -margin);
    qreal side = qMin(rect.width(), rect.height());
    if (side <= 0) {
        painter.end();
        return;
    }

    /* 居中 LED */
    QPointF center = this->rect().center();
    qreal radius = side / 2.0;

    /* --- 1. 外圈边框 --- */
    QPen borderPen(QColor(100, 100, 100), 2.0);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);

    /* --- 2. 内部填充 --- */
    QRectF innerRect(center.x() - radius + 2, center.y() - radius + 2,
                     (radius - 2) * 2, (radius - 2) * 2);

    if (m_on) {
        /* 亮状态：使用径向渐变模拟发光 */
        QRadialGradient gradient(center, radius);
        gradient.setColorAt(0.0, m_color.lighter(150));
        gradient.setColorAt(0.7, m_color);
        gradient.setColorAt(1.0, m_color.darker(130));
        painter.setBrush(gradient);
    } else {
        /* 灭状态：深灰色 */
        painter.setBrush(QColor(50, 50, 50));
    }

    painter.setPen(Qt::NoPen);
    painter.drawEllipse(innerRect);

    /* --- 3. 高光效果（小圆弧在左上角模拟反光） --- */
    if (m_on) {
        QPointF highlight(center.x() - radius * 0.3,
                          center.y() - radius * 0.3);
        qreal hlRadius = radius * 0.25;
        QRadialGradient hlGrad(highlight, hlRadius);
        hlGrad.setColorAt(0.0, QColor(255, 255, 255, 100));
        hlGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
        painter.setBrush(hlGrad);
        painter.drawEllipse(highlight, hlRadius, hlRadius);
    }

    painter.end();
}
