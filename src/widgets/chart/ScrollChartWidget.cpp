/**
 * @file ScrollChartWidget.cpp
 * @brief 轻量级滚动折线图 — 多通道实时数据流可视化实现
 *
 * 管理200可见点数据窗口的多通道滚动折线图。
 * 统计重置逻辑见 ScrollChartWidgetStats.cpp。
 */

#include "widgets/chart/ScrollChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QFontMetrics>
#include <QtMath>

#include "core/theme/ThemeManager.h"

/**
 * @brief 构造函数 — 初始化默认参数
 * @param parent 父控件指针
 */
ScrollChartWidget::ScrollChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_visiblePoints(200)
    , m_autoScale(true)
    , m_yMin(0.0)
    , m_yMax(100.0)
    , m_gridEnabled(true)
    , m_lineWidth(1.5)
    , m_leftMargin(50)
    , m_bottomMargin(24)
{
    setObjectName("ScrollChartWidget");
}

/**
 * @brief 添加新数据通道
 * @param name 通道名称
 * @param color 折线颜色
 * @return 新通道的索引
 */
int ScrollChartWidget::addChannel(const QString& name, const QColor& color)
{
    Channel ch;
    ch.name = name;
    ch.color = color;
    ch.data.reserve(m_visiblePoints * 2);
    ch.minVal = 0.0;
    ch.maxVal = 0.0;
    ch.visible = true;

    m_channels.append(std::move(ch));
    const int index = m_channels.size() - 1;

    ++m_stats.totalChannelChanges;
    if (m_channels.size() > m_stats.maxChannels) {
        m_stats.maxChannels = m_channels.size();
    }

    emit channelAdded(index);
    update();
    return index;
}

/**
 * @brief 移除指定索引的数据通道
 * @param index 通道索引
 */
void ScrollChartWidget::removeChannel(int index)
{
    if (index < 0 || index >= m_channels.size()) {
        return;
    }

    m_channels.removeAt(index);
    ++m_stats.totalChannelChanges;

    emit channelRemoved(index);
    update();
}

/**
 * @brief 向指定通道追加一个数据点
 * @param channelIndex 通道索引
 * @param value 数据值
 */
void ScrollChartWidget::addSample(int channelIndex, double value)
{
    if (channelIndex < 0 || channelIndex >= m_channels.size()) {
        return;
    }

    Channel& ch = m_channels[channelIndex];
    ch.data.append(value);

    /* 限制缓冲区为2倍可见点数，超出时裁剪 */
    const int maxBuffer = m_visiblePoints * 2;
    if (ch.data.size() > maxBuffer) {
        ch.data.erase(ch.data.begin(), ch.data.begin() + (ch.data.size() - maxBuffer));
        emit dataOverflow(channelIndex);
    }

    /* 更新通道历史最小/最大值 */
    if (ch.data.size() == 1) {
        ch.minVal = value;
        ch.maxVal = value;
    } else {
        if (value < ch.minVal) ch.minVal = value;
        if (value > ch.maxVal) ch.maxVal = value;
    }

    ++m_stats.totalSamplesAdded;
    update();
}

/**
 * @brief 设置可见数据点数
 * @param count 可见点数(>=10)
 */
void ScrollChartWidget::setVisiblePoints(int count)
{
    m_visiblePoints = qMax(10, count);
    update();
}

/**
 * @brief 获取当前可见数据点数
 * @return 可见点数
 */
int ScrollChartWidget::visiblePoints() const
{
    return m_visiblePoints;
}

/**
 * @brief 设置是否自动缩放Y轴
 * @param enabled true=自动 false=手动
 */
void ScrollChartWidget::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
    update();
}

/**
 * @brief 设置手动Y轴范围
 * @param min 最小值
 * @param max 最大值
 */
void ScrollChartWidget::setYRange(double min, double max)
{
    m_yMin = min;
    m_yMax = max;
    m_autoScale = false;
    update();
}

/**
 * @brief 设置是否显示网格线
 * @param enabled true=显示
 */
