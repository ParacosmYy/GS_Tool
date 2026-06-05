/**
 * @file SpectralWhitening2.cpp
 * @brief 谱白化2实现 — 频谱平坦化+自适应增益
 *
 * 谱白化处理器，通过压缩频谱动态范围实现频谱平坦化。
 * 支持固定强度和自适应模式，计算谱平坦度和谱质心等指标。
 */

#include "utils/signal48/SpectralWhitening2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SpectralWhitening2::SpectralWhitening2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置FFT大小
 * @param fftSize FFT窗口大小
 */
void SpectralWhitening2::setFFTSize(int fftSize)
{
    m_fftSize = qMax(64, fftSize);
    m_avgSpectrum.clear();
    m_frameCount = 0;
}

/**
 * @brief 设置白化强度
 * @param strength 白化强度[0,1]，0=无白化，1=完全白化
 */
void SpectralWhitening2::setStrength(double strength)
{
    m_strength = qBound(0.0, strength, 1.0);
}

/**
 * @brief 启用/禁用自适应模式
 * @param adaptive true启用自适应强度调节
 */
void SpectralWhitening2::setAdaptive(bool adaptive)
{
    m_adaptive = adaptive;
}

/**
 * @brief 处理单个频谱帧，进行谱白化
 * @param spectrum 输入频谱幅度
 * @return 白化后的频谱幅度
 *
 * 白化公式: output = input^strength * avg^(strength-1)
 * strength=0时退化为原始频谱，strength=1时完全白化。
 */
QVector<double> SpectralWhitening2::process(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int n = spectrum.size();
    if (n == 0) return spectrum;

    /* 更新平均频谱估计 */
    if (m_avgSpectrum.size() != n) {
        m_avgSpectrum = spectrum;
        m_frameCount = 1;
    } else {
        double alpha = 0.05; /* EMA平滑系数 */
        for (int i = 0; i < n; ++i) {
            m_avgSpectrum[i] = (1.0 - alpha) * m_avgSpectrum[i] + alpha * spectrum[i];
        }
        m_frameCount++;
    }

    /* 计算平滑后的平均频谱 */
    QVector<double> smoothAvg = smoothSpectrum(m_avgSpectrum);

    /* 计算白化曲线 */
    QVector<double> whiteningCurve = computeWhiteningCurve(spectrum);

    /* 确定白化强度 */
    double effectiveStrength = m_strength;
    if (m_adaptive) {
        double flatness = spectralFlatness(spectrum);
        /* 频谱越不平坦，白化强度越大 */
        effectiveStrength = m_strength * (1.0 - flatness);
        effectiveStrength = qBound(0.0, effectiveStrength, 1.0);
    }

    /* 应用白化增益 */
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double original = spectrum[i];
        double whitened = (smoothAvg[i] > 1e-12)
                              ? original * whiteningCurve[i]
                              : original;

        /* 按强度混合原始和白化结果 */
        output[i] = (1.0 - effectiveStrength) * original + effectiveStrength * whitened;

        /* 确保非负 */
        output[i] = qMax(0.0, output[i]);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalFramesProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    double flatBefore = spectralFlatness(spectrum);
    double flatAfter = spectralFlatness(output);
    emit processingCompleted(flatBefore, flatAfter);

    return output;
}

/**
 * @brief 计算白化增益曲线
 * @param spectrum 输入频谱
 * @return 白化增益曲线（每个bin的增益因子）
 *
 * 增益 = avg / current，使得白化后所有频谱分量趋近于平均值。
 */
QVector<double> SpectralWhitening2::computeWhiteningCurve(const QVector<double>& spectrum) const
{
    const int n = spectrum.size();
    QVector<double> curve(n, 1.0);

    if (m_avgSpectrum.size() != n) return curve;

    QVector<double> smoothAvg = smoothSpectrum(m_avgSpectrum);

    for (int i = 0; i < n; ++i) {
        if (spectrum[i] > 1e-12 && smoothAvg[i] > 1e-12) {
            /* 白化增益 = sqrt(avg / current) */
            double ratio = smoothAvg[i] / spectrum[i];
            curve[i] = qSqrt(qBound(0.01, ratio, 100.0));
        }
    }

    return curve;
}

/**
 * @brief 计算谱平坦度
 * @param spectrum 输入频谱幅度
 * @return 谱平坦度[0,1]，1=完全平坦(白噪声)
 *
 * SFM = geometric_mean / arithmetic_mean
 * 在对数域计算以避免数值下溢。
 */
double SpectralWhitening2::spectralFlatness(const QVector<double>& spectrum) const
{
    const int n = spectrum.size();
    if (n == 0) return 0.0;

    double sumLog = 0.0;
    double sumLin = 0.0;
    int validCount = 0;

    for (int i = 0; i < n; ++i) {
        double mag = qMax(1e-12, spectrum[i]);
        sumLog += qLn(mag);
        sumLin += mag;
        validCount++;
    }

    if (validCount == 0 || sumLin < 1e-24) return 0.0;

    double geoMean = qExp(sumLog / validCount);
    double ariMean = sumLin / validCount;

    double flatness = (ariMean > 1e-24) ? geoMean / ariMean : 0.0;
    return qBound(0.0, flatness, 1.0);
}

/**
 * @brief 计算谱质心
 * @param spectrum 输入频谱幅度
 * @return 谱质心(归一化频率bin索引)
 *
 * 质心 = sum(f * |X(f)|) / sum(|X(f)|)
 */
double SpectralWhitening2::spectralCentroid(const QVector<double>& spectrum) const
{
    const int n = spectrum.size();
    if (n == 0) return 0.0;

    double weightedSum = 0.0;
    double totalWeight = 0.0;

    for (int i = 0; i < n; ++i) {
        double mag = qFabs(spectrum[i]);
        weightedSum += i * mag;
        totalWeight += mag;
    }

    return (totalWeight > 1e-24) ? weightedSum / totalWeight : 0.0;
}

/**
 * @brief 对频谱进行移动平均平滑
 * @param spec 输入频谱
 * @return 平滑后的频谱
 */
QVector<double> SpectralWhitening2::smoothSpectrum(const QVector<double>& spec) const
{
    const int n = spec.size();
    if (n == 0) return spec;

    QVector<double> smoothed(n, 0.0);
    const int halfWin = qMax(1, n / 32); /* 平滑窗口半径 */

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        int count = 0;
        int lo = qMax(0, i - halfWin);
        int hi = qMin(n - 1, i + halfWin);
        for (int j = lo; j <= hi; ++j) {
            sum += spec[j];
            count++;
        }
        smoothed[i] = (count > 0) ? sum / count : 0.0;
    }

    return smoothed;
}

/**
 * @brief 重置所有统计信息
 */
void SpectralWhitening2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
