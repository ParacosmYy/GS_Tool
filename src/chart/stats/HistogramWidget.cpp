/**
 * @file HistogramWidget.cpp
 * @brief 统计直方图控件实现 -- 通道数据值分布分析面板
 *
 * 实现HistogramWidget的构造函数、信号连接、直方图刷新和事件回调。
 * 从ChartModel读取通道数据，计算值分布直方图，
 * 使用QBarSeries绘制直方图柱状图，下方显示统计摘要。
 *
 * UI布局、工具栏、图表创建及主题颜色见 HistogramWidgetUI.cpp
 * 统计计算、导出和计数器接口见 HistogramWidgetStats.cpp
 */

#include "chart/stats/HistogramWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <QtCharts>

/** @brief 构造直方图控件 @param model 数据模型指针 @param parent 父控件 */
HistogramWidget::HistogramWidget(ChartModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_series(nullptr)
    , m_barSet(nullptr)
    , m_xAxis(nullptr)
    , m_yAxis(nullptr)
{
    setObjectName(QStringLiteral("HistogramWidget"));
    setupUI();

    if (m_model) {
        connect(m_model, &ChartModel::dataUpdated,
                this, &HistogramWidget::onDataUpdated);
        connect(m_model, &ChartModel::channelsChanged,
                this, &HistogramWidget::onChannelsChanged);
    }
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &HistogramWidget::onThemeChanged);

    applyThemeColors();
    onChannelsChanged();
}

// ---- UI布局/工具栏/图表创建/主题颜色已拆分至 HistogramWidgetUI.cpp ----
// ---- 统计计算/导出/计数器已拆分至 HistogramWidgetStats.cpp ----

/** @brief 刷新直方图显示 — 从模型读取数据、计算分桶、更新柱状图和统计标签 */
void HistogramWidget::refreshHistogram()
{
    if (!m_model || !m_barSet) {
        return;
    }

    QString channel = m_channelCombo->currentText();
    if (channel.isEmpty()) {
        m_statsLabel->setText(tr("无通道数据"));
        return;
    }

    QVector<QPointF> data = m_model->channelData(channel);
    if (data.isEmpty()) {
        m_barSet->remove(0, m_barSet->count());
        m_statsLabel->setText(tr("无数据"));
        return;
    }

    int bins = m_binsSpin->value();
    auto histData = computeHistogram(data, bins);
    ++m_totalBinRecalculations;

    // 更新柱体数据
    m_barSet->remove(0, m_barSet->count());
    QStringList categories;
    for (const auto& [center, count] : histData) {
        *m_barSet << count;
        categories.append(QStringLiteral("%1").arg(center, 0, 'f', 2));
    }
    m_xAxis->setCategories(categories);

    // 自动调整Y轴
    int maxCount = 0;
    int peakIdx = 0;
    for (int i = 0; i < histData.size(); ++i) {
        const auto& [center, count] = histData[i];
        if (count > maxCount) {
            maxCount = count;
            peakIdx = i;
        }
    }
    m_yAxis->setRange(0, qMax(maxCount + 1, 1));
    ++m_totalAutoRanges;
    m_peakBinIndex = peakIdx;
    m_maxBinCount = maxCount;
    ++m_totalBinsComputed;

    // 更新统计摘要
    Stats s = computeStats(data);
    m_statsLabel->setText(
        tr("均值: %1 | 标准差: %2 | 最小: %3 | 最大: %4 | 样本: %5")
            .arg(s.mean, 0, 'f', 3)
            .arg(s.stddev, 0, 'f', 3)
            .arg(s.min, 0, 'f', 3)
            .arg(s.max, 0, 'f', 3)
            .arg(s.count));
    ++m_totalUpdates;
    ++m_totalDistributionUpdates;
}

/** @brief 通道选择变更回调 @param index 下拉框当前索引 */
void HistogramWidget::onChannelChanged(int /*index*/)
{
    ++m_totalChannelSwitches;
    if (m_autoRefresh) { refreshHistogram(); }
}

/** @brief 分桶数变更回调 @param value 新的分桶数值 */
void HistogramWidget::onBinsChanged(int /*value*/)
{
    ++m_totalBinChanges;
    if (m_autoRefresh) { refreshHistogram(); }
}

/** @brief 自动刷新开关切换回调 @param checked true=启用自动刷新 */
void HistogramWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
    ++m_totalAutoRefreshToggles;
}

/** @brief ChartModel数据更新回调 — 仅在自动刷新且当前通道有更新时重绘 @param updatedChannels 本次更新的通道名称列表 */
void HistogramWidget::onDataUpdated(const QStringList& updatedChannels)
{
    if (!m_autoRefresh) { return; }
    if (updatedChannels.contains(m_channelCombo->currentText())) {
        refreshHistogram();
    }
}

/** @brief 通道列表变更回调 — 重建下拉框并尽量恢复之前的选择 */
void HistogramWidget::onChannelsChanged()
{
    if (!m_model) { return; }

    QString prevChannel = m_channelCombo->currentText();
    m_channelCombo->blockSignals(true);
    m_channelCombo->clear();
    for (const auto& name : m_model->channelNames()) {
        m_channelCombo->addItem(name);
    }

    int idx = m_channelCombo->findText(prevChannel);
    if (idx >= 0) {
        m_channelCombo->setCurrentIndex(idx);
    } else if (m_channelCombo->count() > 0) {
        m_channelCombo->setCurrentIndex(0);
    }
    m_channelCombo->blockSignals(false);

    if (m_autoRefresh && m_channelCombo->count() > 0) {
        refreshHistogram();
    }
}

/** @brief 主题切换回调 — 重新应用主题颜色 */
void HistogramWidget::onThemeChanged() { ++m_totalThemeChanges; applyThemeColors(); }
