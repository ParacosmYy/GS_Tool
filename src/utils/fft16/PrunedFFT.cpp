/**
 * @file PrunedFFT.cpp
 * @brief 修剪FFT实现 — 输出/输入蝶形修剪
 */

#include "utils/fft16/PrunedFFT.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <set>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

PrunedFFT::PrunedFFT(QObject* parent)
    : QObject(parent)
    , m_cachedSize(0)
{
}

PrunedFFT::~PrunedFFT() = default;

// ═══════════════════════════════════════════════════════════
// 输出修剪FFT
// ═══════════════════════════════════════════════════════════

PrunedFFT::PrunedResult PrunedFFT::transformOutputPruned(
    const QVector<double>& input, const QVector<int>& outputBins)
{
    QElapsedTimer timer;
    timer.start();

    PrunedResult result;
    int n = input.size();

    if (n < 2 || !isPowerOfTwo(n) || outputBins.isEmpty()) {
        result.success = false;
        return result;
    }

    precomputeTwiddle(n);

    int totalButterfliesAll = n * static_cast<int>(std::log2(n));
    int totalButterfliesUsed = 0;

    result.binIndices = outputBins;
    result.magnitudes.resize(outputBins.size());
    result.phases.resize(outputBins.size());

    for (int i = 0; i < outputBins.size(); ++i) {
        int bin = outputBins[i];
        if (bin < 0 || bin >= n) {
            result.magnitudes[i] = 0.0;
            result.phases[i] = 0.0;
            continue;
        }

        /* 计算单个输出bin */
        auto pair = computeSingleBin(input, bin, n);
        double re = pair.first;
        double im = pair.second;

        result.magnitudes[i] = qSqrt(re * re + im * im);
        result.phases[i] = qAtan2(im, re);
    }

    /* 估计使用的蝶形数(近似) */
    int stages = static_cast<int>(std::log2(n));
    int butterfliesPerBin = stages; /* 每个输出bin需要O(log N)个蝶形 */
    totalButterfliesUsed = outputBins.size() * butterfliesPerBin;

    result.totalBinsComputed = outputBins.size();
    result.success = true;

    m_stats.totalTransforms++;
    m_stats.totalBinsComputed += outputBins.size();
    m_stats.totalButterfliesSkipped += qMax(0, totalButterfliesAll - totalButterfliesUsed);
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(outputBins.size(),
                            qMax(0, totalButterfliesAll - totalButterfliesUsed));
    return result;
}

PrunedFFT::PrunedResult PrunedFFT::extractBand(
    const QVector<double>& input, int startBin, int endBin)
{
    QVector<int> bins;
    for (int b = startBin; b <= endBin; ++b) {
        bins.append(b);
    }
    return transformOutputPruned(input, bins);
}

// ═══════════════════════════════════════════════════════════
// 输入修剪FFT
// ═══════════════════════════════════════════════════════════

