#include "GaborTransform3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Gabor变换处理器
 * @param parent 父QObject对象指针
 */
GaborTransform3::GaborTransform3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行Gabor变换（高斯窗短时傅里叶变换）
 *
 * 使用高斯窗函数的STFT，提供最优的时频分辨率（Heisenberg不等式下界）：
 * 1. 将信号分帧，帧长由sigma参数决定
 * 2. 每帧乘以高斯窗 g(t) = exp(-t^2/(2*sigma^2))
 * 3. 计算每帧的DFT得到时频表示
 *
 * @param samples 输入采样数据
 * @param sigma 高斯窗宽度参数，控制时频分辨率权衡
 * @param hopSize 帧移（采样点数）
 * @return 时频系数矩阵(帧数 x 频率数)
 */
QVector<QVector<double>> GaborTransform3::transform(const QVector<double>& samples,
                                                      double sigma, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    if (N == 0 || sigma <= 0.0 || hopSize <= 0) return {};

    m_sigma = sigma;

    /// 根据sigma确定窗口大小（3sigma覆盖99.7%）
    const int windowSize = qMin(N, static_cast<int>(6.0 * sigma) | 1);  ///< 确保奇数
    const int halfWindow = windowSize / 2;
    const int numFrames = (N - windowSize) / hopSize + 1;
    const int freqBins = windowSize / 2 + 1;

    if (numFrames <= 0) return {};

    /// 预计算高斯窗
    QVector<double> gaussianWindow(windowSize);
    for (int i = 0; i < windowSize; ++i) {
        double t = static_cast<double>(i - halfWindow);
        gaussianWindow[i] = std::exp(-t * t / (2.0 * sigma * sigma));
    }

    /// 逐帧计算Gabor系数
    QVector<QVector<double>> coefficients;
    coefficients.reserve(numFrames);

    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * hopSize;
        QVector<double> frameCoeffs(freqBins, 0.0);

        /// 计算加窗DFT（实数幅度谱）
        for (int k = 0; k < freqBins; ++k) {
            double real = 0.0, imag = 0.0;
            for (int i = 0; i < windowSize; ++i) {
                int idx = start + i;
                double sample = (idx >= 0 && idx < N) ? samples[idx] : 0.0;
                double windowed = sample * gaussianWindow[i];
                double angle = 2.0 * M_PI * k * i / windowSize;
                real += windowed * std::cos(angle);
                imag -= windowed * std::sin(angle);
            }
            frameCoeffs[k] = std::sqrt(real * real + imag * imag);
        }

        coefficients.append(frameCoeffs);
    }

    /// 更新统计信息
    m_stats.totalTransforms++;
    m_stats.totalWindowsApplied += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, freqBins);
    return coefficients;
}

/**
 * @brief 逆Gabor变换重建信号
 *
 * 通过重叠-相加(Overlap-Add)方法从Gabor系数重建原始信号：
 * 1. 对每帧系数执行逆DFT
 * 2. 乘以高斯窗（综合窗）
 * 3. 重叠相加重建完整信号
 *
 * @param coefficients Gabor时频系数矩阵
 * @param outputLength 输出信号长度
 * @return 重建的时域信号
 */
QVector<double> GaborTransform3::inverse(const QVector<QVector<double>>& coefficients,
                                           int outputLength)
{
    QElapsedTimer timer;
    timer.start();

    const int numFrames = coefficients.size();
    if (numFrames == 0 || outputLength <= 0) return {};

    const int freqBins = coefficients[0].size();
    const int windowSize = (freqBins - 1) * 2;

    QVector<double> output(outputLength, 0.0);
    QVector<double> windowSum(outputLength, 0.0);

    /// 预计算综合高斯窗
    const int halfWindow = windowSize / 2;
    const int hopSize = outputLength / (numFrames + 1);
    QVector<double> synthWindow(windowSize);
    for (int i = 0; i < windowSize; ++i) {
        double t = static_cast<double>(i - halfWindow);
        synthWindow[i] = std::exp(-t * t / (2.0 * m_sigma * m_sigma));
    }

    /// 逐帧逆变换并重叠相加
    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * qMax(1, hopSize);

        /// 简化逆DFT（仅实部）
        for (int i = 0; i < windowSize; ++i) {
            int idx = start + i;
            if (idx >= outputLength) break;

            double sample = 0.0;
            for (int k = 0; k < freqBins; ++k) {
                double angle = 2.0 * M_PI * k * i / windowSize;
                sample += coefficients[frame][k] * std::cos(angle);
            }
            sample /= windowSize;

            output[idx] += sample * synthWindow[i];
            windowSum[idx] += synthWindow[i] * synthWindow[i];
        }
    }

    /// 归一化（除以窗函数重叠和）
    for (int i = 0; i < outputLength; ++i) {
        if (windowSum[i] > 1e-10) {
            output[i] /= windowSum[i];
        }
    }

    /// 更新统计信息
    m_timeSum += timer.elapsed();

    return output;
}

/**
 * @brief 获取当前统计数据
 * @return 包含变换次数、窗口应用数和平均耗时的Stats结构
 */
GaborTransform3::Stats GaborTransform3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void GaborTransform3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
