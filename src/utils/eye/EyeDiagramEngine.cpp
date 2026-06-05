/**
 * @file EyeDiagramEngine.cpp
 * @brief 眼图引擎实现 — 信号分段叠加+眼图参数测量
 */

#include "utils/eye/EyeDiagramEngine.h"

#include <QtMath>
#include <algorithm>

EyeDiagramEngine::EyeDiagramEngine(QObject* parent)
    : QObject(parent) {}

void EyeDiagramEngine::setConfig(const Config& config) { m_config = config; }

/** @brief 生成眼图 @param data 信号 @return 叠加数据 */
QList<QVector<double>> EyeDiagramEngine::generate(const QVector<double>& data)
{
    QList<QVector<double>> result;
    if (data.size() < 10) return result;

    double samplesPerUI = m_config.sampleRate / m_config.symbolRate;
    int uiSamples = qMax(2, static_cast<int>(samplesPerUI));
    int totalSamples = uiSamples * m_config.tracesPerEye;

    int traceCount = data.size() / uiSamples;
    result.reserve(totalSamples);

    for (int col = 0; col < totalSamples; ++col) {
        result.append(QVector<double>());
    }

    for (int t = 0; t < traceCount; ++t) {
        int offset = t * uiSamples;
        for (int col = 0; col < totalSamples; ++col) {
            int idx = offset + col % uiSamples;
            if (idx < data.size()) {
                result[col].append(data[idx]);
            }
        }
    }

    m_stats.totalTracesOverlaid += static_cast<quint64>(traceCount);
    ++m_stats.totalEyesAnalyzed;

    emit eyeGenerated(traceCount);
    return result;
}

/** @brief 测量眼图 @param eyeData 眼图数据 @return 测量 */
EyeDiagramEngine::Measurement EyeDiagramEngine::measure(
    const QList<QVector<double>>& eyeData)
{
    Measurement m;
    if (eyeData.isEmpty()) return m;

    /* 找最大/最小值计算眼高 */
    double globalMax = -1e300, globalMin = 1e300;
    for (const auto& col : eyeData) {
        for (double v : col) {
            if (v > globalMax) globalMax = v;
            if (v < globalMin) globalMin = v;
        }
    }

    double range = globalMax - globalMin;
    m.eyeHeight = range;

    /* 交叉点在50%处附近 */
    m.crossingLevel = (globalMax + globalMin) / 2.0;

    /* 简化眼宽估计: 在中间列找到信号的持续时间 */
    int midCol = eyeData.size() / 2;
    if (midCol < eyeData.size() && !eyeData[midCol].isEmpty()) {
        double threshold = m.crossingLevel;
        int crossings = 0;
        const auto& col = eyeData[midCol];
        for (int i = 1; i < col.size(); ++i) {
            if ((col[i] >= threshold) != (col[i - 1] >= threshold)) {
                ++crossings;
            }
        }
        m.eyeWidth = (crossings > 1) ? 1.0 : 0.5;
    }

    /* 抖动估计 */
    double jitterSum = 0.0, jitterSqSum = 0.0;
    int jitterCount = 0;
    for (const auto& col : eyeData) {
        for (double v : col) {
            double dev = qAbs(v - m.crossingLevel);
            jitterSum += dev;
            jitterSqSum += dev * dev;
            ++jitterCount;
        }
    }
    if (jitterCount > 0) {
        double meanJitter = jitterSum / jitterCount;
        m.jitterRms = qSqrt(jitterSqSum / jitterCount - meanJitter * meanJitter);
        m.jitterPp = m.jitterRms * 6.0; /* 6σ = ~99.7% */
    }

    m.eyeOpening = (range > 0) ? m.eyeHeight / range * 100.0 : 0.0;
    if (m.eyeHeight > m_stats.peakEyeHeight) {
        m_stats.peakEyeHeight = m.eyeHeight;
    }

    return m;
}

void EyeDiagramEngine::resetStatistics() { m_stats = Stats{}; }
