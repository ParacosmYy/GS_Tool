#include "Chromagram5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化色度图计算器
 * @param parent 父对象指针
 */
Chromagram5::Chromagram5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置八度范围
 * @param minOctave 最低八度编号
 * @param maxOctave 最高八度编号
 */
void Chromagram5::setOctaveRange(int minOctave, int maxOctave)
{
    m_minOctave = qMin(minOctave, maxOctave);
    m_maxOctave = qMax(minOctave, maxOctave);
}

/**
 * @brief 计算输入信号的色度特征
 *
 * 将频谱映射到12个音级(Pitch Class)上：
 * 1. 计算DFT频谱
 * 2. 将每个频率bin映射到最近的音级
 * 3. 累加相同音级的能量
 * 4. 归一化输出12维色度向量
 *
 * @param samples 输入音频采样
 */
void Chromagram5::compute(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        emit computed(0);
        return;
    }

    const int N = samples.size();
    double sampleRate = 44100.0;

    /* 计算DFT幅度谱 */
    int halfN = N / 2;
    QVector<double> magnitude(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += samples[n] * std::cos(angle);
            im += samples[n] * std::sin(angle);
        }
        magnitude[k] = std::sqrt(re * re + im * im);
    }

    /* 映射到12个音级 */
    const int numPitchClasses = 12;
    QVector<double> chroma(numPitchClasses, 0.0);

    /* 参考频率: C2 = 65.4064 Hz (MIDI note 36) */
    double refFreq = 65.4064;

    for (int k = 1; k < halfN; ++k) {
        double freq = static_cast<double>(k) * sampleRate / N;
        if (freq < refFreq || freq > 8000.0) continue;

        /* 计算音级: pitchClass = round(12 * log2(freq/refFreq)) % 12 */
        double semitones = 12.0 * std::log2(freq / refFreq);
        int pitchClass = static_cast<int>(std::round(semitones)) % numPitchClasses;
        if (pitchClass < 0) pitchClass += numPitchClasses;

        chroma[pitchClass] += magnitude[k] * magnitude[k];
    }

    /* 归一化 */
    double maxChroma = *std::max_element(chroma.begin(), chroma.end());
    if (maxChroma > 1e-10) {
        for (int i = 0; i < numPitchClasses; ++i) {
            chroma[i] /= maxChroma;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
    emit computed(1);
}

/**
 * @brief 重置统计数据
 */
void Chromagram5::resetStatistics()
{
    m_stats.totalComputed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
