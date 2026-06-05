/**
 * @file EnvelopeDetector.cpp
 * @brief 信号包络检测器2实现 — 峰值/RMS/Hilbert/Attack-Release
 */

#include "utils/signal5/EnvelopeDetector.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
EnvelopeDetector2::EnvelopeDetector2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 峰值包络 @param signal 输入信号 @param windowSize 窗口大小 @return 包络曲线 */
QVector<double> EnvelopeDetector2::peakEnvelope(
    const QVector<double>& signal, int windowSize)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0 || windowSize <= 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /* 确保窗口为奇数 */
    if (windowSize % 2 == 0) ++windowSize;
    int half = windowSize / 2;

    QVector<double> envelope(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double maxVal = 0.0;
        int start = qMax(0, i - half);
        int end = qMin(n - 1, i + half);
        for (int j = start; j <= end; ++j) {
            double val = qAbs(signal[j]);
            if (val > maxVal) maxVal = val;
        }
        envelope[i] = maxVal;
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit envelopeDetected(tr("peak"), n);
    return envelope;
}

/** @brief RMS包络 @param signal 输入信号 @param windowSize 窗口大小 @return RMS包络曲线 */
QVector<double> EnvelopeDetector2::rmsEnvelope(
    const QVector<double>& signal, int windowSize)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0 || windowSize <= 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    QVector<double> envelope(n, 0.0);

    /* 预计算平方值 */
    QVector<double> sq(n);
    for (int i = 0; i < n; ++i) {
        sq[i] = signal[i] * signal[i];
    }

    int half = windowSize / 2;
    for (int i = 0; i < n; ++i) {
        int start = qMax(0, i - half);
        int end = qMin(n - 1, i + half);
        double sum = 0.0;
        for (int j = start; j <= end; ++j) {
            sum += sq[j];
        }
        int count = end - start + 1;
        envelope[i] = qSqrt(sum / static_cast<double>(count));
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit envelopeDetected(tr("rms"), n);
    return envelope;
}

/** @brief Hilbert变换(通过DFT) @param signal 输入信号 @return 虚部 */
QVector<double> EnvelopeDetector2::hilbertTransform(
    const QVector<double>& signal) const
{
    int n = signal.size();

    /* 找到大于等于n的2的幂次 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    /* DFT: 实数和虚数数组 */
    QVector<double> re(fftSize, 0.0), im(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        re[i] = signal[i];
    }

    /* Cooley-Tukey FFT (原地) */
    /* 位反转置换 */
    for (int i = 1, j = 0; i < fftSize; ++i) {
        int bit = fftSize >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= fftSize; len *= 2) {
        double ang = -2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < fftSize; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    /* Hilbert变换: 正频率*2, 负频率*0, DC和Nyquist不变 */
    for (int i = 1; i < fftSize / 2; ++i) {
        re[i] *= 2.0;
        im[i] *= 2.0;
    }
    for (int i = fftSize / 2 + 1; i < fftSize; ++i) {
        re[i] = 0.0;
        im[i] = 0.0;
    }

    /* 逆FFT */
    for (int i = 1, j = 0; i < fftSize; ++i) {
        int bit = fftSize >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    for (int len = 2; len <= fftSize; len *= 2) {
        double ang = 2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < fftSize; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    double inv = 1.0 / fftSize;
    QVector<double> hImag(n);
    for (int i = 0; i < n; ++i) {
        hImag[i] = im[i] * inv;
    }
    return hImag;
}

/** @brief Hilbert变换包络 @param signal 输入信号 @return 包络曲线 */
QVector<double> EnvelopeDetector2::hilbertEnvelope(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    QVector<double> hImag = hilbertTransform(signal);

    /* 包络 = sqrt(signal^2 + Hilbert^2) */
    QVector<double> envelope(n);
    for (int i = 0; i < n; ++i) {
        envelope[i] = qSqrt(signal[i] * signal[i] + hImag[i] * hImag[i]);
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit envelopeDetected(tr("hilbert"), n);
    return envelope;
}

/** @brief Attack-Release包络 @param signal 输入信号 @param attackCoeff 攻击系数 @param releaseCoeff 释放系数 @return 包络曲线 */
QVector<double> EnvelopeDetector2::attackReleaseEnvelope(
    const QVector<double>& signal,
    double attackCoeff,
    double releaseCoeff)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /* 限制系数在(0,1)范围 */
    attackCoeff = qBound(0.001, attackCoeff, 1.0);
    releaseCoeff = qBound(0.001, releaseCoeff, 1.0);

    QVector<double> envelope(n, 0.0);
    double level = 0.0;

    for (int i = 0; i < n; ++i) {
        double input = qAbs(signal[i]);
        if (input > level) {
            /* 攻击: 快速上升 */
            level += attackCoeff * (input - level);
        } else {
            /* 释放: 慢速下降 */
            level -= releaseCoeff * (level - input);
        }
        envelope[i] = level;
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit envelopeDetected(tr("attackRelease"), n);
    return envelope;
}

/** @brief 重置统计 */
void EnvelopeDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