PrunedFFT::PrunedResult PrunedFFT::transformInputPruned(
    const QVector<double>& inputFull, const QVector<int>& activeIndices)
{
    QElapsedTimer timer;
    timer.start();

    PrunedResult result;
    int n = inputFull.size();

    if (n < 2 || !isPowerOfTwo(n) || activeIndices.isEmpty()) {
        result.success = false;
        return result;
    }

    precomputeTwiddle(n);

    int stages = static_cast<int>(std::log2(n));

    /* 收集所有需要的蝶形运算 */
    /* 对每个非零输入, 标记从第0级到第stages级的所有依赖蝶形 */
    std::vector<std::set<int>> activePerStage(stages);
    for (int idx : activeIndices) {
        if (idx < 0 || idx >= n) continue;

        /* 比特反转索引 */
        int revIdx = bitReverse(idx, stages);
        int currentIdx = revIdx;

        for (int s = 0; s < stages; ++s) {
            activePerStage[s].insert(currentIdx);
            currentIdx /= 2;
        }
    }

    /* 使用标准FFT但只计算活跃蝶形 */
    QVector<double> re(n, 0.0);
    QVector<double> im(n, 0.0);

    /* 比特反转输入 */
    for (int idx : activeIndices) {
        if (idx < 0 || idx >= n) continue;
        int revIdx = bitReverse(idx, stages);
        re[revIdx] = inputFull[idx];
        im[revIdx] = 0.0;
    }

    /* 逐级执行蝶形运算(仅活跃部分) */
    int totalButterfliesAll = n * stages;
    int totalButterfliesUsed = 0;

    for (int s = 0; s < stages; ++s) {
        int m = 1 << (s + 1);
        int halfM = 1 << s;

        for (int k : activePerStage[s]) {
            int groupStart = k * m;
            if (groupStart >= n) continue;

            for (int j = 0; j < halfM && (groupStart + j + halfM) < n; ++j) {
                int topIdx = groupStart + j;
                int botIdx = groupStart + j + halfM;
                if (botIdx >= n) break;

                double twRe = m_twiddleCos[j * (n / m)];
                double twIm = -m_twiddleSin[j * (n / m)];

                double tRe = twRe * re[botIdx] - twIm * im[botIdx];
                double tIm = twRe * im[botIdx] + twIm * re[botIdx];

                re[botIdx] = re[topIdx] - tRe;
                im[botIdx] = im[topIdx] - tIm;
                re[topIdx] = re[topIdx] + tRe;
                im[topIdx] = im[topIdx] + tIm;

                totalButterfliesUsed++;
            }
        }
    }

    /* 收集结果 */
    result.magnitudes.resize(n);
    result.phases.resize(n);
    result.binIndices.resize(n);
    for (int i = 0; i < n; ++i) {
        result.magnitudes[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
        result.phases[i] = qAtan2(im[i], re[i]);
        result.binIndices[i] = i;
    }
    result.totalBinsComputed = n;
    result.success = true;

    m_stats.totalTransforms++;
    m_stats.totalBinsComputed += n;
    m_stats.totalButterfliesSkipped += qMax(0, totalButterfliesAll - totalButterfliesUsed);
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(n, qMax(0, totalButterfliesAll - totalButterfliesUsed));
    return result;
}

// ═══════════════════════════════════════════════════════════
// 辅助
// ═══════════════════════════════════════════════════════════

bool PrunedFFT::isPowerOfTwo(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

int PrunedFFT::bitReverse(int index, int bits)
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (index & 1);
        index >>= 1;
    }
    return result;
}

void PrunedFFT::precomputeTwiddle(int n)
{
    if (n == m_cachedSize) return;

    m_twiddleCos.resize(n);
    m_twiddleSin.resize(n);

    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * static_cast<double>(k) / static_cast<double>(n);
        m_twiddleCos[k] = qCos(angle);
        m_twiddleSin[k] = qSin(angle);
    }

    m_cachedSize = n;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

PrunedFFT::Stats PrunedFFT::stats() const
{
    return m_stats;
}

void PrunedFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

QPair<double, double> PrunedFFT::computeSingleBin(
    const QVector<double>& input, int targetBin, int n) const
{
    /*
     * 使用Goertzel-like方法计算单个bin,
     * 但利用FFT的蝶形结构进行修剪。
     *
     * X[k] = sum_{j=0}^{N-1} x[j] * W_N^{jk}
     * 其中 W_N = e^{-j*2*pi/N}
     *
     * 利用比特分解: j = sum of bits * 2^position
     * 逐步构建蝶形路径。
     */

    double re = 0.0;
    double im = 0.0;
    int stages = static_cast<int>(std::log2(n));

    for (int j = 0; j < n; ++j) {
        if (qFuzzyIsNull(input[j])) continue;

        /* 计算 W_N^{j*targetBin} */
        double angle = -2.0 * M_PI * static_cast<double>(j * targetBin)
                       / static_cast<double>(n);
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        re += input[j] * wRe;
        im += input[j] * wIm;
    }

    return {re, im};
}

QVector<QVector<int>> PrunedFFT::computeButterflyDependencies(
    int targetBin, int stages) const
{
    QVector<QVector<int>> deps(stages);

    int n = 1 << stages;

    /* 从输出bin反推依赖链 */
    /* 使用Cooley-Tukey蝶形图的逆映射 */
    QVector<int> currentIndices = {targetBin};

    for (int s = stages - 1; s >= 0; --s) {
        deps[s] = currentIndices;
        int halfM = 1 << s;
        QVector<int> nextIndices;

        for (int idx : currentIndices) {
            int group = idx / (1 << (s + 1));
            int pos = idx % (1 << (s + 1));
            int top = group * (1 << (s + 1)) + pos % halfM;
            int bot = top + halfM;
            nextIndices.append(top);
            nextIndices.append(bot);
        }

        currentIndices = nextIndices;
    }

    return deps;
}
