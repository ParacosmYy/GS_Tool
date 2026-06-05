/**
 * @file WienerFilter.cpp
 * @brief WienerFilter 实现
 *
 * 实现频域维纳滤波器：FFT分析、噪声PSD估计、维纳增益计算、IFFT合成。
 */

#include "utils/signal163/WienerFilter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
WienerFilter::WienerFilter(QObject* parent)
    : QObject(parent)
{
}

void WienerFilter::setFftSize(int nfft)
{
    int power = 1;
    while (power < nfft) power <<= 1;
    m_nfft = qMax(64, power);
}

void WienerFilter::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
}

void WienerFilter::setNoiseEstimation(NoiseEstimation method)
{
    m_noiseMethod = method;
}

void WienerFilter::setSmoothingFactor(double alpha)
{
    m_alpha = qBound(0.0, alpha, 1.0);
}

void WienerFilter::setOverSubtraction(double beta)
{
    m_beta = qMax(1.0, beta);
}

/**
 * @brief 就地基2 FFT
 */
void WienerFilter::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    if (n <= 1) return;

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
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nr;
            }
        }
    }
}

/**
 * @brief 就地基2 IFFT
 */
void WienerFilter::ifft(QVector<double>& real, QVector<double>& imag) const
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
 * @brief 从噪声参考信号估计噪声PSD
 *
 * 对纯噪声段做FFT，计算功率谱并平均。
 */
void WienerFilter::estimateNoiseProfile(const QVector<double>& noise)
{
    if (noise.size() < m_nfft) return;

    m_noisePSD.resize(m_nfft / 2 + 1, 0.0);
    int numFrames = 0;

    for (int start = 0; start + m_nfft <= noise.size(); start += m_nfft / 2) {
        QVector<double> real(m_nfft), imag(m_nfft, 0.0);
        /* 汉宁窗 */
        for (int i = 0; i < m_nfft; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_nfft - 1)));
            real[i] = noise[start + i] * w;
        }

        fft(real, imag);

        for (int i = 0; i <= m_nfft / 2; ++i) {
            double power = real[i] * real[i] + imag[i] * imag[i];
            m_noisePSD[i] += power;
        }
        numFrames++;
    }

    if (numFrames > 0) {
        for (auto& v : m_noisePSD) v /= numFrames;
    }

    m_noiseEstimated = true;
    m_priorSNR.resize(m_nfft / 2 + 1, 0.0);
    m_noiseMinBuf = m_noisePSD;
    m_minTrackLen = 0;
}

/**
 * @brief 最小值统计噪声跟踪
 *
 * 在运行时持续跟踪信号频谱的最小值作为噪声估计。
 */
void WienerFilter::updateNoiseEstimate(const QVector<double>& magnitude)
{
    if (!m_noiseEstimated) {
        /* 首次：用当前帧初始化 */
        m_noisePSD.resize(magnitude.size());
        for (int i = 0; i < magnitude.size(); ++i) {
            m_noisePSD[i] = magnitude[i] * magnitude[i];
        }
        m_noiseMinBuf = m_noisePSD;
        m_noiseEstimated = true;
        return;
    }

    m_minTrackLen++;

    for (int i = 0; i < qMin(magnitude.size(), m_noisePSD.size()); ++i) {
        double power = magnitude[i] * magnitude[i];

        /* 更新最小值缓冲 */
        m_noiseMinBuf[i] = qMin(m_noiseMinBuf[i] * 1.01, power);

        /* 缓慢衰减 */
        m_noisePSD[i] = 0.95 * m_noisePSD[i] + 0.05 * m_noiseMinBuf[i];
    }

    /* 周期性重置最小值缓冲 */
    if (m_minTrackLen > 100) {
        m_noiseMinBuf = m_noisePSD;
        m_minTrackLen = 0;
    }
}

/**
 * @brief 计算维纳增益
 *
 * G(k) = max((S(k) - β*N(k)) / S(k), floor)
 * 其中 S(k) = 信号PSD, N(k) = 噪声PSD
 */
