#include "SpectralBrightness3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱亮度分析器
 * @param parent 父QObject对象指针
 */
SpectralBrightness3::SpectralBrightness3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算单帧频谱的亮度特征
 *
 * 频谱亮度定义为高频能量占总能量的比例：
 * brightness = sum(|X(f)|^2, f > cutoff) / sum(|X(f)|^2, all f)
 *
 * 值域[0, 1]，值越大表示高频能量越丰富（声音越"亮"）。
 * cutoff频率通常设为4000Hz，对应语音中的清音/浊音分界。
 *
 * @param spectrum 单帧幅度谱
 * @param sampleRate 采样率(Hz)
 * @param cutoffHz 高低频分界频率(Hz)，默认4000Hz
 * @return 亮度值[0, 1]
 */
double SpectralBrightness3::compute(const QVector<double>& spectrum,
                                     double sampleRate, double cutoffHz)
{
    QElapsedTimer timer;
    timer.start();

    const int numBins = spectrum.size();
    if (numBins == 0) return 0.0;

    m_cutoffHz = qBound(100.0, cutoffHz, sampleRate / 2.0);

    /// 计算cutoff对应的频谱bin索引
    double binResolution = sampleRate / (2.0 * (numBins - 1));
    int cutoffBin = static_cast<int>(m_cutoffHz / binResolution);
    cutoffBin = qBound(0, cutoffBin, numBins - 1);

    /// 计算总能量和高频能量
    double totalEnergy = 0.0;
    double highFreqEnergy = 0.0;

    for (int i = 0; i < numBins; ++i) {
        double energy = spectrum[i] * spectrum[i];
        totalEnergy += energy;
        if (i >= cutoffBin) {
            highFreqEnergy += energy;
        }
    }

    /// 计算亮度比例
    double brightness = (totalEnergy > 1e-20) ? highFreqEnergy / totalEnergy : 0.0;
    brightness = qBound(0.0, brightness, 1.0);

    /// 更新统计信息
    m_stats.totalFramesAnalyzed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesAnalyzed;

    emit brightnessComputed(brightness, m_cutoffHz);
    return brightness;
}

/**
 * @brief 批量计算频谱亮度序列
 *
 * 对多帧频谱逐帧计算亮度，返回亮度时间序列。
 * 可用于分析音频的亮度变化趋势（如乐器音色演变）。
 *
 * @param spectra 频谱帧序列
 * @param sampleRate 采样率(Hz)
 * @param cutoffHz 高低频分界频率(Hz)，默认4000Hz
 * @return 亮度值序列[0, 1]
 */
QVector<double> SpectralBrightness3::computeSequence(const QVector<QVector<double>>& spectra,
                                                       double sampleRate, double cutoffHz)
{
    QElapsedTimer timer;
    timer.start();

    const int numFrames = spectra.size();
    if (numFrames == 0) return {};

    m_cutoffHz = qBound(100.0, cutoffHz, sampleRate / 2.0);

    QVector<double> brightnessValues;
    brightnessValues.reserve(numFrames);

    int peaksDetected = 0;

    for (int f = 0; f < numFrames; ++f) {
        const auto& spectrum = spectra[f];
        const int numBins = spectrum.size();

        if (numBins == 0) {
            brightnessValues.append(0.0);
            continue;
        }

        double binResolution = sampleRate / (2.0 * (numBins - 1));
        int cutoffBin = static_cast<int>(m_cutoffHz / binResolution);
        cutoffBin = qBound(0, cutoffBin, numBins - 1);

        double totalEnergy = 0.0;
        double highFreqEnergy = 0.0;

        for (int i = 0; i < numBins; ++i) {
            double energy = spectrum[i] * spectrum[i];
            totalEnergy += energy;
            if (i >= cutoffBin) {
                highFreqEnergy += energy;
            }
        }

        double brightness = (totalEnergy > 1e-20) ? highFreqEnergy / totalEnergy : 0.0;
        brightness = qBound(0.0, brightness, 1.0);
        brightnessValues.append(brightness);

        /// 检测亮度峰值（音色变化点）
        if (f > 0 && brightnessValues[f] > brightnessValues[f - 1] * 1.5) {
            ++peaksDetected;
        }
    }

    /// 更新统计信息
    m_stats.totalFramesAnalyzed += numFrames;
    m_stats.totalPeaksDetected += peaksDetected;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesAnalyzed;

    return brightnessValues;
}

/**
 * @brief 获取当前统计数据
 * @return 包含分析帧数、峰值数和平均耗时的Stats结构
 */
SpectralBrightness3::Stats SpectralBrightness3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void SpectralBrightness3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
