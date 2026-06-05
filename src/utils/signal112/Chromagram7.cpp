#include "Chromagram7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化色度图分析器
 * @param parent 父对象指针
 */
Chromagram7::Chromagram7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置参考频率A4(Hz)
 * @param freqHz 参考频率，默认440Hz
 */
void Chromagram7::setReferenceFrequency(double freqHz)
{
    m_refFreq = qMax(1.0, freqHz);
}

/**
 * @brief 设置帧参数
 * @param frameSize 帧长(样本数)
 * @param hopSize 跳步(样本数)
 */
void Chromagram7::setFrameParams(int frameSize, int hopSize)
{
    m_frameSize = qMax(64, frameSize);
    m_hopSize = qMax(1, hopSize);
}

/**
 * @brief 计算色度特征序列
 *
 * 将FFT频谱映射到12个音级(C, C#, D, ..., B)：
 * 1. 对每帧信号施加Hanning窗并计算FFT
 * 2. 将每个FFT频率bin映射到最近的音级
 * 3. 累加各音级的能量得到12维色度向量
 * 4. 归一化每个色度向量
 *
 * @param audio 输入音频信号
 * @return 12维色度向量序列
 */
QVector<QVector<double>> Chromagram7::compute(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> chromaFrames;
    const int n = audio.size();
    if (n < m_frameSize) {
        emit analysisCompleted(0);
        return chromaFrames;
    }

    const int numFrames = (n - m_frameSize) / m_hopSize + 1;

    /* 预计算Hanning窗 */
    QVector<double> window(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_frameSize - 1)));
    }

    /* 12个音级的中心频率映射常数 */
    /* MIDI音符号69 = A4 = 440Hz, 每个半音比率2^(1/12) */
    const double semitoneRatio = qPow(2.0, 1.0 / 12.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 计算DFT幅度谱（仅正频率部分） */
        const int fftLen = m_frameSize / 2;
        QVector<double> magnitude(fftLen, 0.0);
        for (int k = 0; k < fftLen; ++k) {
            double re = 0.0, im = 0.0;
            for (int i = 0; i < m_frameSize; ++i) {
                double sample = audio[start + i] * window[i];
                double angle = 2.0 * M_PI * k * i / m_frameSize;
                re += sample * qCos(angle);
                im -= sample * qSin(angle);
            }
            magnitude[k] = qSqrt(re * re + im * im);
        }

        /* 映射到12个音级 */
        QVector<double> chroma(12, 0.0);
        for (int k = 1; k < fftLen; ++k) {
            double freq = static_cast<double>(k) * 44100.0 / m_frameSize;
            if (freq <= 0.0) continue;

            /* 计算该频率对应的半音偏移（相对于C0） */
            double semitones = 12.0 * qLn(freq / (m_refFreq / 32.0)) / qLn(2.0);
            int pitchClass = static_cast<int>(qRound(semitones)) % 12;
            if (pitchClass < 0) pitchClass += 12;

            chroma[pitchClass] += magnitude[k] * magnitude[k];
        }

        /* 归一化 */
        double maxVal = *std::max_element(chroma.begin(), chroma.end());
        if (maxVal > 1e-10) {
            for (int i = 0; i < 12; ++i) {
                chroma[i] /= maxVal;
            }
        }

        chromaFrames.append(chroma);
    }

    m_stats.totalAnalyzed += chromaFrames.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAnalyzed;

    emit analysisCompleted(chromaFrames.size());
    return chromaFrames;
}

/**
 * @brief 重置所有统计信息
 */
void Chromagram7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
