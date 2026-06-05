#include "SpectralFlatness3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱平坦度分析器
 * @param parent 父对象指针
 */
SpectralFlatness3::SpectralFlatness3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析帧大小(采样点数)
 * @param size 帧大小，通常为2的幂
 */
void SpectralFlatness3::setFrameSize(int size)
{
    m_frameSize = qMax(2, size);
}

/**
 * @brief 对输入信号帧计算频谱平坦度
 *
 * 频谱平坦度定义为功率谱的几何平均与算术平均之比，
 * 值接近1表示类噪声信号(平坦)，接近0表示类音调信号(峰值)。
 *
 * @param frame 输入信号帧
 * @return 频谱平坦度值(0.0~1.0)
 */
double SpectralFlatness3::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.size() < 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0.0);
        return 0.0;
    }

    const int N = frame.size();

    /* 计算功率谱(简化的DFT幅度平方) */
    int halfN = N / 2;
    QVector<double> powerSpectrum(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double realPart = 0.0;
        double imagPart = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            realPart += frame[n] * std::cos(angle);
            imagPart -= frame[n] * std::sin(angle);
        }
        double magnitude = std::sqrt(realPart * realPart + imagPart * imagPart);
        powerSpectrum[k] = magnitude * magnitude / (N * N);
    }

    /* 计算算术平均 */
    double arithmeticMean = 0.0;
    for (double v : powerSpectrum) arithmeticMean += v;
    arithmeticMean /= powerSpectrum.size();

    if (arithmeticMean < 1e-30) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0.0);
        return 0.0;
    }

    /* 计算几何平均(通过对数域避免下溢) */
    double logSum = 0.0;
    int validCount = 0;
    for (double v : powerSpectrum) {
        if (v > 1e-30) {
            logSum += std::log(v);
            validCount++;
        }
    }

    double flatness = 0.0;
    if (validCount > 0) {
        double geometricMean = std::exp(logSum / validCount);
        flatness = geometricMean / arithmeticMean;
        flatness = qBound(0.0, flatness, 1.0);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
    emit computed(flatness);
    return flatness;
}

/**
 * @brief 重置统计数据
 */
void SpectralFlatness3::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