QVector<double> WienerFilter::computeWienerGain(const QVector<double>& signalPSD,
                                                 const QVector<double>& noisePSD)
{
    const int k = qMin(signalPSD.size(), noisePSD.size());
    QVector<double> gain(k);

    for (int i = 0; i < k; ++i) {
        double s = qMax(signalPSD[i], 1e-20);
        double n = qMax(noisePSD[i], 1e-20);

        /* 维纳增益: H = max(1 - beta * N/S, floor) */
        double wiener = 1.0 - m_beta * n / s;
        gain[i] = qMax(wiener, 0.01); /* 下限防止完全静音 */
    }

    return gain;
}

/**
 * @brief 处理一帧含噪信号
 *
 * 1) FFT分析 → 2) 噪声PSD跟踪 → 3) 计算维纳增益 → 4) 增益应用 → 5) IFFT合成
 */
QVector<double> WienerFilter::process(const QVector<double>& noisySignal)
{
    QElapsedTimer timer;
    timer.start();

    if (noisySignal.size() < m_nfft) return noisySignal;

    /* 加窗FFT */
    QVector<double> real(m_nfft), imag(m_nfft, 0.0);
    for (int i = 0; i < m_nfft; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_nfft - 1)));
        real[i] = (i < noisySignal.size()) ? noisySignal[i] * w : 0.0;
    }
    fft(real, imag);

    /* 计算幅度谱和功率谱 */
    const int halfN = m_nfft / 2 + 1;
    QVector<double> magnitude(halfN);
    QVector<double> signalPSD(halfN);

    for (int i = 0; i < halfN; ++i) {
        magnitude[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
        signalPSD[i] = magnitude[i] * magnitude[i];
    }

    /* 噪声估计 */
    if (m_noiseMethod == NoiseEstimation::MinimumStatistics) {
        updateNoiseEstimate(magnitude);
    }

    if (!m_noiseEstimated) return noisySignal;

    /* 计算维纳增益 */
    QVector<double> gain = computeWienerGain(signalPSD, m_noisePSD);

    /* 先验SNR平滑(决策引导法) */
    for (int i = 0; i < halfN; ++i) {
        double postSNR = qMax(signalPSD[i] / qMax(m_noisePSD[i], 1e-20), 0.001);
        double priorSNR = m_alpha * m_priorSNR[i] + (1.0 - m_alpha) * qMax(postSNR - 1.0, 0.0);
        m_priorSNR[i] = priorSNR;

        /* 用先验SNR修正增益 */
        double refinedGain = priorSNR / (1.0 + priorSNR);
        gain[i] = qMax(gain[i] * 0.5 + refinedGain * 0.5, 0.01);
    }

    /* 应用增益 */
    for (int i = 0; i < halfN; ++i) {
        real[i] *= gain[i];
        imag[i] *= gain[i];
    }
    /* 对称部分 */
    for (int i = halfN; i < m_nfft; ++i) {
        int mirror = m_nfft - i;
        real[i] = real[mirror] * gain[mirror];
        imag[i] = -imag[mirror] * gain[mirror];
    }

    /* IFFT */
    ifft(real, imag);

    /* 计算SNR */
    double sigPow = 0.0, noisePow = 0.0;
    for (int i = 0; i < halfN; ++i) {
        sigPow += signalPSD[i] * gain[i] * gain[i];
        noisePow += m_noisePSD[i] * (1.0 - gain[i]) * (1.0 - gain[i]);
    }
    double snr = (noisePow > 1e-20) ? 10.0 * qLn(sigPow / noisePow) / qLn(10.0) : 60.0;

    m_stats.avgSNR = (m_stats.totalFrames == 0) ? snr
        : 0.95 * m_stats.avgSNR + 0.05 * snr;
    m_stats.estimatedNoiseFloor = 10.0 * qLn(m_noisePSD[0] + 1e-20) / qLn(10.0);

    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit frameProcessed(snr);
    return real;
}

void WienerFilter::reset()
{
    m_noisePSD.clear();
    m_priorSNR.clear();
    m_noiseMinBuf.clear();
    m_noiseEstimated = false;
    m_minTrackLen = 0;
}

void WienerFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
