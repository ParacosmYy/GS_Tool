/**
 * @file GoertzelAlgorithm4.cpp
 * @brief Goertzel算法实现
 *
 * 实现高效的单一频率DFT检测算法，无需计算完整FFT。
 * 适用于DTMF检测、音调检测等只需要少量频率分量的场景。
 */

#include "utils/fft72/GoertzelAlgorithm4.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
GoertzelAlgorithm4::GoertzelAlgorithm4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析块大小
 * @param n 块大小（采样点数）
 */
void GoertzelAlgorithm4::setBlockSize(int n)
{
    m_blockSize = qMax(8, n);
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void GoertzelAlgorithm4::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
}

/**
 * @brief 添加目标检测频率
 * @param freq 目标频率(Hz)
 */
void GoertzelAlgorithm4::addTargetFreq(double freq)
{
    if (freq > 0.0 && freq < m_sampleRate / 2.0 && !m_targets.contains(freq)) {
        m_targets.append(freq);
    }
}

/**
 * @brief 清除所有目标频率
 */
void GoertzelAlgorithm4::clearTargets()
{
    m_targets.clear();
}

/**
 * @brief 计算所有目标频率的幅度
 * @param signal 输入信号
 * @return 频率到幅度的映射
 */
QMap<double, double> GoertzelAlgorithm4::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QMap<double, double> result;

    for (double freq : m_targets) {
        result[freq] = computeSingle(signal, freq);
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalComputations++;
    m_stats.totalBins += m_targets.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_targets.size(), m_blockSize);
    return result;
}

/**
 * @brief 计算单个频率的幅度
 * @param signal 输入信号
 * @param freq 目标频率(Hz)
 * @return 该频率的幅度值
 */
double GoertzelAlgorithm4::computeSingle(const QVector<double>& signal, double freq) const
{
    int N = qMin(signal.size(), m_blockSize);

    // 计算Goertzel系数
    double k = freq * N / m_sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);

    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    // 计算幅度
    double re = s1 - s2 * qCos(w);
    double im = s2 * qSin(w);
    double magnitude = qSqrt(re * re + im * im);

    return magnitude;
}

/**
 * @brief 重置统计信息
 */
void GoertzelAlgorithm4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
