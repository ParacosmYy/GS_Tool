#include "MelSpectrogram6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file MelSpectrogram6.cpp
 * @brief Mel频谱图计算器实现
 *
 * 将线性频率频谱通过Mel滤波器组映射到Mel刻度:
 * 1. 计算STFT频谱
 * 2. 构建Mel三角滤波器组
 * 3. 对频谱应用滤波器组得到Mel频谱
 * Mel刻度模拟人耳对频率的感知(对数尺度)。
 */

/**
 * @brief 构造函数，初始化默认Mel参数
 * @param parent 父QObject对象指针
 */
MelSpectrogram6::MelSpectrogram6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void MelSpectrogram6::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置Mel频率箱数量
 * @param bins Mel滤波器组的滤波器数量
 */
void MelSpectrogram6::setBinCount(int bins)
{
    m_binCount = qMax(1, bins);
}

/**
 * @brief Hz转Mel频率
 * @param hz 频率(Hz)
 * @return Mel频率
 */
static double hzToMel(double hz)
{
    return 2595.0 * std::log10(1.0 + hz / 700.0);
}

/**
 * @brief Mel频率转Hz
 * @param mel Mel频率
 * @return 频率(Hz)
 */
static double melToHz(double mel)
{
    return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
}

/**
 * @brief 计算Mel频谱特征
 *
 * 处理流程:
 * 1. 对输入信号分帧、加窗(汉宁窗)
 * 2. 计算每帧的FFT幅度谱
 * 3. 构建Mel三角滤波器组
 * 4. 对每帧幅度谱应用Mel滤波器组
 * 5. 输出Mel频谱矩阵(帧 x Mel箱)
 *
 * @param samples 输入音频采样数据
 * @return Mel频谱特征矩阵(每帧一个Mel特征向量)
 */
QVector<QVector<double>> MelSpectrogram6::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const int frameSize = 512;
    const int hopSize = 256;
    const int numFrames = (N - frameSize) / hopSize + 1;

    if (numFrames <= 0) return {};

    // 构建Mel滤波器组
    const double maxMel = hzToMel(m_sampleRate / 2.0);
    const int numFilters = m_binCount + 2; // 包含两端额外点
    QVector<double> melPoints(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        melPoints[i] = i * maxMel / (numFilters - 1);
    }

    // Mel点转回Hz再转FFT索引
    QVector<int> binPoints(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        const double hz = melToHz(melPoints[i]);
        binPoints[i] = static_cast<int>(hz * frameSize / m_sampleRate);
    }

    QVector<QVector<double>> melSpectrogram;

    for (int frame = 0; frame < numFrames; ++frame) {
        const int offset = frame * hopSize;

        // 计算幅度谱(加汉宁窗)
        QVector<double> spectrum(frameSize / 2 + 1, 0.0);
        for (int k = 0; k <= frameSize / 2; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < frameSize; ++n) {
                const int idx = offset + n;
                if (idx < N) {
                    const double win = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / frameSize));
                    const double angle = -2.0 * M_PI * k * n / frameSize;
                    re += samples[idx] * win * std::cos(angle);
                    im += samples[idx] * win * std::sin(angle);
                }
            }
            spectrum[k] = re * re + im * im; // 功率谱
        }

        // 应用Mel滤波器组
        QVector<double> melFrame(m_binCount, 0.0);
        for (int f = 0; f < m_binCount; ++f) {
            const int left = binPoints[f];
            const int center = binPoints[f + 1];
            const int right = binPoints[f + 2];

            for (int k = left; k <= center && k < spectrum.size(); ++k) {
                const double weight = static_cast<double>(k - left) / qMax(1, center - left);
                melFrame[f] += weight * spectrum[k];
            }
            for (int k = center + 1; k <= right && k < spectrum.size(); ++k) {
                const double weight = static_cast<double>(right - k) / qMax(1, right - center);
                melFrame[f] += weight * spectrum[k];
            }
        }

        melSpectrogram.append(melFrame);
    }

    // 更新统计信息
    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(numFrames);
    return melSpectrogram;
}

/**
 * @brief 重置所有统计信息
 */
void MelSpectrogram6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
