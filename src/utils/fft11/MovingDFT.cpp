/**
 * @file MovingDFT.cpp
 * @brief 滑动DFT引擎实现 — O(1)逐样本实时频谱更新
 */

#include "utils/fft11/MovingDFT.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
MovingDFT::MovingDFT(QObject* parent)
    : QObject(parent)
    , m_sampleRate(1000.0)
    , m_binCount(0)
    , m_sampleIndex(0)
    , m_prevSample(0.0)
    , m_resetInterval(4096)
    , m_samplesSinceReset(0)
    , m_timeSum(0.0)
{
}

/** @brief 初始化滑动DFT @param sampleRate 采样率 @param binCount bin数量 */
void MovingDFT::initialize(double sampleRate, int binCount)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_binCount = qMax(1, binCount);
    m_sampleIndex = 0;
    m_prevSample = 0.0;
    m_samplesSinceReset = 0;

    /* 初始化所有频率bin状态为零 */
    m_bins.resize(m_binCount);
    for (auto& b : m_bins) {
        b = std::complex<double>(0.0, 0.0);
    }
}

/** @brief 输入单个采样值，O(1)更新所有bin @param sample 新采样值 */
void MovingDFT::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    if (m_binCount <= 0) {
        return;
    }

    /* 滑动DFT核心公式: S_k = (S_k - x_old + x_new) * e^(j*2*pi*k/N)
     * 其中 x_old 是N个样本前的值。这里用递推形式:
     * S_k = (S_k + x_new - x_prev) * twiddle(k) */
    double delta = sample - m_prevSample;

    for (int k = 0; k < m_binCount; ++k) {
        std::complex<double> coeff = twiddleFactor(k);
        m_bins[k] = (m_bins[k] + delta) * coeff;
    }

    m_prevSample = sample;
    m_sampleIndex++;
    m_samplesSinceReset++;

    /* 周期性重置防止数值漂移 */
    if (m_samplesSinceReset >= m_resetInterval) {
        resetBins();
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_stats.totalSamplesProcessed++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSamplesProcessed);

    emit spectrumUpdated(m_sampleIndex, m_binCount);
}

/** @brief 批量输入采样值 @param samples 采样数组 */
void MovingDFT::pushSamples(const QVector<double>& samples)
{
    for (const auto& s : samples) {
        pushSample(s);
    }
}

/** @brief 获取当前所有频率bin的结果 @return bin结果列表 */
QList<MovingDFT::BinResult> MovingDFT::currentSpectrum() const
{
    QList<BinResult> results;
    results.reserve(m_binCount);

    for (int k = 0; k < m_binCount; ++k) {
        results.append(binAt(k));
    }
    return results;
}

/** @brief 获取指定bin结果 @param index bin索引 @return bin结果 */
MovingDFT::BinResult MovingDFT::binAt(int index) const
{
    BinResult result;
    if (index < 0 || index >= m_binCount) {
        return result;
    }

    result.frequency = static_cast<double>(index) * m_sampleRate
                       / static_cast<double>(m_binCount);
    result.magnitude = std::abs(m_bins[index]);
    result.phase = std::arg(m_bins[index]);
    return result;
}

/** @brief 获取频率bin数量 @return bin数量 */
int MovingDFT::binCount() const
{
    return m_binCount;
}

/** @brief 重置所有bin状态(消除数值漂移) */
void MovingDFT::resetBins()
{
    for (auto& b : m_bins) {
        b = std::complex<double>(0.0, 0.0);
    }
    m_samplesSinceReset = 0;
    m_stats.totalResets++;
}

/** @brief 重置统计信息 */
void MovingDFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算旋转因子 @param k bin索引 @return 旋转因子 */
std::complex<double> MovingDFT::twiddleFactor(int k) const
{
    /* e^(-j * 2*pi*k/N) — 每推入一个样本旋转一次 */
    double angle = -2.0 * M_PI * static_cast<double>(k)
                   / static_cast<double>(m_binCount);
    return std::complex<double>(qCos(angle), qSin(angle));
}