void ScrollChartWidget::setGridEnabled(bool enabled)
{
    m_gridEnabled = enabled;
    update();
}

/**
 * @brief 设置折线宽度
 * @param width 线宽(像素)
 */
void ScrollChartWidget::setLineWidth(qreal width)
{
    m_lineWidth = qMax(0.5, width);
    update();
}

/**
 * @brief 获取当前通道数
 * @return 通道数量
 */
int ScrollChartWidget::channelCount() const
{
    return m_channels.size();
}

/**
 * @brief 获取指定通道信息
 * @param index 通道索引
 * @return Channel常量引用
 */
const ScrollChartWidget::Channel& ScrollChartWidget::channelInfo(int index) const
{
    return m_channels.at(index);
}

/**
 * @brief 导出当前图表为PNG图片
 * @param filePath 目标文件路径
 * @param width 图片宽度
 * @param height 图片高度
 * @return true导出成功
 */
bool ScrollChartWidget::exportToPng(const QString& filePath, int width, int height)
{
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    /* 临时调整绘图区域大小以匹配导出尺寸 */
    const QRectF exportRect(0, 0, width, height);
    const QRectF plotArea(m_leftMargin, 8,
                          width - m_leftMargin - 8,
                          height - 8 - m_bottomMargin);

    auto& theme = ThemeManager::instance();

    /* 背景 */
    painter.fillRect(exportRect, theme.color(ThemeManager::SemanticColor::BgPrimary));

    /* 网格 */
    if (m_gridEnabled) {
        drawGrid(painter, plotArea);
    }

    /* 坐标轴标签 */
    drawAxisLabels(painter, plotArea);

    /* 折线 */
    drawChannels(painter, plotArea);

    /* 图例 */
    QFont legendFont = font();
    legendFont.setPointSize(9);
    painter.setFont(legendFont);
    qreal legendX = plotArea.right() - 120;
    qreal legendY = plotArea.top() + 6;
    for (int i = 0; i < m_channels.size(); ++i) {
        const Channel& ch = m_channels[i];
        if (!ch.visible) continue;

        painter.setPen(Qt::NoPen);
        painter.setBrush(ch.color);
        painter.drawRect(QRectF(legendX, legendY + i * 18, 12, 12));

        painter.setPen(theme.color(ThemeManager::SemanticColor::TextPrimary));
        painter.drawText(QRectF(legendX + 16, legendY + i * 18, 100, 12),
                         Qt::AlignLeft | Qt::AlignVCenter, ch.name);
    }

    painter.end();

    ++m_stats.totalExports;
    return pixmap.save(filePath, "PNG");
}

/**
 * @brief 获取统计数据的只读引用
 * @return Stats常量引用
 */
const ScrollChartWidget::Stats& ScrollChartWidget::stats() const
{
    return m_stats;
}

/**
 * @brief 建议最小尺寸
 * @return 200x120像素
 */
QSize ScrollChartWidget::minimumSizeHint() const
{
    return QSize(200, 120);
}

/**
 * @brief 计算绘图区域(扣除Y轴/X轴标签边距)
 * @return 绘图区域矩形
 */
QRectF ScrollChartWidget::computePlotArea() const
{
    return QRectF(m_leftMargin, 8,
                  width() - m_leftMargin - 8,
                  height() - 8 - m_bottomMargin);
}

/**
 * @brief 绘制事件 — 背景 → 网格 → 坐标轴标签 → 折线 → 图例
 * @param event 绘制事件参数
 */
void ScrollChartWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ++m_stats.totalPaints;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto& theme = ThemeManager::instance();

    /* 背景 */
    painter.fillRect(rect(), theme.color(ThemeManager::SemanticColor::BgPrimary));

    const QRectF plotArea = computePlotArea();
    if (plotArea.width() <= 0 || plotArea.height() <= 0) {
        painter.end();
        return;
    }

    /* 网格 */
    if (m_gridEnabled) {
        drawGrid(painter, plotArea);
    }

    /* 坐标轴标签 */
    drawAxisLabels(painter, plotArea);

    /* 折线 */
    drawChannels(painter, plotArea);

    /* 图例 — 右上角 */
    QFont legendFont = font();
    legendFont.setPointSize(8);
    painter.setFont(legendFont);
    QFontMetrics fm(legendFont);

    qreal legendX = plotArea.right();
    qreal legendY = plotArea.top();

    /* 先计算图例总宽度以右对齐 */
    qreal maxNameWidth = 0;
    int visibleCount = 0;
    for (const auto& ch : m_channels) {
        if (!ch.visible) continue;
        maxNameWidth = qMax(maxNameWidth, static_cast<qreal>(fm.horizontalAdvance(ch.name)));
        ++visibleCount;
    }
    const qreal legendBlockWidth = 16 + maxNameWidth + 4;
    legendX = plotArea.right() - legendBlockWidth - 4;

    for (int i = 0; i < m_channels.size(); ++i) {
        const Channel& ch = m_channels[i];
        if (!ch.visible) continue;

        const qreal rowY = legendY + i * 16;

        /* 色块 */
        painter.setPen(Qt::NoPen);
        painter.setBrush(ch.color);
        painter.drawRect(QRectF(legendX, rowY + 2, 10, 10));

        /* 名称 */
        painter.setPen(theme.color(ThemeManager::SemanticColor::TextSecondary));
        painter.drawText(QRectF(legendX + 14, rowY, maxNameWidth + 4, 14),
                         Qt::AlignLeft | Qt::AlignVCenter, ch.name);
    }

    painter.end();
}

/**
 * @brief 绘制网格线 — 5条水平线 + 6条垂直虚线
 * @param painter 画笔引用
 * @param plotArea 绘图区域
 */
void ScrollChartWidget::drawGrid(QPainter& painter, const QRectF& plotArea)
{
    auto& theme = ThemeManager::instance();
    QPen gridPen(theme.color(ThemeManager::SemanticColor::Border));
    gridPen.setStyle(Qt::DashLine);
    gridPen.setWidthF(0.6);
    painter.setPen(gridPen);

    /* 水平网格线: 5条 */
    const int hLines = 5;
    for (int i = 1; i <= hLines; ++i) {
        qreal fraction = static_cast<qreal>(i) / (hLines + 1);
        qreal y = plotArea.top() + fraction * plotArea.height();
        painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
    }

    /* 垂直网格线: 6条 */
    const int vLines = 6;
    for (int i = 1; i <= vLines; ++i) {
        qreal fraction = static_cast<qreal>(i) / (vLines + 1);
        qreal x = plotArea.left() + fraction * plotArea.width();
        painter.drawLine(QPointF(x, plotArea.top()), QPointF(x, plotArea.bottom()));
    }
}

/**
 * @brief 绘制所有可见通道的折线 — 使用QPainterPath高效绘制
 * @param painter 画笔引用
 * @param plotArea 绘图区域
 */
