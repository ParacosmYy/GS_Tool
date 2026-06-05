/**
 * @file GoertzelAlgorithm3.cpp
 * @brief Goertzel算法实现
 *
 * Goertzel算法是一种高效计算离散傅里叶变换（DFT）特定频率分量的算法。
 * 与FFT计算所有频率分量不同，Goertzel算法只计算指定的目标频率，
 * 因此在只需要检测少量频率时效率远高于FFT。
 *
 * 算法复杂度: O(N) per target frequency (FFT为 O(N log N) for all frequencies)
 * 空间复杂度: O(1) per target frequency (只需保存两个状态变量)
 *
 * 核心递推公式:
 *   s[n] = x[n] + 2*cos(2*pi*k/N)*s[n-1] - s[n-2],  n = 0, 1, ..., N-1
 *   s[-1] = s[-2] = 0
 *
 * 最终结果:
 *   |X[k]|^2 = s[N-1]^2 + s[N-2]^2 - 2*cos(2*pi*k/N)*s[N-1]*s[N-2]
 *
 * 典型应用场景:
 * - DTMF音调检测（电话按键音）
 * - 音频频率测量（乐器调音器）
 * - 简单的频谱监控（特定频率能量检测）
 * - 呼吸频带检测（生物信号处理）
 *
 * 与FFT对比:
 * - 检测1个频率: Goertzel O(N) vs FFT O(N log N) -- Goertzel更快
 * - 检测k个频率: Goertzel O(kN) vs FFT O(N log N)
 * - 当 k < log2(N)/2 时Goertzel更高效
 * - 例如: N=256, log2(N)/2=4, 检测4个以下频率Goertzel更优
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft53/GoertzelAlgorithm3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 *
 * 默认采样率44100Hz，块大小256采样。
 * 频率分辨率 = 44100/256 = 172.3Hz。
 * @param parent 父QObject对象指针
 */
GoertzelAlgorithm3::GoertzelAlgorithm3(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_blockSize(256)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置采样率
 *
 * 采样率决定了可检测的频率范围（0 ~ sampleRate/2）
 * 和频率分辨率（delta_f = sampleRate / blockSize）。
 *
 * @param sr 采样率（Hz），用于将目标频率转换为DFT bin索引
 */
void GoertzelAlgorithm3::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置分析块大小
 *
 * 块大小影响频率分辨率: delta_f = sampleRate / blockSize。
 * 块越大，频率分辨率越高，但时间分辨率越低。
 *
 * 典型设置（44.1kHz采样率）:
 * - N=256: 分辨率172Hz，适合DTMF检测
 * - N=512: 分辨率86Hz，适合音调检测
 * - N=1024: 分辨率43Hz，适合精确频率测量
 *
 * @param n 块大小（采样数），影响频率分辨率 delta_f = sr / n
 */
void GoertzelAlgorithm3::setBlockSize(int n)
{
    m_blockSize = qMax(1, n);
}

/**
 * @brief 添加目标检测频率
 *
 * 将目标频率加入检测列表。频率必须在奈奎斯特范围内 (0, sampleRate/2)。
 * 重复调用可添加多个目标频率。
 *
 * @param freq 目标频率（Hz），范围 (0, sampleRate/2)
 */
void GoertzelAlgorithm3::addTarget(double freq)
{
    if (freq > 0.0 && freq < m_sampleRate / 2.0) {
        m_targets.append(freq);
    }
}

/**
 * @brief 对信号执行Goertzel算法，计算所有目标频率的幅度
 *
 * 对输入信号逐个计算已添加目标频率的幅度响应。
 * 每个目标频率独立执行Goertzel递推，提取该频率分量的幅度。
 *
 * 处理步骤（对每个目标频率）:
 * 1. 计算DFT bin索引 k = freq * N / sampleRate
 * 2. 计算旋转因子系数 coeff = 2 * cos(2*pi*k/N)
 * 3. 执行N步递推: s[n] = x[n] + coeff*s[n-1] - s[n-2]
 * 4. 从最终状态计算功率和幅度
 *
 * @param signal 输入信号
 * @return 各目标频率的幅度值，与m_targets一一对应
 */
QVector<double> GoertzelAlgorithm3::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    QVector<double> magnitudes;

    if (N == 0 || m_targets.isEmpty()) {
        return magnitudes;
    }

    magnitudes.reserve(m_targets.size());

    /* 对每个目标频率执行Goertzel算法 */
    for (double targetFreq : m_targets) {
        /* 计算对应的DFT bin索引（浮点数，允许非整数bin以支持任意频率） */
        double k = targetFreq * N / m_sampleRate;
        double omega = 2.0 * M_PI * k / N;
        double coeff = 2.0 * qCos(omega);

        /* Goertzel递推: 只需要保存前两个状态 */
        double s0 = 0.0;  ///< s[n]: 当前状态
        double s1 = 0.0;  ///< s[n-1]: 上一步状态
        double s2 = 0.0;  ///< s[n-2]: 上两步状态

        for (int i = 0; i < N; ++i) {
            /* 核心递推: s[n] = x[n] + 2*cos(w)*s[n-1] - s[n-2] */
            s0 = signal[i] + coeff * s1 - s2;
            s2 = s1;
            s1 = s0;
        }

        /* 从最终状态计算功率: |X[k]|^2 = s1^2 + s2^2 - coeff*s1*s2 */
        double power = s1 * s1 + s2 * s2 - coeff * s1 * s2;

        /* 计算幅度（取绝对值防止微小负数） */
        double magnitude = qSqrt(qAbs(power));
        magnitudes.append(magnitude);
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalDetections++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    /* 找到峰值频率并发射信号 */
    double peakFreq = 0.0;
    double peakMag = 0.0;
    for (int i = 0; i < magnitudes.size(); ++i) {
        if (magnitudes[i] > peakMag) {
            peakMag = magnitudes[i];
            peakFreq = m_targets[i];
        }
    }

    emit detectionCompleted(peakFreq, peakMag);
    return magnitudes;
}

/**
 * @brief 计算指定频率的幅度响应
 *
 * 对输入信号在指定频率处执行Goertzel算法，返回该频率分量的幅度。
 * 这是一种快速的单频率检测方法，不需要计算完整的FFT。
 * 适用于实时频率检测（如调音器应用）。
 *
 * @param freq 目标频率（Hz）
 * @param signal 输入信号
 * @return 目标频率处的幅度值
 */
double GoertzelAlgorithm3::magnitudeAt(double freq, const QVector<double>& signal) const
{
    const int N = signal.size();
    if (N == 0) {
        return 0.0;
    }

    /* 计算DFT bin索引 */
    double k = freq * N / m_sampleRate;
    double omega = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(omega);

    /* Goertzel递推（紧凑版本） */
    double s1 = 0.0;
    double s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        double s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算幅度 */
    double power = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    return qSqrt(qAbs(power));
}

/**
 * @brief 重置所有统计计数器
 *
 * 将检测次数、总采样数、平均处理时间等统计指标归零。
 * 不影响采样率、块大小和目标频率列表。
 */
void GoertzelAlgorithm3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
