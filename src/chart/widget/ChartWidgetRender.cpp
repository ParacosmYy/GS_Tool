/**
 * @file ChartWidgetRender.cpp
 * @brief ChartWidget 数据渲染与通道同步实现
 *
 * 从 ChartWidget.cpp 拆分而来，包含:
 *   - updateChart()         帧数据到达时的series数据刷新与坐标轴范围更新
 *   - onChannelsChanged()   通道配置变化时的series/Y轴全量重建
 *   - onDataCleared()       数据清空时的series点清除与轴范围重置
 */

#include "chart/widget/ChartWidget.h"
#include "chart/widget/ChartColors.h"
#include "core/theme/ThemeManager.h"

#include <QLineSeries>

// ============================================================================
// 槽函数 -- ChartModel 信号驱动的渲染更新
// ============================================================================

/** @brief 图表数据更新回调，刷新可见通道的series数据和坐标轴范围 @param updatedChannels 更新的通道名称列表 */
void ChartWidget::updateChart(const QStringList& updatedChannels)
{
    if (m_paused) return;

    ++m_totalRenders;

    // FPS追踪: 基于两次渲染间隔计算瞬时FPS
    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (m_lastRenderTimeMs > 0) {
        qint64 deltaMs = nowMs - m_lastRenderTimeMs;
        if (deltaMs > 0) {
            m_renderFps = 1000.0 / static_cast<double>(deltaMs);
        }
    }
    m_lastRenderTimeMs = nowMs;

    // 刷新每个更新通道的series数据
    for (const QString& name : updatedChannels) {
        auto it = m_seriesMap.find(name);
        if (it == m_seriesMap.end()) continue;

        QLineSeries* series = it.value();
        QVector<QPointF> data = m_model->channelData(name);
        series->replace(data);
    }

    // 更新X轴范围
    QPair<double, double> xRange = m_model->xRange();
    m_xAxis->setRange(xRange.first, xRange.second);

    // 自动Y轴范围: 按通道独立更新
    if (m_autoYRange) {
        for (const QString& name : updatedChannels) {
            if (!m_seriesMap.contains(name)) continue;
            QPair<double, double> yRange = m_model->channelYRange(name);
            if (yRange.first != 0.0 || yRange.second != 0.0) {
                double margin = (yRange.second - yRange.first) * 0.1;
                if (margin < 0.001) margin = 1.0;
                m_yAxisManager->updateRange(name,
                    yRange.first - margin, yRange.second + margin);
                ++m_totalAutoScales;
            }
        }
    }

    // 更新状态标签
    m_statusLabel->setText(tr("通道: %1 | 帧: %2")
        .arg(m_seriesMap.size())
        .arg(m_model->currentFrameIndex()));
}

/** @brief 通道配置变化回调，同步series新增/删除通道对应的QLineSeries */
void ChartWidget::onChannelsChanged()
{
    // 清除旧的series
    for (auto* series : m_seriesMap) {
        m_chart->removeSeries(series);
        series->deleteLater();
    }
    m_seriesMap.clear();

    // 清除所有旧Y轴（全量重建）
    m_yAxisManager->clearAll();

    // 获取当前主题对应的调色板
    bool isDark = ThemeManager::instance().isSystemDarkMode()
        || ThemeManager::instance().currentTheme().contains("dark");
    QVector<QColor> palette = ChartColors::colorsForTheme(isDark);
    if (palette.isEmpty()) {
        palette = {Qt::cyan};  // 降级回退色，防止除零崩溃
    }

    // ---- 降级路径: m_configSet 尚未配置时，直接从 model 获取通道名 ----
    // 场景: 协议桥数据先于帧编辑器配置到达，此时 channelsChanged 信号已触发
    //       但 configureFromFrameDefinition() 还未被调用，m_configSet 为空
    if (m_configSet.channels().isEmpty() && m_model) {
        const QStringList names = m_model->channelNames();
        // 自动分配左右侧
        QStringList emptyUnits;
        for (int i = 0; i < names.size(); ++i) emptyUnits.append(QString());
        QVector<YAxisSide> sides = YAxisManager::autoAssignSides(names, emptyUnits);

        for (int i = 0; i < names.size(); ++i) {
            QColor chColor = palette[i % palette.size()];
            m_yAxisManager->createAxis(names[i], chColor, sides[i]);
            createSeries(names[i], chColor);
        }
        m_statusLabel->setText(tr("通道: %1").arg(m_seriesMap.size()));
        return;
    }

    // ---- 主路径: 根据完整的通道配置创建series和Y轴 ----
    const QVector<ChannelConfig>& channels = m_configSet.channels();
    QStringList names, units;
    for (const ChannelConfig& cfg : channels) {
        if (cfg.enabled) {
            names.append(cfg.displayName);
            units.append(cfg.unit);
        }
    }

    // 自动分配左右侧
    QVector<YAxisSide> sides = YAxisManager::autoAssignSides(names, units);

    int idx = 0;
    for (const ChannelConfig& cfg : channels) {
        if (!cfg.enabled) continue;
        // 如果通道配置有自定义颜色则使用，否则从主题调色板获取
        QColor chColor = cfg.color.isValid() ? cfg.color :
            palette[idx % palette.size()];

        // 创建独立Y轴
        m_yAxisManager->createAxis(cfg.displayName, chColor,
            sides[idx], cfg.unit);

        createSeries(cfg.displayName, chColor);
        ++idx;
    }

    m_statusLabel->setText(tr("通道: %1").arg(m_seriesMap.size()));
}

/** @brief 数据清空回调，清除所有series数据点 */
void ChartWidget::onDataCleared()
{
    // 清除所有series的数据点
    for (auto* series : m_seriesMap) {
        series->clear();
    }
    m_xAxis->setRange(0, 10);
    // 重置所有Y轴到默认范围
    for (const QString& ch : m_seriesMap.keys()) {
        m_yAxisManager->updateRange(ch, 0, 100);
    }
}
