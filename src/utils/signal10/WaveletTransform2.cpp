/**
 * @file WaveletTransform2.cpp
 * @brief 连续小波变换实现 — Morlet/Paul/DOG小波时频分析
 */

#include "utils/signal10/WaveletTransform2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
WaveletTransform2::WaveletTransform2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void WaveletTransform2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置小波类型 @param type 小波类型 */
void WaveletTransform2::setWaveletType(WaveletType type)
{
    m_waveletType = type;
}

/** @brief 设置分析频率范围 @param minFreq 最低频率 @param maxFreq 最高频率 */
void WaveletTransform2::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = qMax(0.001, minFreq);
    m_maxFreq = qMax(m_minFreq + 1.0, maxFreq);
}

/** @brief 设置尺度数量 @param count 尺度数 */
void WaveletTransform2::setScaleCount(int count)
{
    m_scaleCount = qMax(4, count);
}

/** @brief 执行连续小波变换 @param signal 输入信号 @return CWT结果 */
WaveletTransform2::CwtResult WaveletTransform2::transform(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    CwtResult result;
    int N = signal.size();
    if (N < 4) return result;

    /* 生成对数等间距尺度数组 */
    double minScale = frequencyToScale(m_maxFreq);
    double maxScale = frequencyToScale(m_minFreq);
    if (minScale <= 0) minScale = 1.0;
    if (maxScale <= minScale) maxScale = minScale * 2.0;

    result.scales.resize(m_scaleCount);
    result.frequencies.resize(m_scaleCount);
    for (int i = 0; i < m_scaleCount; ++i) {
        double t = static_cast<double>(i) / qMax(1, m_scaleCount - 1);
        result.scales[i] = minScale * qPow(maxScale / minScale, t);
        result.frequencies[i] = scaleToFrequency(result.scales[i]);
    }

    result.magnitude.resize(m_scaleCount);
    result.phase.resize(m_scaleCount);
    result.timeSteps = N;
    result.scaleCount = m_scaleCount;

    /* 对每个尺度做卷积 */
    int halfN = N / 2;
    for (int s = 0; s < m_scaleCount; ++s) {
        double scale = result.scales[s];
        int waveletLen = qMin(N, qMax(4, static_cast<int>(scale * 8)));

        QPair<QVector<double>, QVector<double>> wavelet;
        switch (m_waveletType) {
        case WaveletType::Morlet:
            wavelet = morletWavelet(scale, waveletLen);
            break;
        case WaveletType::Paul:
            wavelet = paulWavelet(scale, waveletLen);
            break;
        case WaveletType::DOG:
            wavelet = dogWavelet(scale, waveletLen);
            break;
        }

        result.magnitude[s].resize(N);
        result.phase[s].resize(N);

        /* 滑动卷积 */
        int halfWave = waveletLen / 2;
        for (int t = 0; t < N; ++t) {
            double re = 0.0, im = 0.0;
            for (int k = 0; k < waveletLen; ++k) {
                int sigIdx = t - halfWave + k;
                if (sigIdx >= 0 && sigIdx < N) {
                    re += signal[sigIdx] * wavelet.first[k];
                    im += signal[sigIdx] * wavelet.second[k];
                }
            }
            /* 归一化 */
            double normFactor = qSqrt(static_cast<double>(waveletLen));
            re /= normFactor;
            im /= normFactor;

            result.magnitude[s][t] = qSqrt(re * re + im * im);
            result.phase[s][t] = qAtan2(im, re);
        }

        emit progress(s + 1, m_scaleCount);
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += N;
    m_stats.totalScalesComputed += m_scaleCount;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformComplete(N, m_scaleCount);
    return result;
}

/** @brief 快速CWT仅返回幅度 @param signal 输入信号 @return 幅度矩阵 */
QVector<QVector<double>> WaveletTransform2::transformMagnitude(
    const QVector<double>& signal)
{
    CwtResult result = transform(signal);
    return result.magnitude;
}

/** @brief 尺度转频率 @param scale 尺度 @return 频率(Hz) */
double WaveletTransform2::scaleToFrequency(double scale) const
{
    if (scale <= 0) return 0.0;

    switch (m_waveletType) {
    case WaveletType::Morlet:
        /* f = omega0 / (2*pi*s), omega0=6 for Morlet */
        return 6.0 / (2.0 * M_PI * scale) * m_sampleRate;
    case WaveletType::Paul:
        return 4.0 / (2.0 * M_PI * scale) * m_sampleRate;
    case WaveletType::DOG:
        return 2.0 / (2.0 * M_PI * scale) * m_sampleRate;
    }
    return 0.0;
}

/** @brief 频率转尺度 @param freq 频率 @return 尺度 */
double WaveletTransform2::frequencyToScale(double freq) const
{
    if (freq <= 0) return 0.0;
    /* 逆运算 */
    double s = scaleToFrequency(1.0);
    return s / freq;
}

/** @brief 计算Morlet小波在指定尺度的锥形影响域 @param scale 尺度 @return COI时间点数 */
int WaveletTransform2::coneOfInfluence(double scale) const
{
    /* Morlet COI = sqrt(2) * s */
    return qMax(1, static_cast<int>(qSqrt(2.0) * scale));
}

/** @brief 重置统计 */
void WaveletTransform2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 生成指定尺度的Morlet小波 */
QPair<QVector<double>, QVector<double>> WaveletTransform2::morletWavelet(
    double scale, int signalLength) const
{
    int halfLen = signalLength / 2;
    QVector<double> re(signalLength);
    QVector<double> im(signalLength);

    double omega0 = 6.0;
    double norm = qPow(M_PI, -0.25) / qSqrt(scale);

    for (int i = 0; i < signalLength; ++i) {
        double t = (static_cast<double>(i) - halfLen) / scale;
        double envelope = norm * qExp(-0.5 * t * t);
        re[i] = envelope * qCos(omega0 * t);
        im[i] = envelope * qSin(omega0 * t);
    }
    return {re, im};
}

/** @brief 生成Paul小波 */
QPair<QVector<double>, QVector<double>> WaveletTransform2::paulWavelet(
    double scale, int signalLength) const
{
    int halfLen = signalLength / 2;
    QVector<double> re(signalLength);
    QVector<double> im(signalLength);

    int m = 4; /* Paul阶数 */
    double norm = qPow(2.0, m) / qSqrt(static_cast<double>(factorial(m)));

    for (int i = 0; i < signalLength; ++i) {
        double t = (static_cast<double>(i) - halfLen) / scale;
        double envelope = norm * qPow(1.0 + t * t, -static_cast<double>(m + 1) / 2.0);
        double phase = static_cast<double>(m + 1) * qAtan(t);
        re[i] = envelope * qCos(phase) / qSqrt(scale);
        im[i] = envelope * qSin(phase) / qSqrt(scale);
    }
    return {re, im};
}

/** @brief 生成DOG(高斯导数)小波 */
QPair<QVector<double>, QVector<double>> WaveletTransform2::dogWavelet(
    double scale, int signalLength) const
{
    int halfLen = signalLength / 2;
    QVector<double> re(signalLength);
    QVector<double> im(signalLength);

    int order = 2; /* DOG阶数 */
    double norm = -1.0 / qSqrt(scale) * qPow(M_PI, -0.25);

    for (int i = 0; i < signalLength; ++i) {
        double t = (static_cast<double>(i) - halfLen) / scale;
        double gauss = qExp(-0.5 * t * t);
        /* DOG(m=2) = -d²/dx²(exp(-x²/2)) = (1-x²)*exp(-x²/2) */
        re[i] = norm * (1.0 - t * t) * gauss;
        im[i] = 0.0; /* DOG小波是实值小波 */
    }
    return {re, im};
}

/** @brief 阶乘辅助函数 */
long long WaveletTransform2::factorial(int n) const
{
    long long result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}
