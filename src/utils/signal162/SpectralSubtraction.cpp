/**
 * @file SpectralSubtraction.cpp
 * @brief 谱减法降噪器实现
 *
 * 实现完整的谱减法降噪流程：噪声谱估计 → 分帧加窗 → FFT →
 * 谱减 → 谱地板 → IFFT → overlap-add重建。
 */

#include "utils/signal162/SpectralSubtraction.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SpectralSubtraction::SpectralSubtraction(QObject* parent)
    : QObject(parent)
{
}

void SpectralSubtraction::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
    m_noiseEstimated = false;
}

void SpectralSubtraction::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

void SpectralSubtraction::setOversubtraction(double alpha)
{
    m_alpha = qBound(1.0, alpha, 3.0);
}

void SpectralSubtraction::setSpectralFloor(double beta)
{
    m_beta = qBound(0.0, beta, 1.0);
}

void SpectralSubtraction::setNoiseEstimationFrames(int frames)
{
    m_noiseFrames = qMax(1, frames);
}

void SpectralSubtraction::setNoiseSpectrum(const QVector<double>& noiseMagnitude)
{
    m_noiseSpectrum = noiseMagnitude;
    m_noiseEstimated = true;
}

/**
 * @brief 汉宁窗
 */
QVector<double> SpectralSubtraction::hanningWindow(int size) const
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i) {
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
    }
    return w;
}

/**
 * @brief 基2 FFT(就地Cooley-Tukey)
 */
void SpectralSubtraction::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    if (n <= 1) return;

    /* 位反转 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uR = real[i + j], uI = imag[i + j];
                double tR = cR * real[i + j + len / 2] - cI * imag[i + j + len / 2];
                double tI = cR * imag[i + j + len / 2] + cI * real[i + j + len / 2];
                real[i + j] = uR + tR;
                imag[i + j] = uI + tI;
                real[i + j + len / 2] = uR - tR;
                imag[i + j + len / 2] = uI - tI;
                double newCR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = newCR;
            }
        }
    }
}

/**
 * @brief 基2 IFFT
 */
void SpectralSubtraction::ifft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    for (auto& v : imag) v = -v;
    fft(real, imag);
    double inv = 1.0 / n;
    for (int i = 0; i < n; ++i) {
        real[i] *= inv;
        imag[i] = -imag[i] * inv;
    }
}

/**
 * @brief 从起始帧估计噪声幅度谱
 */
void SpectralSubtraction::estimateNoise(const QVector<double>& signal)
{
    m_noiseSpectrum.resize(m_frameSize / 2 + 1, 0.0);
    QVector<double> window = hanningWindow(m_frameSize);

    int count = 0;
    for (int start = 0; start + m_frameSize <= signal.size() && count < m_noiseFrames;
         start += m_hopSize) {
        QVector<double> frameR(m_frameSize, 0.0), frameI(m_frameSize, 0.0);
        for (int i = 0; i < m_frameSize; ++i) {
            frameR[i] = signal[start + i] * window[i];
        }
        fft(frameR, frameI);

        for (int i = 0; i < m_frameSize / 2 + 1; ++i) {
            m_noiseSpectrum[i] += qSqrt(frameR[i] * frameR[i] + frameI[i] * frameI[i]);
        }
        count++;
    }

    if (count > 0) {
        for (auto& v : m_noiseSpectrum) v /= count;
    }
    m_noiseEstimated = true;
}

/**
 * @brief 对音频信号执行谱减法降噪
 */
QVector<double> SpectralSubtraction::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int sigLen = input.size();
    if (sigLen < m_frameSize) return input;

    /* 噪声估计 */
    if (!m_noiseEstimated) {
        estimateNoise(input);
    }

    QVector<double> window = hanningWindow(m_frameSize);
    QVector<double> output(sigLen, 0.0);
    QVector<double> winSum(sigLen, 0.0);

    double frameSnrSum = 0.0;
    int frameCount = 0;

    for (int start = 0; start + m_frameSize <= sigLen; start += m_hopSize) {
        /* 分帧加窗 */
        QVector<double> frameR(m_frameSize, 0.0), frameI(m_frameSize, 0.0);
        for (int i = 0; i < m_frameSize; ++i) {
            frameR[i] = input[start + i] * window[i];
        }

        /* FFT */
        fft(frameR, frameI);

        /* 计算幅度谱和相位 */
        QVector<double> mag(m_frameSize / 2 + 1);
        for (int i = 0; i <= m_frameSize / 2; ++i) {
            mag[i] = qSqrt(frameR[i] * frameR[i] + frameI[i] * frameI[i]);
        }

        /* 谱减法 */
        double noisePower = 0.0;
        double signalPower = 0.0;
        for (int i = 0; i <= m_frameSize / 2; ++i) {
            double noisyMag = mag[i];
            double noiseMag = m_noiseSpectrum[i];
            signalPower += noisyMag * noisyMag;
            noisePower += noiseMag * noiseMag;

            /* 谱减: |S|² = |Y|² - α * |N|² */
            double magSq = noisyMag * noisyMag - m_alpha * noiseMag * noiseMag;

            /* 谱地板 */
            double floor = m_beta * noisyMag * noisyMag;
            magSq = qMax(magSq, floor);

            /* 重建频谱(保留原始相位) */
            double cleanMag = qSqrt(qMax(magSq, 0.0));
            double scale = (noisyMag > 1e-20) ? cleanMag / noisyMag : 0.0;
            frameR[i] *= scale;
            frameI[i] *= scale;
        }

        /* 对称补全 */
        for (int i = m_frameSize / 2 + 1; i < m_frameSize; ++i) {
            frameR[i] = frameR[m_frameSize - i];
            frameI[i] = -frameI[m_frameSize - i];
        }

        /* IFFT */
        ifft(frameR, frameI);

        /* Overlap-Add */
        for (int i = 0; i < m_frameSize && start + i < sigLen; ++i) {
            output[start + i] += frameR[i] * window[i];
            winSum[start + i] += window[i] * window[i];
        }

        /* SNR计算 */
        double snr = (noisePower > 1e-20)
            ? 10.0 * qLn(signalPower / noisePower) / qLn(10.0) : 30.0;
        frameSnrSum += snr;
        frameCount++;

        m_stats.totalFrames++;
    }

    /* 归一化 */
    for (int i = 0; i < sigLen; ++i) {
        if (winSum[i] > 1e-10) output[i] /= winSum[i];
    }

    /* 统计更新 */
    double avgNoisePower = 0.0;
    for (double v : m_noiseSpectrum) avgNoisePower += v * v;
    avgNoisePower = (m_noiseSpectrum.size() > 0) ? avgNoisePower / m_noiseSpectrum.size() : 0.0;
    m_noisePowerSum += 10.0 * qLn(qMax(avgNoisePower, 1e-20)) / qLn(10.0);
    m_stats.avgNoisePowerDb = m_noisePowerSum / (m_stats.totalFrames > 0 ? m_stats.totalFrames : 1);

    if (frameCount > 0) {
        double avgSnr = frameSnrSum / frameCount;
        m_snrImprovementSum += avgSnr;
        m_stats.avgSnrImprovementDb = m_snrImprovementSum / m_stats.totalFrames;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit frameProcessed(frameCount, (frameCount > 0) ? frameSnrSum / frameCount : 0.0);
    return output;
}

void SpectralSubtraction::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_noisePowerSum = 0.0;
    m_snrImprovementSum = 0.0;
}
