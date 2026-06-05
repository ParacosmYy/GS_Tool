/**
 * @file NoiseProfile.cpp
 * @brief 噪声轮廓分析实现 — 背景噪声估计/频谱减法/噪声门/自适应阈值
 */

#include "utils/signal33/NoiseProfile.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
NoiseProfile::NoiseProfile(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void NoiseProfile::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置FFT大小 @param size FFT大小 */
void NoiseProfile::setFFTSize(int size)
{
    m_fftSize = qMax(64, size);
    m_noiseSpectrum.resize(m_fftSize / 2 + 1, 0.0);
    m_prevGain.resize(m_fftSize / 2 + 1, 1.0);
}

/** @brief 从纯噪声段估计噪声频谱 @param noiseSamples 噪声采样 */
void NoiseProfile::estimateNoise(const QVector<double>& noiseSamples)
{
    QElapsedTimer timer;
    timer.start();

    int halfN = m_fftSize / 2 + 1;
    m_noiseSpectrum.resize(halfN, 0.0);

    int numFrames = 0;
    int pos = 0;

    while (pos + m_fftSize <= noiseSamples.size()) {
        /* 加窗FFT */
        QVector<double> real(m_fftSize, 0.0);
        QVector<double> imag(m_fftSize, 0.0);

        for (int i = 0; i < m_fftSize; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
            real[i] = noiseSamples[pos + i] * w;
        }

        forwardFFT(real, imag);

        /* 累加功率谱 */
        for (int i = 0; i < halfN; ++i) {
            m_noiseSpectrum[i] += real[i] * real[i] + imag[i] * imag[i];
        }

        ++numFrames;
        pos += m_fftSize / 2; /* 50%重叠 */
    }

    /* 平均功率谱 */
    if (numFrames > 0) {
        for (int i = 0; i < halfN; ++i) {
            m_noiseSpectrum[i] /= numFrames;
        }
    }

    /* 初始化前帧增益 */
    m_prevGain.fill(1.0);

    m_stats.totalProfiles++;
    m_stats.totalFramesAnalyzed += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalProfiles));

    emit noiseEstimated(numFrames);
}

/** @brief 对输入信号进行降噪处理 @param input 输入采样 @return 降噪后采样 */
QVector<double> NoiseProfile::reduceNoise(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || m_noiseSpectrum.isEmpty()) return input;

    int halfN = m_fftSize / 2 + 1;
    QVector<double> output(input.size(), 0.0);
    int numFrames = 0;

    int pos = 0;
    while (pos + m_fftSize <= input.size()) {
        /* 加窗FFT */
        QVector<double> real(m_fftSize, 0.0);
        QVector<double> imag(m_fftSize, 0.0);

        for (int i = 0; i < m_fftSize; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
            real[i] = input[pos + i] * w;
        }

        forwardFFT(real, imag);

        /* 频谱减法 + 维纳滤波增益 */
        for (int i = 0; i < halfN; ++i) {
            double power = real[i] * real[i] + imag[i] * imag[i];
            double noisePower = m_noiseSpectrum[i] * m_strength;

            /* 维纳增益: max(1 - noisePower/power, floor) */
            double gain = 0.0;
            if (power > 1e-10) {
                double snrEst = power / qMax(1e-10, noisePower);
                gain = qMax(0.01, 1.0 - 1.0 / snrEst);
                gain = qPow(gain, 0.5); /* 过减模式 */

                /* 时间平滑: 避免音乐噪声 */
                gain = m_smoothing * m_prevGain[i] + (1.0 - m_smoothing) * gain;
            }
            m_prevGain[i] = gain;

            real[i] *= gain;
            imag[i] *= gain;

            /* 镜像频率 */
            if (i > 0 && i < halfN - 1) {
                real[m_fftSize - i] = real[i];
                imag[m_fftSize - i] = -imag[i];
            }
        }

        /* 逆FFT */
        forwardFFT(real, imag);
        for (int i = 0; i < m_fftSize; ++i) {
            real[i] /= m_fftSize;
        }

        /* 重叠相加 */
        double windowSum = 0.0;
        for (int i = 0; i < m_fftSize; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
            windowSum += w * w;
        }

        for (int i = 0; i < m_fftSize && pos + i < output.size(); ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
            output[pos + i] += real[i] * w / qMax(1e-10, windowSum / m_fftSize);
        }

        ++numFrames;
        pos += m_fftSize / 2;
    }

    /* 估算SNR */
    double signalEnergy = 0.0;
    double noiseEnergy = 0.0;
    for (int i = 0; i < input.size(); ++i) {
        signalEnergy += input[i] * input[i];
        double diff = output[i] - input[i];
        noiseEnergy += diff * diff;
    }

    m_stats.totalFramesAnalyzed += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalProfiles));

    double snr = (noiseEnergy > 1e-10)
        ? 10.0 * qLn(signalEnergy / noiseEnergy) / qLn(10.0) : 100.0;

    emit reductionComplete(input.size(), snr);
    return output;
}

/** @brief 获取噪声频谱 @return 功率谱 */
QVector<double> NoiseProfile::noiseSpectrum() const
{
    return m_noiseSpectrum;
}

/** @brief 获取信噪比 @return SNR(dB) */
double NoiseProfile::snr() const
{
    if (m_noiseSpectrum.isEmpty()) return 0.0;

    double totalNoise = 0.0;
    for (double v : m_noiseSpectrum) totalNoise += v;
    return (totalNoise > 1e-10) ? 10.0 * qLn(1.0 / totalNoise) / qLn(10.0) : 100.0;
}

/** @brief 设置降噪强度 @param strength 强度[0,3] */
void NoiseProfile::setReductionStrength(double strength)
{
    m_strength = qBound(0.0, strength, 3.0);
}

/** @brief 设置时间平滑系数 @param smooth 平滑系数[0.5,0.99] */
void NoiseProfile::setSmoothing(double smooth)
{
    m_smoothing = qBound(0.5, smooth, 0.99);
}

/** @brief 原地FFT(Cooley-Tukey) @param real 实部 @param imag 虚部 */
void NoiseProfile::forwardFFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    if (N <= 1) return;

    /* 位反转 */
    int bits = 0;
    for (int tmp = N; tmp > 1; tmp >>= 1) ++bits;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) j |= (1 << (bits - 1 - b));
        }
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle);
        double wI = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nr;
            }
        }
    }
}

/** @brief 重置统计 */
void NoiseProfile::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
