/**
 * @file WaveformGeneratorWidgetStats.cpp
 * @brief WaveformGeneratorWidget 统计接口实现
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/wavegen/WaveformGeneratorWidget.h"
#include "utils/wavegen/WaveformGenerator.h"

WaveGen::Stats WaveformGeneratorWidget::stats() const
{
    return m_generator->stats();
}

void WaveformGeneratorWidget::resetStatistics()
{
    m_generator->resetStatistics();
}
