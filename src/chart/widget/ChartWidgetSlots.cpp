/**
 * @file ChartWidgetSlots.cpp
 * @brief 实时波形图控件 — 槽函数与公开接口方法
 *
 * 从 ChartWidget.cpp 拆分而来，包含帧数据接收槽函数和控制栏按钮回调:
 *   - onFrameParsed(): 帧解析回调，转发到ChartModel
 *   - onPauseToggled(): 暂停/继续按钮切换
 *   - onClearClicked(): 清除按钮回调
 */

#include "chart/widget/ChartWidget.h"
#include "protocol/parser/FrameDefinition.h"

// ============================================================================
// 公开接口
// ============================================================================

/** @brief 返回波形数据模型指针 @return ChartModel指针 */
ChartModel* ChartWidget::model() const
{
    return m_model;
}

/** @brief 从帧定义配置波形图，自动创建通道映射 @param def 帧定义 */
void ChartWidget::configureFromFrameDefinition(const FrameDefinition& def)
{
    // 从帧定义的字段列表自动生成通道配置
    m_configSet = ChannelConfigSet::generateDefaults(def.fields);

    // 应用到ChartModel（会触发 channelsChanged 信号 -> 重建渲染层）
    m_model->setChannelConfigSet(m_configSet);
}

/** @brief 设置滑动窗口大小 @param points 窗口点数 */
void ChartWidget::setWindowSize(int points)
{
    m_model->setWindowSize(points);
}

/** @brief 清除波形数据 */
void ChartWidget::clear()
{
    m_model->clear();
}

/** @brief 返回当前通道名称列表 @return 通道名列表 */
QStringList ChartWidget::channels() const
{
    return m_model->channelNames();
}

/** @brief 设置Y轴固定范围，禁用自动Y轴 @param min 最小值 @param max 最大值 */
void ChartWidget::setYRange(double min, double max)
{
    m_autoYRange = false;
    // 设置所有已有Y轴的范围
    for (const QString& ch : m_seriesMap.keys()) {
        m_yAxisManager->updateRange(ch, min, max);
    }
}

/** @brief 设置是否启用Y轴自动范围 @param enabled true=自动 */
void ChartWidget::setAutoYRange(bool enabled)
{
    m_autoYRange = enabled;
}

// ============================================================================
// 槽函数 -- 帧数据接收（兼容旧接口，委托给ChartModel）
// ============================================================================

/** @brief 帧解析回调，转发到ChartModel @param fields 字段映射 @param rawFrame 原始帧 */
void ChartWidget::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    if (m_paused) return;
    ++m_totalDataUpdates;
    ++m_totalSamplesAppended;
    m_model->onFrameParsed(fields, rawFrame);
}

// ============================================================================
// 槽函数 -- 控制栏按钮
// ============================================================================

/** @brief 暂停/继续按钮切换回调 @param paused true=暂停 */
void ChartWidget::onPauseToggled(bool paused)
{
    ++m_totalInteractions;
    m_paused = paused;
    m_pauseBtn->setText(paused ? tr("继续") : tr("暂停"));
}

/** @brief 清除按钮回调，清空波形数据和series */
void ChartWidget::onClearClicked()
{
    ++m_totalInteractions;
    clear();
}
