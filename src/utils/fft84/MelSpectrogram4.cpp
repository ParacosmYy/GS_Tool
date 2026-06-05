#include "MelSpectrogram4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Mel频谱图计算器
 * @param parent 父QObject对象指针
 */
MelSpectrogram4::MelSpectrogram4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算Mel频谱图
 *
 * 将线性频谱转换为Mel刻度频谱的完整流程：
 * 1. 分帧加窗（汉宁窗）
 * 2. 计算每帧的FFT幅度谱
 * 3. 通过Mel滤波器组将线性频谱映射到Mel频带
 * 4. 取对数得到dB刻度的Mel频谱
 *
 * @param samples 输入音频采样数据
 * @param sampleRate 采样率(Hz)
 * @param fftSize FFT长度
 * @param melBands Mel频带数，默认40
 * @return Mel频谱矩阵(帧数 x melBands)
 */
QVector<QVector<double>> MelSpectrogram4::compute(const QVector<double>& samples,
                                                    double sampleRate, int fftSize, int melBands)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    if (n == 0 || fftSize <= 0) return {};

    m_melBands = qMax(1, melBands);
    const int hopSize = fftSize / 4;  ///< 75%重叠
    const int numFrames = (n - fftSize) / hopSize + 1;

    if (numFrames <= 0) return {};

    /// 获取Mel滤波器组
    QVector<QVector<double>> filterBank = melFilterBank(fftSize, m_melBands, sampleRate);

    /// 逐帧处理
    QVector<QVector<double>> melSpectrogram;
    melSpectrogram.reserve(numFrames);

    for (int frame = 0; frame < numFrames; ++frame) {
        int start = frame * hopSize;

        /// 加汉宁窗并计算幅度谱
        QVector<double> magnitude(fftSize / 2 + 1, 0.0);
        for (int i = 0; i < fftSize; ++i) {
            int idx = start + i;
            double sample = (idx < n) ? samples[idx] : 0.0;
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (fftSize - 1)));

            /// 简化DFT（仅计算幅度谱的一半）
            for (int k = 0; k <= fftSize / 2; ++k) {
                double angle = 2.0 * M_PI * k * i / fftSize;
                magnitude[k] += sample * window * std::cos(angle);  ///< 仅实部近似
            }
        }

        /// 应用Mel滤波器组
        QVector<double> melFrame(m_melBands, 0.0);
        for (int m = 0; m < m_melBands; ++m) {
            double energy = 0.0;
            for (int k = 0; k < qMin(filterBank[m].size(), magnitude.size()); ++k) {
                energy += filterBank[m][k] * std::abs(magnitude[k]);
            }
            /// 取对数（dB刻度）
            melFrame[m] = (energy > 1e-10) ? 20.0 * std::log10(energy) : -100.0;
        }

        melSpectrogram.append(melFrame);
    }

    /// 更新统计信息
    m_stats.totalFramesComputed += numFrames;
    m_stats.totalMelBins += m_melBands;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesComputed);

    emit spectrogramComputed(numFrames, m_melBands);
    return melSpectrogram;
}

/**
 * @brief 构造Mel三角滤波器组
 *
 * 在Mel刻度上均匀分布三角滤波器，每个滤波器
 * 在中心频率处增益为1，两侧线性衰减到0。
 *
 * @param fftSize FFT长度
 * @param melBands Mel频带数
 * @param sampleRate 采样率(Hz)
 * @return 滤波器组矩阵(melBands x fftSize/2+1)
 */
QVector<QVector<double>> MelSpectrogram4::melFilterBank(int fftSize, int melBands,
                                                          double sampleRate) const
{
    const int numBins = fftSize / 2 + 1;
    const double fMax = sampleRate / 2.0;

    /// Hz转Mel的辅助函数
    auto hzToMel = [](double hz) { return 2595.0 * std::log10(1.0 + hz / 700.0); };
    auto melToHz = [](double mel) { return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0); };

    /// 在Mel刻度上均匀分布频带边界
    double melMin = hzToMel(0.0);
    double melMax = hzToMel(fMax);
    QVector<double> melPoints(melBands + 2);
    for (int i = 0; i < melBands + 2; ++i) {
        melPoints[i] = melToHz(melMin + (melMax - melMin) * i / (melBands + 1));
    }

    /// 转换为FFT bin索引
    QVector<int> binPoints(melBands + 2);
    for (int i = 0; i < melBands + 2; ++i) {
        binPoints[i] = static_cast<int>(melPoints[i] / fMax * (numBins - 1));
    }

    /// 构造三角滤波器
    QVector<QVector<double>> filterBank(melBands, QVector<double>(numBins, 0.0));
    for (int m = 0; m < melBands; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];

        for (int k = left; k <= center; ++k) {
            if (center > left) filterBank[m][k] = static_cast<double>(k - left) / (center - left);
        }
        for (int k = center; k <= right; ++k) {
            if (right > center) filterBank[m][k] = static_cast<double>(right - k) / (right - center);
        }
    }

    return filterBank;
}

/**
 * @brief 获取当前统计数据
 * @return 包含计算帧数、Mel频带数和平均耗时的Stats结构
 */
MelSpectrogram4::Stats MelSpectrogram4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void MelSpectrogram4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
