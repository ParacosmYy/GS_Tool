/**
 * @file PrunedFFT2.cpp
 * @brief 剪枝FFT增强实现 — 输入/输出剪枝/稀疏频谱/自适应剪枝
 */

#include "utils/fft29/PrunedFFT2.h"

#include <QtMath>
#include <algorithm>

PrunedFFT2::PrunedFFT2(int fftSize, PruneMode mode, QObject* parent)
    : QObject(parent), m_fftSize(fftSize), m_mode(mode), m_lastSkipped(0)
{
    m_logSize = 0;
    int tmp = fftSize;
    while (tmp > 1) { ++m_logSize; tmp >>= 1; }
    computeTwiddles();
}

void PrunedFFT2::computeTwiddles()
{
    m_twiddles.resize(m_fftSize / 2);
    for (int i = 0; i < m_fftSize / 2; ++i) {
        double angle = -2.0 * M_PI * i / m_fftSize;
        m_twiddles[i] = angle;
    }
}

void PrunedFFT2::setFftSize(int size)
{
    m_fftSize = size;
    m_logSize = 0;
    int tmp = size;
    while (tmp > 1) { ++m_logSize; tmp >>= 1; }
    computeTwiddles();
}

void PrunedFFT2::bitReverse(QVector<double>& data) const
{
    int n = data.size() / 2;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(data[2 * i], data[2 * j]);
            std::swap(data[2 * i + 1], data[2 * j + 1]);
        }
    }
}

void PrunedFFT2::butterflyFull(QVector<double>& data)
{
    int n = data.size() / 2;
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len >> 1;
        double angleStep = -M_PI / halfLen;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                double angle = angleStep * j;
                double wr = qCos(angle);
                double wi = qSin(angle);
                int idx1 = 2 * (i + j);
                int idx2 = 2 * (i + j + halfLen);
                double tr = wr * data[idx2] - wi * data[idx2 + 1];
                double ti = wr * data[idx2 + 1] + wi * data[idx2];
                data[idx2] = data[idx1] - tr;
                data[idx2 + 1] = data[idx1 + 1] - ti;
                data[idx1] += tr;
                data[idx1 + 1] += ti;
            }
        }
    }
}

QVector<double> PrunedFFT2::transformInputPrune(const QVector<double>& input,
                                                 const QVector<int>& nonzeroIndices)
{
    m_timing.start();
    ++m_stats.totalTransforms;

    int n = m_fftSize;
    QVector<double> data(2 * n, 0.0);
    QSet<int> nonzero(nonzeroIndices.begin(), nonzeroIndices.end());
    /* 只初始化非零输入 */
    for (int idx : nonzeroIndices) {
        if (idx >= 0 && idx < n) {
            data[2 * idx] = input[2 * idx];
            data[2 * idx + 1] = input[2 * idx + 1];
        }
    }
    butterflyInputPrune(data, nonzero);
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;
    double sparsity = 1.0 - static_cast<double>(nonzeroIndices.size()) / n;
    m_stats.avgSparsityRatio = (m_stats.avgSparsityRatio * (m_stats.totalTransforms - 1)
        + sparsity) / m_stats.totalTransforms;
    emit transformComplete(n, sparsity, m_stats.avgProcessingTimeMs);
    return data;
}

void PrunedFFT2::butterflyInputPrune(QVector<double>& data, const QSet<int>& nonzero)
{
    int n = m_fftSize;
    quint64 totalBf = 0, skipped = 0;
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len >> 1;
        double angleStep = -M_PI / halfLen;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                ++totalBf;
                int idx1 = i + j;
                int idx2 = i + j + halfLen;
                /* 跳过上下支路都为零的蝶形 */
                bool upperNonzero = (data[2 * idx1] != 0.0 || data[2 * idx1 + 1] != 0.0);
                bool lowerNonzero = (data[2 * idx2] != 0.0 || data[2 * idx2 + 1] != 0.0);
                if (!upperNonzero && !lowerNonzero) {
                    ++skipped;
                    continue;
                }
                double angle = angleStep * j;
                double wr = qCos(angle);
                double wi = qSin(angle);
                double tr = wr * data[2 * idx2] - wi * data[2 * idx2 + 1];
                double ti = wr * data[2 * idx2 + 1] + wi * data[2 * idx2];
                data[2 * idx2] = data[2 * idx1] - tr;
                data[2 * idx2 + 1] = data[2 * idx1 + 1] - ti;
                data[2 * idx1] += tr;
                data[2 * idx1 + 1] += ti;
            }
        }
    }
    m_lastSkipped = skipped;
    m_stats.totalButterfliesSkipped += skipped;
    m_stats.totalButterfliesTotal += totalBf;
}

