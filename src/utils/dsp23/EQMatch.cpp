/**
 * @file EQMatch.cpp
 * @brief EQ匹配引擎实现 — 源到目标匹配/最小相位/平滑插值
 */

#include "utils/dsp23/EQMatch.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
EQMatch::EQMatch(QObject* parent)
    : QObject(parent)
    , m_mode(MatchMode::MagnitudeOnly)
    , m_maxGain(12.0)
    , m_smoothness(0.5)
{
}

/** @brief 设置匹配模式 @param mode 匹配模式 */
void EQMatch::setMatchMode(MatchMode mode)
{
    m_mode = mode;
}

/** @brief 设置最大增益 @param maxGainDb 最大增益(dB) */
void EQMatch::setMaxGain(double maxGainDb)
{
    m_maxGain = qMax(0.1, maxGainDb);
}

/** @brief 设置平滑系数 @param smoothness 0~1 */
void EQMatch::setSmoothness(double smoothness)
{
    m_smoothness = qBound(0.0, smoothness, 1.0);
}

/** @brief 计算EQ匹配曲线 @param sourceMag 源幅度 @param targetMag 目标幅度 @param frequencies 频率轴 @return 匹配增益(dB) */
QVector<double> EQMatch::matchEQ(const QVector<double>& sourceMag,
                                  const QVector<double>& targetMag,
                                  const QVector<double>& frequencies)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(qMin(sourceMag.size(), targetMag.size()),
                 frequencies.size());
    if (n < 2) return {};

    /* 计算差值增益(目标-源) */
    QVector<double> gains(n);
    double matchError = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = targetMag[i] - sourceMag[i];
        gains[i] = qBound(-m_maxGain, diff, m_maxGain);
        matchError += qAbs(gains[i] - diff);
    }
    matchError /= n;

    /* 平滑增益曲线 */
    gains = smoothGainCurve(gains, frequencies);

    /* 最小相位变换(如果需要) */
    if (m_mode == MatchMode::MinimumPhase) {
        QVector<double> linearMag(n);
        for (int i = 0; i < n; ++i) {
            linearMag[i] = qPow(10.0, gains[i] / 20.0);
        }
        minimumPhaseImpulse(linearMag);
    }

    double elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalBinsProcessed += n;
    m_stats.lastMatchError = matchError;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalMatches);

    emit matchComplete(matchError);
    return gains;
}

/** @brief 设计参数EQ @param gains 增益曲线 @param frequencies 频率轴 @param numBands 频段数 @return EQ频段列表 */
QList<EQMatch::EQBand> EQMatch::designParametricEQ(
    const QVector<double>& gains, const QVector<double>& frequencies,
    int numBands)
{
    QList<EQBand> bands;
    if (gains.isEmpty() || frequencies.isEmpty()) return bands;

    int n = qMin(gains.size(), frequencies.size());
    numBands = qBound(1, numBands, n);

    /* 在对数频率轴上均匀分布频段 */
    double logMin = qLn(qMax(1.0, frequencies.first()));
    double logMax = qLn(qMax(1.0, frequencies.last()));
    double logRange = logMax - logMin;
    if (logRange < 1e-6) return bands;

    for (int b = 0; b < numBands; ++b) {
        double logCenter = logMin + (b + 0.5) * logRange / numBands;
        double centerFreq = qExp(logCenter);

        /* 在中心频率附近取平均增益 */
        double logHalfBand = logRange / (2.0 * numBands);
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n; ++i) {
            double logF = qLn(qMax(1.0, frequencies[i]));
            if (qAbs(logF - logCenter) < logHalfBand) {
                sum += gains[i];
                count++;
            }
        }

        EQBand band;
        band.frequency = centerFreq;
        band.gain = (count > 0) ? sum / count : 0.0;
        band.Q = qMax(0.1, centerFreq / (2.0 * (qExp(logHalfBand) - 1.0)));
        band.bandwidth = 2.0 * logHalfBand / qLn(2.0);
        bands.append(band);
    }

    emit eqDesigned(numBands);
    return bands;
}

