/**
 * @file FftWidgetSlots.cpp
 * @brief FFT频谱控件 - 槽函数与统计接口实现
 *
 * 从 FftWidget.cpp 拆分而来，包含通道/窗函数/FFT大小变更槽、
 * 自动刷新、数据更新、通道重建、主题切换和统计计数器方法。
 */

#include "chart/fft/FftWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

/** @brief 通道选择变更槽函数，自动刷新模式下触发频谱重算 @param index 下拉框新索引(未使用) */
void FftWidget::onChannelChanged(int /*index*/)
{
    ++m_totalChannelSwitches;
    if (m_autoRefresh) {
        ++m_totalAutoRefreshes;
        refreshSpectrum();
    }
}

/** @brief 窗函数选择变更槽函数，自动刷新模式下触发频谱重算 @param index 下拉框新索引(未使用) */
void FftWidget::onWindowChanged(int /*index*/)
{
    ++m_totalWindowChanges;
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

/** @brief FFT大小变更槽函数，自动刷新模式下触发频谱重算 @param value 新的FFT大小(未使用) */
void FftWidget::onFftSizeChanged(int /*value*/)
{
    ++m_totalSizeChanges;
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

/** @brief 自动刷新开关切换槽函数 @param checked true=开启自动刷新 */
void FftWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
}

/** @brief ChartModel数据更新槽函数，仅当当前通道有新数据时自动刷新 @param updatedChannels 本次更新的通道名列表 */
void FftWidget::onDataUpdated(const QStringList& updatedChannels)
{
    if (!m_autoRefresh) {
        return;
    }

    QString currentChannel = m_channelCombo->currentText();
    if (updatedChannels.contains(currentChannel)) {
        ++m_totalAutoRefreshes;
        refreshSpectrum();
    }
}

/** @brief 通道列表变更槽函数，重建通道下拉框并尝试恢复之前的选择 */
void FftWidget::onChannelsChanged()
{
    if (!m_model) {
        return;
    }

    /* 保存当前选择 */
    QString prevChannel = m_channelCombo->currentText();

    /* 重建通道列表 */
    m_channelCombo->blockSignals(true);
    m_channelCombo->clear();
    QStringList names = m_model->channelNames();
    for (const auto& name : names) {
        m_channelCombo->addItem(name);
    }

    /* 恢复之前的选择（如果仍存在） */
    int idx = m_channelCombo->findText(prevChannel);
    if (idx >= 0) {
        m_channelCombo->setCurrentIndex(idx);
    } else if (m_channelCombo->count() > 0) {
        m_channelCombo->setCurrentIndex(0);
    }
    m_channelCombo->blockSignals(false);

    /* 自动刷新频谱 */
    if (m_autoRefresh && m_channelCombo->count() > 0) {
        refreshSpectrum();
    }
}

/** @brief 主题切换槽函数，重新应用颜色到图表 */
void FftWidget::onThemeChanged()
{
    applyThemeColors();
}

/** @brief 获取累计FFT变换次数 */
quint64 FftWidget::totalTransforms() const
{
    return m_totalTransforms;
}

/** @brief 获取累计峰值搜索次数(频谱最大幅度检测) */
quint64 FftWidget::totalPeakSearches() const
{
    return m_totalPeakSearches;
}

/** @brief 获取累计窗函数变更次数 */
quint64 FftWidget::totalWindowChanges() const
{
    return m_totalWindowChanges;
}

/** @brief 获取累计FFT大小变更次数 */
quint64 FftWidget::totalSizeChanges() const
{
    return m_totalSizeChanges;
}

/** @brief 重置所有FFT控件统计计数器 */
void FftWidget::resetFftWidgetStatistics()
{
    m_totalTransforms = 0;
    m_totalPeakSearches = 0;
    m_totalWindowChanges = 0;
    m_totalSizeChanges = 0;
    m_totalAutoRefreshes = 0;
    m_totalChannelSwitches = 0;
    m_totalRenderErrors = 0;
}
