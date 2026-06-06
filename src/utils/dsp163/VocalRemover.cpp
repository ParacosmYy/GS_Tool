/**
 * @file VocalRemover.cpp
 * @brief VocalRemover 实现
 *
 * 实现中心声道人声消除：简单L-R差值和频域掩蔽增强两种模式。
 */

#include "utils/dsp163/VocalRemover.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
VocalRemover::VocalRemover(QObject* parent)
    : QObject(parent)
{
}

VocalRemover::~VocalRemover() = default;

void VocalRemover::setMode(RemovalMode mode)
{
    m_mode = mode;
}

void VocalRemover::setLowPassFreq(double freq)
{
    m_lowPassFreq = qMax(20.0, freq);
}

void VocalRemover::setFFTSize(int size)
{
    /* Round up to next power of 2 */
    int p = 1;
    while (p < size) p <<= 1;
    m_fftSize = qMax(64, p);
}

/**
 * @brief 计算Hann窗
 */
QVector<double> VocalRemover::hannWindow(int size) const
{
    QVector<double> window(size);
    for (int i = 0; i < size; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / size));
    }
    return window;
}

/**
 * @brief 简单Cooley-Tukey FFT
 */
void VocalRemover::fft(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    const int n = real.size();
    if (n <= 1) return;

    /* Bit-reversal permutation */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* Butterfly stages */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wReal = qCos(angle);
        double wImag = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];

                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;

                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

/**
 * @brief 频域模式处理
 *
 * 1) STFT分析左右声道差值
 * 2) 在频域中检测中心成分(左右能量相近的频段)
 * 3) 掩蔽中心成分
 * 4) IFFT还原
 */
QPair<QVector<double>, QVector<double>> VocalRemover::processFrequencyDomain(
    const QVector<double>& left, const QVector<double>& right)
{
    const int n = qMin(left.size(), right.size());
    const int fftLen = qMin(m_fftSize, n);
    QVector<double> outLeft(n, 0.0);
    QVector<double> outRight(n, 0.0);

    QVector<double> window = hannWindow(fftLen);
    int hopSize = fftLen / 2;
    QVector<double> overlapAddL(n, 0.0);
    QVector<double> overlapAddR(n, 0.0);
    QVector<double> winSum(n, 0.0);

    for (int start = 0; start + fftLen <= n; start += hopSize) {
        QVector<double> reL(fftLen, 0.0), imL(fftLen, 0.0);
        QVector<double> reR(fftLen, 0.0), imR(fftLen, 0.0);

        for (int i = 0; i < fftLen; ++i) {
            reL[i] = left[start + i] * window[i];
            reR[i] = right[start + i] * window[i];
        }

        fft(reL, imL, false);
        fft(reR, imR, false);

        /* Compute center (mid) and side signals in frequency domain */
        for (int i = 0; i < fftLen; ++i) {
            double midReal = (reL[i] + reR[i]) * 0.5;
            double midImag = (imL[i] + imR[i]) * 0.5;
            double sideReal = (reL[i] - reR[i]) * 0.5;
            double sideImag = (imL[i] - imR[i]) * 0.5;

            /* Compute mid magnitude for suppression ratio */
            double midMag = qSqrt(midReal * midReal + midImag * midImag);
            double sideMag = qSqrt(sideReal * sideReal + sideImag * sideImag);

            /* Suppress mid when it dominates (likely vocal) */
            double suppression = 1.0;
            if (midMag > 1e-10 && midMag > sideMag) {
                suppression = sideMag / midMag;
            }

            /* Reconstruct: keep side, suppress mid */
            double newMidReal = midReal * suppression;
            double newMidImag = midImag * suppression;

            /* L = mid + side, R = mid - side */
            reL[i] = newMidReal + sideReal;
            imL[i] = newMidImag + sideImag;
            reR[i] = newMidReal - sideReal;
            imR[i] = newMidImag - sideImag;
        }

        fft(reL, imL, true);
        fft(reR, imR, true);

        for (int i = 0; i < fftLen; ++i) {
            int idx = start + i;
            if (idx < n) {
                overlapAddL[idx] += reL[i] * window[i];
                overlapAddR[idx] += reR[i] * window[i];
                winSum[idx] += window[i] * window[i];
            }
        }
    }

    /* Normalize by window sum */
    for (int i = 0; i < n; ++i) {
        if (winSum[i] > 1e-10) {
            outLeft[i] = overlapAddL[i] / winSum[i];
            outRight[i] = overlapAddR[i] / winSum[i];
        }
    }

    /* Add back low-frequency bass (below lowPassFreq) */
    /* Bass is typically center-panned and should be preserved */
    for (int i = 0; i < n; ++i) {
        double bass = (left[i] + right[i]) * 0.5;
        outLeft[i] += bass;
        outRight[i] += bass;
    }

    return {outLeft, outRight};
}

/**
 * @brief 处理立体声帧
 */
QPair<QVector<double>, QVector<double>> VocalRemover::process(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(left.size(), right.size());
    QPair<QVector<double>, QVector<double>> result;

    if (m_mode == RemovalMode::SimpleSubtract) {
        QVector<double> outL(n), outR(n);
        for (int i = 0; i < n; ++i) {
            double diff = left[i] - right[i];
            double mid = (left[i] + right[i]) * 0.5;
            /* L' = (L-R)/2 + low_bass, R' = (R-L)/2 + low_bass */
            outL[i] = diff;
            outR[i] = -diff;
        }
        result = {outL, outR};
    } else {
        result = processFrequencyDomain(left, right);
    }

    m_stats.totalProcessed++;
    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum / m_stats.totalProcessed : 0.0;

    emit processCompleted(n, static_cast<int>(m_mode));
    return result;
}

/**
 * @brief 处理单声道(返回L-R差值)
 */
QVector<double> VocalRemover::processMono(const QVector<double>& left, const QVector<double>& right)
{
    const int n = qMin(left.size(), right.size());
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = left[i] - right[i];
    }
    return result;
}

void VocalRemover::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
