/**
 * @file YAxisManager.cpp
 * @brief 多通道独立Y轴管理器实现
 *
 * 实现要点:
 *   1. 每个通道创建独立的 QValueAxis，颜色与通道线条一致
 *   2. 限制左右各最多2个轴（共4个），超过时按单位分组共享
 *   3. 主题切换时更新轴标签和网格线颜色
 *   4. 轴变化时发射 axesChanged() 信号通知 ChartWidget
 */

#include "chart/scale/YAxisManager.h"

#include <QChart>
#include <QValueAxis>
#include <QLineSeries>

// ============================================================================
// 构造 / 析构
// ============================================================================

/**
 * @brief 构造Y轴管理器
 * @param chart 关联的QChart对象，轴将添加到此图表
 * @param parent 父对象
 */
YAxisManager::YAxisManager(QChart* chart, QObject* parent)
    : QObject(parent)
    , m_chart(chart)
{
}

/**
 * @brief 析构时从图表移除并删除所有轴
 */
YAxisManager::~YAxisManager()
{
    clearAll();
}

// ============================================================================
// 轴的创建和删除
// ============================================================================

/**
 * @brief 为通道创建独立Y轴
 *
 * 如果通道已存在轴则跳过。轴颜色设为通道颜色，
 * 标题显示单位文本，网格线默认隐藏（避免多轴时网格重叠）。
 *
 * @param channel 通道名称
 * @param color 通道颜色
 * @param side 左/右侧放置
 * @param unit 单位文本（作为轴标题）
 */
void YAxisManager::createAxis(const QString& channel, const QColor& color,
                               YAxisSide side, const QString& unit)
{
    // 已存在则跳过（防止重复创建）
    if (m_axes.contains(channel)) {
        return;
    }

    auto* axis = new QValueAxis;
    axis->setObjectName(QString("yAxis_%1").arg(channel));

    // 轴标题显示单位文本，颜色与通道一致
    if (!unit.isEmpty()) {
        axis->setTitleText(unit);
        axis->setTitleBrush(color);
    }

    // 标签颜色使用通道颜色，便于识别哪个轴属于哪个通道
    axis->setLabelsBrush(QBrush(color));

    // 网格线: 仅左侧第一个轴和右侧第一个轴显示网格线
    // 其他轴隐藏网格线以避免视觉混乱
    bool showGrid = (countAxesOnSide(side) == 0);
    axis->setGridLineVisible(showGrid);

    // 轴线条颜色使用通道颜色（半透明以柔和）
    QColor axisColor = color;
    axisColor.setAlpha(180);
    axis->setLinePen(QPen(axisColor, 1));

    // 添加到图表
    Qt::Alignment alignment = (side == YAxisSide::Left)
        ? Qt::AlignLeft : Qt::AlignRight;
    m_chart->addAxis(axis, alignment);

    // 记录轴信息
    m_axes[channel] = {axis, side, color, unit};

    ++m_totalAutoScaleEvents;
    emit axesChanged();
}

/**
 * @brief 移除通道对应的Y轴
 * @param channel 通道名称
 */
void YAxisManager::removeAxis(const QString& channel)
{
    auto it = m_axes.find(channel);
    if (it == m_axes.end()) {
        return;
    }

    // 从图表移除轴
    m_chart->removeAxis(it->axis);
    it->axis->deleteLater();
    m_axes.erase(it);

    emit axesChanged();
}

/**
 * @brief 清除所有Y轴（通道变化时全量重建）
 */
void YAxisManager::clearAll()
{
    for (auto it = m_axes.begin(); it != m_axes.end(); ++it) {
        m_chart->removeAxis(it->axis);
        it->axis->deleteLater();
    }
    m_axes.clear();
}

// ============================================================================
// 轴范围和关联
// ============================================================================

/**
 * @brief 更新通道Y轴的范围
 * @param channel 通道名称
 * @param min 最小值
 * @param max 最大值
 */
void YAxisManager::updateRange(const QString& channel, double min, double max)
{
    auto it = m_axes.find(channel);
    if (it != m_axes.end()) {
        it->axis->setRange(min, max);
        ++m_totalRescales;
    }
}

/**
 * @brief 获取通道对应的QValueAxis
 * @param channel 通道名称
 * @return 轴指针，不存在时返回 nullptr
 */
QValueAxis* YAxisManager::axisForChannel(const QString& channel) const
{
    auto it = m_axes.constFind(channel);
    if (it != m_axes.constEnd()) {
        return it->axis;
    }
    return nullptr;
}

