#include "Chromagram6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Chromagram6.cpp
 * @brief 色度图(Chromagram)计算器实现
 *
 * 将频谱能量映射到12个色度音级(C, C#, D, ..., B)，
 * 所有八度的同名音高合并到同一色度箱。
 * 用于和弦识别、音调分析和音乐信息检索。
 */

/// 12个半音的频率基准(A4 = 440Hz)
static const double A4_FREQ = 440.0;

/**
 * @brief 构造函数，初始化默认八度范围
 * @param parent 父QObject对象指针
 */
Chromagram6::Chromagram6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置八度范围
 * @param minOctave 最低八度编号(如2表示C2)
 * @param maxOctave 最高八度编号(如6表示C6)
 */
void Chromagram6::setOctaveRange(int minOctave, int maxOctave)
{
    m_minOctave = qMax(0, minOctave);
    m_maxOctave = qMax(m_minOctave, maxOctave);
}

/**
 * @brief 设置每八度的频率箱数
 * @param binsPerOctave 每个八度内的频率分辨率(通常为12)
 */
void Chromagram6::setBins(int binsPerOctave)
{
    m_bins = qMax(12, binsPerOctave);
}

/**
 * @brief 将频率映射到色度索引
 * @param freq 频率值(Hz)
 * @return 色度索引(0=C, 1=C#, ..., 11=B)
 */
static int freqToChroma(double freq)
{
    if (freq <= 0.0) return 0;
    // 计算MIDI音符号
    const double midiNote = 12.0 * std::log2(freq / A4_FREQ) + 69.0;
    return static_cast<int>(std::round(midiNote)) % 12;
}

/**
 * @brief 计算色度特征向量
 *
 * 处理流程:
 * 1. 对输入信号计算DFT频谱
 * 2. 计算每个频率箱对应的色度音级
 * 3. 将所有八度的同一色度音级的能量累加
 * 4. 归一化输出
 *
 * @param samples 输入音频采样数据
 * @return 12维色度特征向量(归一化)
 */
QVector<double> Chromagram6::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return QVector<double>(m_bins, 0.0);

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();

    // 计算频谱(简化DFT)
    const int numBins = N / 2;
    QVector<double> spectrum(numBins, 0.0);

    for (int k = 0; k < numBins; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            const double angle = -2.0 * M_PI * k * n / N;
            re += samples[n] * std::cos(angle);
            im += samples[n] * std::sin(angle);
        }
        spectrum[k] = std::sqrt(re * re + im * im);
    }

    // 计算色度向量
    QVector<double> chroma(m_bins, 0.0);
    const double sampleRate = 44100.0;
    const double freqResolution = sampleRate / N;

    for (int k = 0; k < numBins; ++k) {
        const double freq = k * freqResolution;

        // 检查是否在目标八度范围内
        const double minFreq = A4_FREQ * std::pow(2.0, (m_minOctave * 12 - 57) / 12.0);
        const double maxFreq = A4_FREQ * std::pow(2.0, (m_maxOctave * 12 + 12 - 57) / 12.0);

        if (freq >= minFreq && freq <= maxFreq) {
            const int chromaIdx = freqToChroma(freq) * m_bins / 12;
            if (chromaIdx >= 0 && chromaIdx < m_bins) {
                chroma[chromaIdx] += spectrum[k] * spectrum[k]; // 能量累加
            }
        }
    }

    // 归一化
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-10) {
        for (double& c : chroma) c /= maxVal;
    }

    // 更新统计信息
    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computed(1);
    return chroma;
}

/**
 * @brief 重置所有统计信息
 */
void Chromagram6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