void ScrollChartWidget::drawChannels(QPainter& painter, const QRectF& plotArea)
{
    if (m_channels.isEmpty()) {
        return;
    }

    /* 计算Y轴范围 */
    double yMin = m_yMin;
    double yMax = m_yMax;

    if (m_autoScale) {
        yMin = std::numeric_limits<double>::max();
        yMax = std::numeric_limits<double>::lowest();
        bool hasData = false;

        for (const auto& ch : m_channels) {
            if (!ch.visible || ch.data.isEmpty()) continue;
            hasData = true;

            /* 仅取最后 m_visiblePoints 个点计算范围 */
            const int start = qMax(0, ch.data.size() - m_visiblePoints);
            for (int i = start; i < ch.data.size(); ++i) {
                const double v = ch.data[i];
                if (v < yMin) yMin = v;
                if (v > yMax) yMax = v;
            }
        }

        if (!hasData) {
            yMin = 0.0;
            yMax = 100.0;
        }

        /* 全部相同值时扩展范围 */
        if (qFuzzyCompare(yMin, yMax)) {
            yMin -= 1.0;
            yMax += 1.0;
        }

        /* 上下各留5%边距 */
        const double range = yMax - yMin;
        const double margin = range * 0.05;
        yMin -= margin;
        yMax += margin;
    }

    const double yRange = yMax - yMin;
    if (qFuzzyIsNull(yRange)) {
        return;
    }

    /* 绘制每条通道折线 */
    for (const auto& ch : m_channels) {
        if (!ch.visible || ch.data.size() < 2) continue;

        /* 取最后 m_visiblePoints 个点 */
        const int start = qMax(0, ch.data.size() - m_visiblePoints);
        const int count = ch.data.size() - start;
        const qreal stepX = plotArea.width() / static_cast<qreal>(m_visiblePoints - 1);

        QPainterPath path;
        bool first = true;

        for (int i = 0; i < count; ++i) {
            const qreal x = plotArea.left() + i * stepX;
            const double normalizedY = (ch.data[start + i] - yMin) / yRange;
            qreal y = plotArea.bottom() - normalizedY * plotArea.height();
            y = qBound(plotArea.top(), y, plotArea.bottom());

            if (first) {
                path.moveTo(x, y);
                first = false;
            } else {
                path.lineTo(x, y);
            }
        }

        QPen linePen(ch.color, m_lineWidth);
        linePen.setCapStyle(Qt::RoundCap);
        linePen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(linePen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
}

/**
 * @brief 绘制Y轴和X轴刻度标签
 * @param painter 画笔引用
 * @param plotArea 绘图区域
 */
void ScrollChartWidget::drawAxisLabels(QPainter& painter, const QRectF& plotArea)
{
    auto& theme = ThemeManager::instance();

    /* 计算当前Y轴范围(与drawChannels逻辑一致) */
    double yMin = m_yMin;
    double yMax = m_yMax;

    if (m_autoScale) {
        yMin = std::numeric_limits<double>::max();
        yMax = std::numeric_limits<double>::lowest();
        bool hasData = false;

        for (const auto& ch : m_channels) {
            if (!ch.visible || ch.data.isEmpty()) continue;
            hasData = true;
            const int start = qMax(0, ch.data.size() - m_visiblePoints);
            for (int i = start; i < ch.data.size(); ++i) {
                const double v = ch.data[i];
                if (v < yMin) yMin = v;
                if (v > yMax) yMax = v;
            }
        }

        if (!hasData) { yMin = 0.0; yMax = 100.0; }
        if (qFuzzyCompare(yMin, yMax)) { yMin -= 1.0; yMax += 1.0; }
        const double range = yMax - yMin;
        const double margin = range * 0.05;
        yMin -= margin;
        yMax += margin;
    }

    QFont labelFont = font();
    labelFont.setPointSize(8);
    painter.setFont(labelFont);
    painter.setPen(theme.color(ThemeManager::SemanticColor::TextMuted));

    /* Y轴标签: 顶部(max) / 中间 / 底部(min) */
    const QString maxText = QString::number(yMax, 'f', 1);
    const QString minText = QString::number(yMin, 'f', 1);
    const QString midText = QString::number((yMin + yMax) / 2.0, 'f', 1);

    painter.drawText(QRectF(0, plotArea.top() - 6, m_leftMargin - 6, 14),
                     Qt::AlignRight | Qt::AlignVCenter, maxText);
    painter.drawText(QRectF(0, plotArea.bottom() - 6, m_leftMargin - 6, 14),
                     Qt::AlignRight | Qt::AlignVCenter, minText);
    painter.drawText(QRectF(0, plotArea.center().y() - 6, m_leftMargin - 6, 14),
                     Qt::AlignRight | Qt::AlignVCenter, midText);

    /* X轴标签: 左(旧) / 右(新) */
    painter.drawText(QRectF(plotArea.left(), plotArea.bottom() + 4,
                            40, m_bottomMargin - 4),
                     Qt::AlignLeft | Qt::AlignTop, tr("old"));
    painter.drawText(QRectF(plotArea.right() - 40, plotArea.bottom() + 4,
                            40, m_bottomMargin - 4),
                     Qt::AlignRight | Qt::AlignTop, tr("new"));
}
