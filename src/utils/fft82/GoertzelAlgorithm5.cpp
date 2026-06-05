#include "GoertzelAlgorithm5.h"
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 构造函数，初始化Goertzel算法处理器
 * @param parent 父QObject对象指针
 */
GoertzelAlgorithm5::GoertzelAlgorithm5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 使用Goertzel算法检测指定频率的幅值
 *
 * Goertzel算法是DFT在单个频率点的高效实现，复杂度O(N)。
 * 通过二阶IIR滤波器递推计算，避免完整FFT运算。
 * 适用于DTMF检测、单音检测等只需要少量频率分量的场景。
 *
 * @param samples 输入采样数据
 * @param targetFreqHz 目标检测频率(Hz)
 * @param sampleRate 采样率(Hz)
 * @return 目标频率的幅值
 */
double GoertzelAlgorithm5::detectMagnitude(const QVector<double>& samples,
                                            double targetFreqHz, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    if (N == 0) return 0.0;

    /// 计算归一化频率系数
    const double k = static_cast<double>(N) * targetFreqHz / sampleRate;
    const double w = 2.0 * M_PI * k / N;
    const double coeff = 2.0 * std::cos(w);

    /// Goertzel递推计算
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /// 计算幅值（复数结果的模）
    double real = s1 - s2 * std::cos(w);
    double imag = s2 * std::sin(w);
    double magnitude = std::sqrt(real * real + imag * imag);

    /// 更新统计信息
    m_stats.totalFrequenciesDetected++;
    m_stats.totalBlocksAnalyzed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocksAnalyzed;

    emit frequencyDetected(targetFreqHz, magnitude);
    return magnitude;
}

/**
 * @brief 批量检测多个频率的幅值
 *
 * 对同一数据块依次应用Goertzel算法检测多个目标频率。
 * 比完整FFT更高效，当只需要少量频率分量时。
 * 典型应用：DTMF双音多频检测需要8个频率。
 *
 * @param samples 输入采样数据
 * @param freqsHz 目标频率列表(Hz)
 * @param sampleRate 采样率(Hz)
 * @return QPair(频率, 幅值)的结果列表
 */
QVector<QPair<double, double>> GoertzelAlgorithm5::detectMulti(
    const QVector<double>& samples, const QVector<double>& freqsHz, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    QVector<QPair<double, double>> results;
    results.reserve(freqsHz.size());

    /// 对每个目标频率执行Goertzel算法
    for (double freq : freqsHz) {
        if (N == 0) {
            results.append(qMakePair(freq, 0.0));
            continue;
        }

        const double k = static_cast<double>(N) * freq / sampleRate;
        const double w = 2.0 * M_PI * k / N;
        const double coeff = 2.0 * std::cos(w);

        double s0 = 0.0, s1 = 0.0, s2 = 0.0;
        for (int i = 0; i < N; ++i) {
            s0 = samples[i] + coeff * s1 - s2;
            s2 = s1;
            s1 = s0;
        }

        double real = s1 - s2 * std::cos(w);
        double imag = s2 * std::sin(w);
        double magnitude = std::sqrt(real * real + imag * imag);
        results.append(qMakePair(freq, magnitude));
    }

    /// 更新统计信息
    m_stats.totalFrequenciesDetected += freqsHz.size();
    m_stats.totalBlocksAnalyzed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocksAnalyzed;

    return results;
}

/**
 * @brief 获取当前统计数据
 * @return 包含检测频率数、分析块数和平均耗时的Stats结构
 */
GoertzelAlgorithm5::Stats GoertzelAlgorithm5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void GoertzelAlgorithm5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