/**
 * @brief 将 series 附加到通道对应的Y轴
 *
 * QLineSeries 需要附加到正确的Y轴才能正确显示刻度。
 * 同时附加到X轴由调用方负责。
 *
 * @param channel 通道名称
 * @param series 要附加的 QLineSeries
 */
void YAxisManager::attachSeries(const QString& channel, QLineSeries* series)
{
    auto* axis = axisForChannel(channel);
    if (axis && series) {
        series->attachAxis(axis);
    }
}

// ============================================================================
// 自动分配策略
// ============================================================================

/**
 * @brief 根据通道列表自动分配Y轴左右侧
 *
 * 策略:
 *   - <= 4通道: 交替分配（偶数索引→左，奇数索引→右）
 *   - > 4通道: 按单位分组，左2右2
 *
 * @param channelNames 通道名称列表
 * @param units 对应的单位列表（与 channelNames 等长）
 * @return 每个通道对应的 YAxisSide
 */
QVector<YAxisSide> YAxisManager::autoAssignSides(
    const QStringList& channelNames, const QStringList& units)
{
    int count = channelNames.size();
    QVector<YAxisSide> sides;
    sides.reserve(count);

    if (count <= 4) {
        // 简单策略: 交替分配左右
        for (int i = 0; i < count; ++i) {
            sides.append((i % 2 == 0) ? YAxisSide::Left : YAxisSide::Right);
        }
    } else {
        // 超过4通道: 按单位分组
        // 收集唯一单位并按出现顺序排列
        QStringList uniqueUnits;
        for (const QString& u : units) {
            if (!uniqueUnits.contains(u)) {
                uniqueUnits.append(u);
            }
        }

        // 为每个唯一单位分配侧（交替左右）
        QMap<QString, YAxisSide> unitSideMap;
        int leftCount = 0;
        int rightCount = 0;
        for (int i = 0; i < uniqueUnits.size(); ++i) {
            // 限制: 左右各最多2个轴
            if (leftCount < 2 && (rightCount >= 2 || i % 2 == 0)) {
                unitSideMap[uniqueUnits[i]] = YAxisSide::Left;
                ++leftCount;
            } else {
                unitSideMap[uniqueUnits[i]] = YAxisSide::Right;
                ++rightCount;
            }
        }

        // 每个通道使用其单位对应的侧
        for (int i = 0; i < count; ++i) {
            sides.append(unitSideMap.value(units[i], YAxisSide::Left));
        }
    }

    return sides;
}

// ============================================================================
// 主题颜色
// ============================================================================

/**
 * @brief 应用当前主题颜色到所有Y轴
 *
 * 更新轴标签颜色（保持通道颜色以标识性）和网格线颜色。
 * 轴标题颜色恢复为通道颜色。
 *
 * @param gridColor 网格线颜色（来自 ThemeManager::Border）
 * @param labelColor 标签文字颜色（来自 ThemeManager::TextSecondary）
 */
void YAxisManager::applyThemeColors(const QColor& gridColor, const QColor& labelColor)
{
    for (auto it = m_axes.begin(); it != m_axes.end(); ++it) {
        QValueAxis* axis = it->axis;

        // 轴标签颜色: 保持通道颜色以区分不同轴
        axis->setLabelsBrush(QBrush(it->color));

        // 轴标题颜色: 使用通道颜色
        axis->setTitleBrush(it->color);

        // 网格线颜色: 仅对显示网格的轴更新
        if (axis->isGridLineVisible()) {
            axis->setGridLineColor(gridColor);
        }

        // 轴线条颜色: 通道颜色（半透明）
        QColor axisColor = it->color;
        axisColor.setAlpha(180);
        axis->setLinePen(QPen(axisColor, 1));
    }
}

// ============================================================================
// 内部辅助
// ============================================================================

/**
 * @brief 获取指定侧已使用的轴数量
 * @param side 左/右侧
 * @return 该侧已有的轴数量
 */
int YAxisManager::countAxesOnSide(YAxisSide side) const
{
    int count = 0;
    for (auto it = m_axes.constBegin(); it != m_axes.constEnd(); ++it) {
        if (it->side == side) {
            ++count;
        }
    }
    return count;
}

/** @brief 获取累计缩放重算次数 */
quint64 YAxisManager::totalRescales() const
{
    return m_totalRescales;
}

/** @brief 获取累计自动缩放事件次数 */
quint64 YAxisManager::totalAutoScaleEvents() const
{
    return m_totalAutoScaleEvents;
}

/** @brief 重置所有Y轴统计计数器 */
void YAxisManager::resetYAxisStatistics()
{
    m_totalRescales = 0;
    m_totalAutoScaleEvents = 0;
}
