/**
 * @file RecursiveDFT3.cpp
 * @brief Goertzel算法实现
 *
 * 实现高效的单一频率DFT检测算法，无需计算完整FFT。
 * 适用于DTMF检测、音调检测等只需要少量频率分量的场景。
 */

#include "utils/fft75/RecursiveDFT3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
RecursiveDFT3::RecursiveDFT3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析块大小
 * @param n 块大小（采样点数）
 */
void RecursiveDFT3::setBlockSize(int n)
{
    m_blockSize = qMax(8, n);
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void RecursiveDFT3::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
}

/**
 * @brief 添加目标检测频率
 * @param freq 目标频率(Hz)
 */
void RecursiveDFT3::addTargetFreq(double freq)
{
    if (freq > 0.0 && freq < m_sampleRate / 2.0 && !m_targets.contains(freq)) {
        m_targets.append(freq);
    }
}

/**
 * @brief 清除所有目标频率
 */
void RecursiveDFT3::clearTargets()
{
    m_targets.clear();
}

/**
 * @brief 计算所有目标频率的幅度
 * @param signal 输入信号
 * @return 频率到幅度的映射
 *
 * 对每个目标频率独立执行Goertzel算法，计算该频率的DFT幅度。
 * 复杂度O(N * K)，K为目标频率数，当K远小于N时比FFT更高效。
 */
QMap<double, double> RecursiveDFT3::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QMap<double, double> result;

    for (double freq : m_targets) {
        result[freq] = computeSingle(signal, freq);
    }

    /* 更新统计信息 */
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
 *
 * Goertzel算法递推公式:
 *   s[n] = x[n] + coeff * s[n-1] - s[n-2]
 * 其中 coeff = 2 * cos(2*pi*k/N)
 * 最终幅度 = sqrt(Re^2 + Im^2)
 */
double RecursiveDFT3::computeSingle(const QVector<double>& signal, double freq) const
{
    int N = qMin(signal.size(), m_blockSize);
    if (N == 0) return 0.0;

    /* 计算Goertzel系数 */
    double k = freq * N / m_sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);

    /* 递推计算 */
    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 从最终状态计算复数DFT值 */
    double re = s1 - s2 * qCos(w);
    double im = s2 * qSin(w);

    /* 返回幅度 */
    double magnitude = qSqrt(re * re + im * im);

    /* 归一化 */
    magnitude /= (N / 2.0);

    return magnitude;
}

/**
 * @brief 重置统计信息
 */
void RecursiveDFT3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算单个频率的相位
 * @param signal 输入信号
 * @param freq 目标频率(Hz)
 * @return 相位值(弧度)
 */
double RecursiveDFT3::computePhase(const QVector<double>& signal, double freq) const
{
    int N = qMin(signal.size(), m_blockSize);
    if (N == 0) return 0.0;

    double k = freq * N / m_sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);

    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double re = s1 - s2 * qCos(w);
    double im = s2 * qSin(w);

    return qAtan2(im, re);
}

/**
 * @brief 计算单个频率的功率
 * @param signal 输入信号
 * @param freq 目标频率(Hz)
 * @return 功率值（幅度的平方）
 */
double RecursiveDFT3::computePower(const QVector<double>& signal, double freq) const
{
    double mag = computeSingle(signal, freq);
    return mag * mag;
}

/**
 * @brief 批量检测DTMF音调
 * @param signal 输入信号
 * @return 检测到的频率-幅度对（仅返回超过阈值的频率）
 */
QVector<QPair<double, double>> RecursiveDFT3::detectPeaks(
    const QVector<double>& signal, double threshold) const
{
    QVector<QPair<double, double>> peaks;

    for (double freq : m_targets) {
        double mag = computeSingle(signal, freq);
        if (mag > threshold) {
            peaks.append({freq, mag});
        }
    }

    // 按幅度降序排列
    std::sort(peaks.begin(), peaks.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return peaks;
}