/** @brief 最小相位冲激响应 @param magnitude 线性幅度谱 @return 冲激响应 */
QVector<double> EQMatch::minimumPhaseImpulse(const QVector<double>& magnitude)
{
    int n = magnitude.size();
    if (n < 4) return {};

    /* 计算倒谱: log|H|的实数逆DFT给出最小相位 */
    QVector<double> logMag(n);
    for (int i = 0; i < n; ++i) {
        logMag[i] = qLn(qMax(1e-10, magnitude[i]));
    }

    QVector<double> cepstrum = computeCepstrum(logMag);

    /* 对称化倒谱系数(因果部分) */
    int halfN = n / 2;
    QVector<double> phase(halfN, 0.0);
    for (int i = 1; i < halfN; ++i) {
        phase[i] = cepstrum[i];
    }

    return phase;
}

/** @brief 平滑增益曲线 @param gains 增益 @param frequencies 频率轴 @return 平滑后增益 */
QVector<double> EQMatch::smoothGainCurve(const QVector<double>& gains,
                                          const QVector<double>& frequencies)
{
    int n = gains.size();
    if (n < 3) return gains;

    /* 可变宽度高斯平滑(对数频率域) */
    int kernelHalf = qBound(1, static_cast<int>(m_smoothness * n * 0.1), n / 2);
    QVector<double> result(n);

    for (int i = 0; i < n; ++i) {
        double weightSum = 0.0;
        double valueSum = 0.0;
        for (int j = qMax(0, i - kernelHalf); j <= qMin(n - 1, i + kernelHalf); ++j) {
            double logDist = 0.0;
            if (i != j && frequencies.size() > j && frequencies.size() > i) {
                double fi = qLn(qMax(1.0, frequencies[i]));
                double fj = qLn(qMax(1.0, frequencies[j]));
                logDist = fi - fj;
            } else if (i != j) {
                logDist = static_cast<double>(i - j);
            }
            double w = qExp(-(logDist * logDist)
                            / (2.0 * m_smoothness * m_smoothness + 1e-10));
            weightSum += w;
            valueSum += gains[j] * w;
        }
        result[i] = (weightSum > 0) ? valueSum / weightSum : gains[i];
    }

    return result;
}

/** @brief 重置统计 */
void EQMatch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算倒谱 @param magnitude 对数幅度谱 @return 倒谱系数 */
QVector<double> EQMatch::computeCepstrum(const QVector<double>& magnitude)
{
    int n = magnitude.size();

    /* 构造对称序列做实数FFT */
    int fftSize = 1;
    while (fftSize < 2 * n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) real[i] = magnitude[i];
    for (int i = n; i < fftSize; ++i) real[i] = 0.0;

    /* 基2 FFT */
    for (int i = 1, j = 0; i < fftSize; ++i) {
        int bit = fftSize >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(real[i], real[j]);
    }
    for (int len = 2; len <= fftSize; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < fftSize; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tRe = cRe * real[o] - cIm * imag[o];
                double tIm = cRe * imag[o] + cIm * real[o];
                real[o] = real[e] - tRe;
                imag[o] = imag[e] - tIm;
                real[e] += tRe;
                imag[e] += tIm;
                double nRe = cRe * wRe - cIm * wIm;
                double nIm = cRe * wIm + cIm * wRe;
                cRe = nRe; cIm = nIm;
            }
        }
    }

    /* 取实部作为倒谱近似 */
    QVector<double> cepstrum(n);
    for (int i = 0; i < n; ++i) {
        cepstrum[i] = real[i] / fftSize;
    }
    return cepstrum;
}

/** @brief 查找峰值增益 @param gains 增益 @return 峰值 */
double EQMatch::findPeakGain(const QVector<double>& gains) const
{
    double peak = 0.0;
    for (double g : gains) peak = qMax(peak, qAbs(g));
    return peak;
}
