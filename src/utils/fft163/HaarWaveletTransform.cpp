/**
 * @file HaarWaveletTransform.cpp
 * @brief HaarWaveletTransform 实现
 *
 * 实现Haar小波变换：多级正变换(cascade分解)、逐级逆变换(重构)、
 * 软阈值去噪和能量分布计算。
 */

#include "utils/fft163/HaarWaveletTransform.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
HaarWaveletTransform::HaarWaveletTransform(QObject* parent)
    : QObject(parent)
{
}

void HaarWaveletTransform::setMaxLevels(int levels)
{
    m_maxLevels = qMax(0, levels);
}

/**
 * @brief 补零到2的幂
 */
int HaarWaveletTransform::nextPowerOf2(int n)
{
    if (n <= 0) return 1;
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * @brief 单级Haar正变换
 *
 * 对长度N的输入生成N/2个近似系数和N/2个细节系数。
 * cA[i] = (input[2i] + input[2i+1]) / sqrt(2)
 * cD[i] = (input[2i] - input[2i+1]) / sqrt(2)
 */
HaarWaveletTransform::WaveletCoeffs HaarWaveletTransform::singleLevelForward(
    const QVector<double>& input) const
{
    const int n = input.size() / 2;
    WaveletCoeffs coeffs;
    coeffs.approximation.resize(n);
    coeffs.detail.resize(n);

    const double invSqrt2 = 1.0 / qSqrt(2.0);

    for (int i = 0; i < n; ++i) {
        double a = input[2 * i];
        double b = input[2 * i + 1];
        coeffs.approximation[i] = (a + b) * invSqrt2;
        coeffs.detail[i] = (a - b) * invSqrt2;
    }

    return coeffs;
}

/**
 * @brief 单级Haar逆变换
 *
 * output[2i]   = (approx[i] + detail[i]) / sqrt(2)
 * output[2i+1] = (approx[i] - detail[i]) / sqrt(2)
 */
QVector<double> HaarWaveletTransform::singleLevelInverse(
    const QVector<double>& approx, const QVector<double>& detail) const
{
    const int n = qMin(approx.size(), detail.size());
    QVector<double> output(n * 2);

    const double invSqrt2 = 1.0 / qSqrt(2.0);

    for (int i = 0; i < n; ++i) {
        output[2 * i] = (approx[i] + detail[i]) * invSqrt2;
        output[2 * i + 1] = (approx[i] - detail[i]) * invSqrt2;
    }

    return output;
}

/**
 * @brief 多级Haar小波正变换
 *
 * 逐级分解：每级对近似系数做单级变换，收集细节系数。
 * 自动补零至2的幂。
 */
QVector<HaarWaveletTransform::WaveletCoeffs> HaarWaveletTransform::forward(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return QVector<WaveletCoeffs>();

    /* 补零到2的幂 */
    int paddedLen = nextPowerOf2(signal.size());
    QVector<double> data(paddedLen, 0.0);
    for (int i = 0; i < signal.size(); ++i) {
        data[i] = signal[i];
    }

    /* 确定分解层数 */
    int levels = m_maxLevels;
    if (levels <= 0) {
        levels = 0;
        int n = paddedLen;
        while (n > 1) { n /= 2; levels++; }
    }
    levels = qMin(levels, static_cast<int>(qLn(paddedLen) / qLn(2)));

    QVector<WaveletCoeffs> allCoeffs;
    allCoeffs.reserve(levels);

    QVector<double> current = data;

    for (int l = 0; l < levels; ++l) {
        WaveletCoeffs coeffs = singleLevelForward(current);
        allCoeffs.append(coeffs);
        current = coeffs.approximation;
    }

    /* 最后一层的近似系数作为附加层 */
    WaveletCoeffs finalCoeffs;
    finalCoeffs.approximation = current;
    finalCoeffs.detail = QVector<double>(current.size(), 0.0);
    allCoeffs.append(finalCoeffs);

    m_stats.totalTransforms++;
    m_stats.totalLevels += levels;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(levels, signal.size());
    return allCoeffs;
}

/**
 * @brief 逆变换：从小波系数重构信号
 */
QVector<double> HaarWaveletTransform::inverse(
    const QVector<WaveletCoeffs>& coeffs, int originalLength)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) return QVector<double>();

    /* 从最深层开始逐级重构 */
    QVector<double> current = coeffs.last().approximation;

    for (int l = coeffs.size() - 2; l >= 0; --l) {
        current = singleLevelInverse(current, coeffs[l].detail);
    }

    /* 截断到原始长度 */
    if (originalLength > 0 && current.size() > originalLength) {
        current.resize(originalLength);
    }

    return current;
}

/**
 * @brief 软阈值去噪
 *
 * 对每层细节系数应用软阈值，然后逆变换重构。
 */
QVector<double> HaarWaveletTransform::denoise(const QVector<double>& signal, double threshold)
{
    if (signal.isEmpty()) return QVector<double>();

    QVector<WaveletCoeffs> coeffs = forward(signal);

    /* 对每层细节系数应用软阈值(跳过最后一层全零细节) */
    for (int l = 0; l < coeffs.size() - 1; ++l) {
        for (int i = 0; i < coeffs[l].detail.size(); ++i) {
            double val = coeffs[l].detail[i];
            if (qAbs(val) <= threshold) {
                coeffs[l].detail[i] = 0.0;
            } else if (val > threshold) {
                coeffs[l].detail[i] = val - threshold;
            } else {
                coeffs[l].detail[i] = val + threshold;
            }
        }
    }

    return inverse(coeffs, signal.size());
}

/**
 * @brief 计算各层能量分布
 */
QVector<double> HaarWaveletTransform::energyDistribution(
    const QVector<WaveletCoeffs>& coeffs) const
{
    QVector<double> energies;
    energies.reserve(coeffs.size());

    for (const auto& c : coeffs) {
        double detailEnergy = 0.0;
        for (double v : c.detail) {
            detailEnergy += v * v;
        }
        double approxEnergy = 0.0;
        for (double v : c.approximation) {
            approxEnergy += v * v;
        }
        energies.append(detailEnergy + approxEnergy);
    }

    return energies;
}

void HaarWaveletTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
