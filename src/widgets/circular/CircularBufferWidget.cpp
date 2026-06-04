/**
 * @file CircularBufferWidget.cpp
 * @brief 环形缓冲区可视化控件实现 -- 状态管理接口
 *
 * 包含构造/析构、setCapacity/setUsed/setReadPos/setWritePos/markOverflow
 * 等状态管理方法及查询接口。paintEvent 绘制逻辑在本文件内。
 * Stats 重置逻辑在 CircularBufferWidgetStats.cpp。
 */

#include "widgets/circular/CircularBufferWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include "core/theme/ThemeManager.h"

// ==================== 构造 / 析构 ====================

/** @brief 构造函数，初始化控件属性 @param parent 父Widget */
CircularBufferWidget::CircularBufferWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("CircularBufferWidget");
}

/** @brief 析构函数 */
CircularBufferWidget::~CircularBufferWidget() = default;

// ==================== 状态设置接口 ====================

/**
 * @brief 设置缓冲区总容量
 * @param capacity 缓冲区容量(字节)，钳位到 >= 1
 */
void CircularBufferWidget::setCapacity(int capacity)
{
    m_capacity = qMax(1, capacity);
    m_used = qBound(0, m_used, m_capacity);
    m_readPos = qBound(0, m_readPos, m_capacity - 1);
    m_writePos = qBound(0, m_writePos, m_capacity - 1);
    ++m_stats.totalCapacityChanges;
    ++m_stats.totalUpdates;
    update();
}

/**
 * @brief 设置当前已使用字节数
 * @param used 已使用字节数，自动钳位到 [0, capacity]
 */
void CircularBufferWidget::setUsed(int used)
{
    m_used = qBound(0, used, m_capacity);
    if (m_used > m_stats.peakUsed) {
        m_stats.peakUsed = m_used;
    }
    m_stats.cumulativePercent += fillPercent();
    ++m_stats.totalUpdates;
    update();
}

/**
 * @brief 设置读指针位置
 * @param pos 读指针偏移量
 */
void CircularBufferWidget::setReadPos(int pos)
{
    m_readPos = qBound(0, pos, m_capacity - 1);
    ++m_stats.totalPointerMoves;
    ++m_stats.totalUpdates;
    update();
}

/**
 * @brief 设置写指针位置
 * @param pos 写指针偏移量
 */
void CircularBufferWidget::setWritePos(int pos)
{
    m_writePos = qBound(0, pos, m_capacity - 1);
    ++m_stats.totalPointerMoves;
    ++m_stats.totalUpdates;
    update();
}

/** @brief 标记一次溢出事件 */
void CircularBufferWidget::markOverflow()
{
    ++m_overflowCount;
    ++m_stats.totalOverflows;
    update();
}

/** @brief 重置溢出计数器为零 */
void CircularBufferWidget::resetOverflowCount()
{
    m_overflowCount = 0;
    update();
}

// ==================== 状态查询接口 ====================

/** @brief 获取缓冲区容量 @return 容量(字节) */
int CircularBufferWidget::capacity() const { return m_capacity; }

/** @brief 获取当前使用量 @return 已用字节数 */
int CircularBufferWidget::used() const { return m_used; }

/** @brief 获取读指针位置 @return 读偏移量 */
int CircularBufferWidget::readPos() const { return m_readPos; }

/** @brief 获取写指针位置 @return 写偏移量 */
int CircularBufferWidget::writePos() const { return m_writePos; }

/** @brief 获取累计溢出次数 @return 溢出计数 */
quint64 CircularBufferWidget::overflowCount() const { return m_overflowCount; }

/**
 * @brief 获取当前填充率百分比
 * @return 0.0 ~ 100.0，容量为0时返回0.0
 */
double CircularBufferWidget::fillPercent() const
{
    if (m_capacity <= 0) return 0.0;
    return static_cast<double>(m_used) / static_cast<double>(m_capacity) * 100.0;
}

/** @brief 获取统计数据的只读引用 @return Stats常量引用 */
const CircularBufferWidget::Stats& CircularBufferWidget::stats() const
{
    return m_stats;
}

/** @brief 建议最小尺寸 @return 120x120像素 */
QSize CircularBufferWidget::minimumSizeHint() const
{
    return QSize(120, 120);
}

// ==================== 绘制 ====================

/**
 * @brief 绘制环形缓冲区可视化图形
 *
 * 绘制流程:
 *   1. 背景圆环(环形底色)
 *   2. 填充弧线(强调色，按填充率比例)
 *   3. 读指针三角标记(绿色)
 *   4. 写指针三角标记(红色)
 *   5. 中心使用率百分比文本
 *   6. 底部容量/溢出信息文本
 *
 * 环形起止角度: 顶部(270度) 顺时针，与Qt坐标系一致
 * @param event 绘制事件
 */
void CircularBufferWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    ++m_stats.totalRenders;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /* 获取有效绘制区域(留边距给外部标签) */
    const int margin = 24;
    QRectF drawRect = rect().adjusted(margin, margin, -margin, -margin);
    if (drawRect.width() <= 0 || drawRect.height() <= 0) {
        painter.end();
        return;
    }

    auto& theme = ThemeManager::instance();

    /* 取正方形区域中心和半径 */
    qreal side = qMin(drawRect.width(), drawRect.height());
    QPointF center = drawRect.center();
    qreal outerRadius = side / 2.0;
    qreal ringWidth = outerRadius * 0.18;          ///< 环形宽度(外径的18%)
    qreal innerRadius = outerRadius - ringWidth;    ///< 内径

    /* --- 1. 背景圆环(环形底色) --- */
    QRectF outerRect(center.x() - outerRadius, center.y() - outerRadius,
                     outerRadius * 2, outerRadius * 2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(theme.color(ThemeManager::SemanticColor::BgTertiary));
    painter.drawEllipse(outerRect);

    /* 挖出内圆形成甜甜圈 */
    painter.setBrush(theme.color(ThemeManager::SemanticColor::BgPrimary));
    QRectF innerRect(center.x() - innerRadius, center.y() - innerRadius,
                     innerRadius * 2, innerRadius * 2);
    painter.drawEllipse(innerRect);

    /* --- 2. 填充弧线(强调色) --- */
    if (m_capacity > 0 && m_used > 0) {
        double fraction = qBound(0.0, static_cast<double>(m_used) / m_capacity, 1.0);
        /* Qt弧度: 起始角度6点钟位置为0，逆时针为正；这里从12点钟(90度)开始顺时针 */
        int startAngle16 = 90 * 16;   ///< 12点钟位置
        int spanAngle16 = -static_cast<int>(fraction * 360.0 * 16);  ///< 负数=顺时针

        /* 根据填充率选择颜色: <70% 强调色, 70~90% 警告色, >90% 错误色 */
        QColor fillColor;
        if (fraction < 0.7) {
            fillColor = theme.color(ThemeManager::SemanticColor::Accent);
        } else if (fraction < 0.9) {
            fillColor = theme.color(ThemeManager::SemanticColor::Warning);
        } else {
            fillColor = theme.color(ThemeManager::SemanticColor::Error);
        }

        /* 绘制填充弧 — 使用ConicalGradient在环形上绘制 */
        QPainterPath fillPath;
        fillPath.addEllipse(outerRect);
        fillPath.addEllipse(innerRect);

        painter.save();
        painter.setClipPath(fillPath);

        /* 旋转坐标系使得0度在顶部 */
        painter.translate(center);
        painter.rotate(-90);  ///< Qt默认0度在3点钟，旋转到12点钟

        painter.setPen(Qt::NoPen);
        painter.setBrush(fillColor);
        painter.drawPie(QRectF(-outerRadius, -outerRadius, outerRadius * 2, outerRadius * 2),
                        0, static_cast<int>(fraction * 360 * 16));

        painter.restore();
    }

    /* --- 3. 读指针标记(绿色三角) --- */
    if (m_capacity > 0) {
        QColor readColor = theme.color(ThemeManager::SemanticColor::Success);
        double readFraction = static_cast<double>(m_readPos) / m_capacity;
        qreal readAngleDeg = -90.0 + readFraction * 360.0;  ///< 从顶部顺时针
        qreal readAngleRad = qDegreesToRadians(readAngleDeg);
        qreal markerR = outerRadius + 6;  ///< 指针在环外侧
        qreal markerSize = qMax(4.0, side * 0.03);

        QPointF readTip(center.x() + markerR * qCos(readAngleRad),
                        center.y() + markerR * qSin(readAngleRad));
        QPointF readBase1(center.x() + (outerRadius - 1) * qCos(readAngleRad) - markerSize * qSin(readAngleRad),
                          center.y() + (outerRadius - 1) * qSin(readAngleRad) + markerSize * qCos(readAngleRad));
        QPointF readBase2(center.x() + (outerRadius - 1) * qCos(readAngleRad) + markerSize * qSin(readAngleRad),
                          center.y() + (outerRadius - 1) * qSin(readAngleRad) - markerSize * qCos(readAngleRad));

        QPainterPath readTriangle;
        readTriangle.moveTo(readTip);
        readTriangle.lineTo(readBase1);
        readTriangle.lineTo(readBase2);
        readTriangle.closeSubpath();

        painter.setPen(Qt::NoPen);
        painter.setBrush(readColor);
        painter.drawPath(readTriangle);

        /* --- 4. 写指针标记(红色三角) --- */
        QColor writeColor = theme.color(ThemeManager::SemanticColor::Error);
        double writeFraction = static_cast<double>(m_writePos) / m_capacity;
        qreal writeAngleDeg = -90.0 + writeFraction * 360.0;
        qreal writeAngleRad = qDegreesToRadians(writeAngleDeg);
        qreal writeMarkerR = outerRadius + 6;
        qreal writeMarkerSize = qMax(4.0, side * 0.03);

        QPointF writeTip(center.x() + writeMarkerR * qCos(writeAngleRad),
                         center.y() + writeMarkerR * qSin(writeAngleRad));
        QPointF writeBase1(center.x() + (outerRadius - 1) * qCos(writeAngleRad) - writeMarkerSize * qSin(writeAngleRad),
                           center.y() + (outerRadius - 1) * qSin(writeAngleRad) + writeMarkerSize * qCos(writeAngleRad));
        QPointF writeBase2(center.x() + (outerRadius - 1) * qCos(writeAngleRad) + writeMarkerSize * qSin(writeAngleRad),
                           center.y() + (outerRadius - 1) * qSin(writeAngleRad) - writeMarkerSize * qCos(writeAngleRad));

        QPainterPath writeTriangle;
        writeTriangle.moveTo(writeTip);
        writeTriangle.lineTo(writeBase1);
        writeTriangle.lineTo(writeBase2);
        writeTriangle.closeSubpath();

        painter.setBrush(writeColor);
        painter.drawPath(writeTriangle);
    }

    /* --- 5. 中心使用率百分比文本 --- */
    QColor textColor = theme.color(ThemeManager::SemanticColor::TextPrimary);
    QFont percentFont = font();
    percentFont.setPointSize(qMax(12, static_cast<int>(side / 7)));
    percentFont.setBold(true);
    painter.setFont(percentFont);
    painter.setPen(textColor);

    int percentValue = qBound(0, static_cast<int>(fillPercent() + 0.5), 100);
    QString percentText = QString("%1%").arg(percentValue);
    QRectF textRect(center.x() - innerRadius, center.y() - innerRadius * 0.5,
                    innerRadius * 2, innerRadius);
    painter.drawText(textRect, Qt::AlignCenter, percentText);

    /* --- 6. 底部容量/溢出信息 --- */
    QFont infoFont = font();
    infoFont.setPointSize(qMax(7, static_cast<int>(side / 20)));
    painter.setFont(infoFont);

    /* 容量文本 */
    QString capText = tr("%1 / %2 bytes").arg(m_used).arg(m_capacity);
    QRectF capRect(rect().left(), rect().bottom() - margin + 4, rect().width(), margin - 4);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextSecondary));
    painter.drawText(capRect, Qt::AlignCenter, capText);

    /* 溢出计数(有溢出时显示警告色) */
    if (m_overflowCount > 0) {
        QString overflowText = tr("Overflow: %1").arg(m_overflowCount);
        QRectF ofRect(center.x() - innerRadius, center.y() + innerRadius * 0.05,
                      innerRadius * 2, innerRadius * 0.4);
        painter.setPen(theme.color(ThemeManager::SemanticColor::Warning));
        painter.drawText(ofRect, Qt::AlignCenter, overflowText);
    }

    /* 指针图例 */
    qreal legendY = center.y() + innerRadius * 0.45;
    qreal legendSpacing = innerRadius * 0.6;
    painter.setFont(infoFont);

    /* R(读)标记 */
    painter.setPen(Qt::NoPen);
    painter.setBrush(theme.color(ThemeManager::SemanticColor::Success));
    painter.drawEllipse(QPointF(center.x() - legendSpacing, legendY), 3, 3);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));
    painter.drawText(QPointF(center.x() - legendSpacing + 6, legendY + 4), "R");

    /* W(写)标记 */
    painter.setPen(Qt::NoPen);
    painter.setBrush(theme.color(ThemeManager::SemanticColor::Error));
    painter.drawEllipse(QPointF(center.x() + legendSpacing - 20, legendY), 3, 3);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));
    painter.drawText(QPointF(center.x() + legendSpacing - 14, legendY + 4), "W");

    painter.end();
}

// resetStatistics 实现在 CircularBufferWidgetStats.cpp