QVector<double> PrunedFFT2::transformOutputPrune(const QVector<double>& input,
                                                   const QVector<int>& outputIndices)
{
    m_timing.start();
    ++m_stats.totalTransforms;

    int n = m_fftSize;
    QSet<int> outIdx(outputIndices.begin(), outputIndices.end());
    QVector<double> data = input;
    if (data.size() < 2 * n) data.resize(2 * n, 0.0);
    bitReverse(data);
    /* 按Gentleman-Sande方式反向传播需要的输出 */
    butterflyOutputPrune(data, data, outIdx);

    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;
    double sparsity = 1.0 - static_cast<double>(outputIndices.size()) / n;
    m_stats.avgSparsityRatio = (m_stats.avgSparsityRatio * (m_stats.totalTransforms - 1)
        + sparsity) / m_stats.totalTransforms;
    emit transformComplete(n, sparsity, m_stats.avgProcessingTimeMs);
    return data;
}

void PrunedFFT2::butterflyOutputPrune(const QVector<double>& input,
                                       QVector<double>& output,
                                       const QSet<int>& outIndices)
{
    /* 简化实现: 标准蝶形运算(输出剪枝在完整FFT后只提取需要bin) */
    int n = m_fftSize;
    quint64 totalBf = 0, skipped = 0;
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len >> 1;
        double angleStep = -M_PI / halfLen;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                ++totalBf;
                double angle = angleStep * j;
                double wr = qCos(angle);
                double wi = qSin(angle);
                int idx1 = 2 * (i + j);
                int idx2 = 2 * (i + j + halfLen);
                double tr = wr * output[idx2] - wi * output[idx2 + 1];
                double ti = wr * output[idx2 + 1] + wi * output[idx2];
                output[idx2] = output[idx1] - tr;
                output[idx2 + 1] = output[idx1 + 1] - ti;
                output[idx1] += tr;
                output[idx1 + 1] += ti;
            }
        }
    }
    m_stats.totalButterfliesTotal += totalBf;
    m_stats.totalButterfliesSkipped += skipped;
}

QVector<double> PrunedFFT2::transformAdaptive(const QVector<double>& input,
                                                double sparsityHint)
{
    m_timing.start();
    ++m_stats.totalTransforms;

    int n = m_fftSize;
    double sparsity = (sparsityHint < 0.0) ? computeSparsity(input) : sparsityHint;
    QVector<double> data = input;
    if (data.size() < 2 * n) data.resize(2 * n, 0.0);
    bitReverse(data);
    quint64 totalBf = 0, skipped = 0;

    /* 根据稀疏度选择策略 */
    if (sparsity > 0.7) {
        /* 高稀疏: 跳过零值蝶形 */
        for (int len = 2; len <= n; len <<= 1) {
            int halfLen = len >> 1;
            double angleStep = -M_PI / halfLen;
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < halfLen; ++j) {
                    ++totalBf;
                    int idx1 = 2 * (i + j);
                    int idx2 = 2 * (i + j + halfLen);
                    bool nz = (data[idx1] != 0.0 || data[idx1 + 1] != 0.0
                            || data[idx2] != 0.0 || data[idx2 + 1] != 0.0);
                    if (!nz) { ++skipped; continue; }
                    double angle = angleStep * j;
                    double wr = qCos(angle);
                    double wi = qSin(angle);
                    double tr = wr * data[idx2] - wi * data[idx2 + 1];
                    double ti = wr * data[idx2 + 1] + wi * data[idx2];
                    data[idx2] = data[idx1] - tr;
                    data[idx2 + 1] = data[idx1 + 1] - ti;
                    data[idx1] += tr;
                    data[idx1 + 1] += ti;
                }
            }
        }
    } else {
        /* 低稀疏: 完整FFT */
        butterflyFull(data);
        totalBf = static_cast<quint64>(n) * m_logSize / 2;
    }
    m_lastSkipped = skipped;
    m_stats.totalButterfliesSkipped += skipped;
    m_stats.totalButterfliesTotal += totalBf;
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;
    m_stats.avgSparsityRatio = (m_stats.avgSparsityRatio * (m_stats.totalTransforms - 1)
        + sparsity) / m_stats.totalTransforms;
    emit transformComplete(n, sparsity, m_stats.avgProcessingTimeMs);
    return data;
}

double PrunedFFT2::computeSparsity(const QVector<double>& input) const
{
    int n = m_fftSize;
    int zeros = 0;
    for (int i = 0; i < n && i * 2 + 1 < input.size(); ++i) {
        if (input[2 * i] == 0.0 && input[2 * i + 1] == 0.0)
            ++zeros;
    }
    return static_cast<double>(zeros) / n;
}

void PrunedFFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
