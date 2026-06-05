/**
 * @file NotchFilterV2.cpp
 * @brief IIR陷波滤波器实现 — 2阶双二阶节(Direct Form I)
 */

#include "utils/filter3/NotchFilter.h"

#include <QElapsedTimer>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

NotchFilterV2::NotchFilterV2(QObject* parent)
    : QObject(parent)
{
}

NotchFilterV2::~NotchFilterV2() = default;

// ═══════════════════════════════════════════════════════════
// 滤波器设计
// ═══════════════════════════════════════════════════════════

void NotchFilterV2::design(double freqHz, double Q, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    /* 参数校验 */
    if (freqHz <= 0.0 || Q <= 0.0 || sampleRate <= 0.0) {
        return;
    }

    /* 频率不能超过奈奎斯特 */
    if (freqHz >= sampleRate / 2.0) {
        return;
    }

    m_params.centerFreqHz = freqHz;
    m_params.qualityFactor = Q;
    m_params.sampleRateHz = sampleRate;

    /* 计算角频率 */
    const double w0 = 2.0 * M_PI * freqHz / sampleRate;
    const double cosW0 = std::cos(w0);
    const double sinW0 = std::sin(w0);
    const double alpha = sinW0 / (2.0 * Q);

    /* 双二阶节系数(陷波: 在中心频率处增益为0) */
    const double b0 = 1.0;
    const double b1 = -2.0 * cosW0;
    const double b2 = 1.0;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosW0;
    const double a2 = 1.0 - alpha;

    /* 归一化(除以a0) */
    m_b0 = b0 / a0;
    m_b1 = b1 / a0;
    m_b2 = b2 / a0;
    m_a1 = a1 / a0;
    m_a2 = a2 / a0;

    m_designed = true;
    resetState();

    m_stats.totalDesigns++;
    emit designCompleted(freqHz);
}

// ═══════════════════════════════════════════════════════════
// 滤波应用
// ═══════════════════════════════════════════════════════════

QVector<double> NotchFilterV2::apply(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_designed || signal.isEmpty()) {
        return {};
    }

    /* 重置延迟线 */
    resetState();

    QVector<double> output(signal.size());
    for (int i = 0; i < signal.size(); ++i) {
        output[i] = processSample(signal[i]);
    }

    m_stats.totalApplications++;
    const qint64 elapsed = timer.elapsed();
    const auto total = m_stats.totalDesigns + m_stats.totalApplications;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (total - 1) / total +
        static_cast<double>(elapsed) / total;

    emit applyCompleted(output.size());
    return output;
}

double NotchFilterV2::processSample(double sample)
{
    /* Direct Form I: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
     *                        - a1*y[n-1] - a2*y[n-2]         */
    const double y = m_b0 * sample + m_b1 * m_x1 + m_b2 * m_x2
                     - m_a1 * m_y1 - m_a2 * m_y2;

    /* 更新延迟线 */
    m_x2 = m_x1;
    m_x1 = sample;
    m_y2 = m_y1;
    m_y1 = y;

    return y;
}

void NotchFilterV2::resetState()
{
    m_x1 = m_x2 = m_y1 = m_y2 = 0.0;
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

NotchFilterV2::Parameters NotchFilterV2::parameters() const
{
    return m_params;
}

bool NotchFilterV2::isDesigned() const
{
    return m_designed;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

NotchFilterV2::Stats NotchFilterV2::stats() const
{
    return m_stats;
}

void NotchFilterV2::resetStatistics()
{
    m_stats = Stats{};
}
