#include "Chromagram4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class Chromagram4
 * @brief 色度图(Chromagram)分析器实现
 *
 * 色度图将频谱能量映射到12个音高类别(音级):
 * C, C#, D, D#, E, F, F#, G, G#, A, A#, B
 * 所有八度中同一音名的能量叠加到同一bin。
 *
 * 映射原理: 对于频率f，其音级 = round(12 * log2(f/440)) mod 12
 * 其中A4 = 440Hz为参考频率。
 *
 * 应用: 和弦识别、调性分析、音乐信息检索。
 */

/**
 * @brief 音级名称(用于调试)
 */
static const char* NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
Chromagram4::Chromagram4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算单帧色度向量(12维)
 *
 * 从频谱中提取12维色度向量:
 * 1. 对每个频率bin计算其对应的音级索引
 * 2. 将同一音级的能量累加
 * 3. 归一化色度向量
 *
 * 频率范围: C1(~32Hz) ~ B8(~7902Hz)
 *
 * @param spectrum 频谱幅值(FFT后的幅度谱)
 * @param sampleRate 采样率(Hz)
 * @return 12维色度向量(0~1归一化)
 */
QVector<double> Chromagram4::compute(const QVector<double>& spectrum, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> chroma(12, 0.0);

    if (spectrum.isEmpty() || sampleRate <= 0.0) {
        m_timeSum += timer.elapsed();
        return chroma;
    }

    int N = spectrum.size();

    /* 遍历频率bin，映射到音级 */
    for (int i = 1; i < N; ++i) {
        /* 计算bin对应的频率 */
        double freq = static_cast<double>(i) * sampleRate / (2.0 * N);

        /* 跳过极低和极高频率 */
        if (freq < 32.0 || freq > 8000.0) continue;

        /* 计算音级: pitchClass = round(12 * log2(f/440)) mod 12 */
        double semitones = 12.0 * qLn(freq / 440.0) / qLn(2.0);
        int pitchClass = static_cast<int>(qRound(semitones)) % 12;
        if (pitchClass < 0) pitchClass += 12;

        /* 累加能量 */
        chroma[pitchClass] += spectrum[i] * spectrum[i];
    }

    /* 归一化(除以最大值或总和) */
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-15) {
        for (int i = 0; i < 12; ++i) {
            chroma[i] /= maxVal;
        }
    }

    /* 找到主导音级 */
    int dominantPitch = 0;
    double dominantVal = 0.0;
    for (int i = 0; i < 12; ++i) {
        if (chroma[i] > dominantVal) {
            dominantVal = chroma[i];
            dominantPitch = i;
        }
    }

    m_stats.totalFramesComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesComputed);

    emit chromagramComputed(m_stats.totalFramesComputed - 1, dominantPitch);

    return chroma;
}

/**
 * @brief 批量计算色度图序列
 *
 * 使用滑动窗口逐帧提取色度向量，组成色度图矩阵。
 * 每帧先做FFT得到频谱，再映射到色度空间。
 *
 * @param samples 完整音频采样序列
 * @param sampleRate 采样率(Hz)
 * @param frameSize 帧大小(采样点，建议2的幂)
 * @param hopSize 帧移(采样点)
 * @return 色度图序列，每行为一帧的12维色度向量
 */
QVector<QVector<double>> Chromagram4::computeSequence(const QVector<double>& samples,
                                                       double sampleRate, int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> chromaSeq;

    if (samples.size() < frameSize || hopSize <= 0 || sampleRate <= 0.0) {
        m_timeSum += timer.elapsed();
        return chromaSeq;
    }

    for (int start = 0; start + frameSize <= samples.size(); start += hopSize) {
        /* 计算当前帧的FFT幅度谱 */
        int fftSize = frameSize;
        QVector<double> magnitude(fftSize / 2, 0.0);

        for (int k = 0; k < fftSize / 2; ++k) {
            double real = 0.0, imag = 0.0;
            for (int n = 0; n < fftSize; ++n) {
                double angle = -2.0 * M_PI * k * n / fftSize;
                double sample = samples[start + n];
                /* 应用Hann窗 */
                double window = 0.5 * (1.0 - qCos(2.0 * M_PI * n / fftSize));
                real += sample * window * qCos(angle);
                imag += sample * window * qSin(angle);
            }
            magnitude[k] = qSqrt(real * real + imag * imag);
        }

        /* 计算色度向量 */
        QVector<double> chroma = compute(magnitude, sampleRate);
        chromaSeq.append(chroma);
    }

    m_stats.totalChordsDeted = chromaSeq.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesComputed);

    return chromaSeq;
}

/**
 * @brief 重置所有统计数据
 *
 * 将帧计算计数、和弦检测计数和计时归零。
 */
void Chromagram4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
