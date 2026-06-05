/**
 * @file SpectralCentroid2.cpp
 * @brief 频谱质心计算器实现
 *
 * 计算频谱的质心(重心频率)、带宽和扩展度:
 * - 质心(centroid): 频谱能量加权平均频率，描述频谱"亮度"
 * - 带宽(bandwidth): 频率偏离质心的加权平均距离，描述频谱宽度
 * - 扩展度(spread): 频率偏离质心的加权均方根距离，描述频谱离散程度
 *
 * 这些特征广泛用于音频分析、音色描述和音乐信息检索。
 * 质心值越高表示频谱能量集中在中高频，声音听起来更"明亮"；
 * 质心值越低表示能量集中在低频，声音听起来更"沉闷"。
 *
 * 数学定义:
 * 质心: centroid = sum(f[k] * X[k]) / sum(X[k])
 * 带宽: bandwidth = sum(|f[k] - centroid| * X[k]) / sum(X[k])
 * 扩展度: spread = sqrt(sum((f[k] - centroid)^2 * X[k]) / sum(X[k]))
 * 其中 f[k] = k * sampleRate / fftSize 为第k个频率bin的频率
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal62/SpectralCentroid2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化频谱质心计算器
 * @param parent 父QObject指针
 *
 * 默认参数: 采样率44100Hz, FFT大小1024点
 */
SpectralCentroid2::SpectralCentroid2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率 (Hz)，默认 44100.0
 *
 * 采样率决定了频率分辨率: freqRes = sr / fftSize
 */
void SpectralCentroid2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT点数，用于计算频率分辨率
 *
 * 频率分辨率 = sampleRate / fftSize
 * 每个频率bin对应的频率 = k * sampleRate / fftSize
 */
void SpectralCentroid2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 计算频谱质心
 *
 * 处理流程:
 * 1. 计算频率分辨率和每个bin对应的频率
 * 2. 计算频谱总能量作为归一化因子
 * 3. 使用能量加权计算质心频率
 * 4. 计算带宽(一阶中心矩)和扩展度(二阶中心矩)
 *
 * 边界情况处理:
 * - 空频谱: 所有输出为0
 * - 零能量频谱: 所有输出为0
 * - 单频率bin: 带宽和扩展度为0
 *
 * @param spectrum 输入频谱 (幅度谱或功率谱，长度为 fftSize/2+1)
 * @return 频谱质心频率 (Hz)
 */
double SpectralCentroid2::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    m_centroid = 0.0;
    m_bandwidth = 0.0;
    m_spread = 0.0;

    const int N = spectrum.size();
    if (N == 0) {
        m_stats.totalComputations++;
        m_stats.totalFrames++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computed(0.0, 0.0);
        return 0.0;
    }

    /* 频率分辨率: 每个频率bin对应的频率间隔 */
    double freqRes = m_sampleRate / m_fftSize;

    /* 步骤1: 计算频谱总能量 (归一化因子) */
    double totalEnergy = 0.0;
    for (int k = 0; k < N; ++k) {
        totalEnergy += spectrum[k];
    }

    /* 边界情况: 零能量频谱 */
    if (totalEnergy < 1e-15) {
        m_stats.totalComputations++;
        m_stats.totalFrames++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0.0, 0.0);
        return 0.0;
    }

    /* 预计算每个频率bin的频率值，避免在后续循环中重复计算 */
    /* freqs[k] = k * sampleRate / fftSize，表示第k个bin的中心频率 */
    QVector<double> freqs(N);
    for (int k = 0; k < N; ++k) {
        freqs[k] = k * freqRes;
    }

    /* 以下是三个统计量的计算，每个都需要遍历整个频谱 */

    /* 步骤2: 计算质心 (能量加权平均频率) */
    double weightedSum = 0.0;
    for (int k = 0; k < N; ++k) {
        weightedSum += freqs[k] * spectrum[k];
    }
    m_centroid = weightedSum / totalEnergy;

    /* 步骤3: 计算带宽 (一阶绝对中心矩) */
    /* 带宽衡量频谱能量偏离质心的平均距离 */
    double bwSum = 0.0;
    for (int k = 0; k < N; ++k) {
        double deviation = qAbs(freqs[k] - m_centroid);
        bwSum += deviation * spectrum[k];
    }
    m_bandwidth = bwSum / totalEnergy;

    /* 步骤4: 计算扩展度 (二阶中心矩的平方根) */
    /* 扩展度衡量频谱能量偏离质点的离散程度 */
    double spreadSum = 0.0;
    for (int k = 0; k < N; ++k) {
        double deviation = freqs[k] - m_centroid;
        spreadSum += deviation * deviation * spectrum[k];
    }
    m_spread = qSqrt(spreadSum / totalEnergy);

    /* 步骤4b: 计算频谱倾斜度 (三阶中心矩，可选特征) */
    /* skewness = sum((f-c)^3 * X) / (spread^3 * sum(X)) */
    /* 正倾斜表示高频能量较多，负倾斜表示低频能量较多 */
    /* 这个特征可以帮助进一步区分不同音色的频谱形状 */
    double skewSum = 0.0;
    for (int k = 0; k < N; ++k) {
        double deviation = freqs[k] - m_centroid;
        skewSum += deviation * deviation * deviation * spectrum[k];
    }
    /* 注意: skewness不直接存储，仅用于内部参考 */
    (void)skewSum; /* 抑制未使用警告 */

    /* 所有频谱特征计算完毕，更新统计信息 */

    /* 步骤5: 更新统计信息 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    /* 发送计算完成信号: 包含质心和带宽信息 */
    emit computed(m_centroid, m_bandwidth);
    return m_centroid;
}

/**
 * @brief 重置所有统计数据
 *
 * 重置统计计数器、计时器累积和计算结果。
 * 调用后 stats() 返回的统计值将全部为零，
 * centroid() / bandwidth() / spread() 均返回 0.0。
 */
void SpectralCentroid2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_centroid = 0.0;
    m_bandwidth = 0.0;
    m_spread = 0.0;
}

/* === 文件末尾: SpectralCentroid2.cpp === */
