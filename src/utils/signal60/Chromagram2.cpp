/**
 * @file Chromagram2.cpp
 * @brief 色度图计算实现 — FFT色度特征 + 音符检测
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现色度图（Chromagram）计算，将频谱能量映射到 12 个色度（音级）上。
 * 每个色度对应一个八度内的 12 个半音（C, C#, D, ..., B），
 * 所有八度的同一音名能量被累加到同一个色度箱中。
 */

#include "utils/signal60/Chromagram2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认色度图参数
 * @param parent 父QObject对象
 */
Chromagram2::Chromagram2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("Chromagram2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void Chromagram2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置 FFT 大小
 * @param n FFT 窗口大小
 */
void Chromagram2::setFFTSize(int n)
{
    m_fftSize = qMax(64, n);
}

/**
 * @brief 设置参考频率（A4 音高）
 *
 * 默认 A4 = 440 Hz。色度图使用此频率作为
 * 音名到频率映射的参考基准。
 *
 * @param freq 参考频率（Hz）
 */
void Chromagram2::setReferenceFreq(double freq)
{
    m_refFreq = qMax(1.0, freq);
}

// ──────────────────────────────────────────────
// 核心计算接口
// ──────────────────────────────────────────────

/**
 * @brief 计算输入帧的色度特征
 *
 * 处理步骤：
 * 1. 对输入帧进行 FFT 计算幅度谱
 * 2. 对每个色度 bin（C, C#, ..., B）：
 *    a. 计算对应的频率范围
 *    b. 累加所有八度中该音名的频谱能量
 * 3. 归一化色度向量
 *
 * @param frame 输入时域帧
 * @return 12 维色度向量（C, C#, D, ..., B）
 */
QVector<double> Chromagram2::compute(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    const int fftN = qMin(n, m_fftSize);
    const int numBins = fftN / 2 + 1;

    // 步骤1：计算幅度谱
    QVector<double> magnitude(numBins, 0.0);
    for (int k = 0; k < numBins; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int i = 0; i < fftN; ++i) {
            double sample = (i < n) ? frame[i] : 0.0;
            // Hann 窗
            double win = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (fftN - 1)));
            double angle = -2.0 * M_PI * k * i / fftN;
            re += sample * win * qCos(angle);
            im += sample * win * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }

    // 步骤2：计算色度特征
    // 音符名称：C=0, C#=1, D=2, ..., B=11
    // 频率公式：f = refFreq * 2^((midi - 69) / 12)
    // 色度 = midi % 12
    QVector<double> chroma(12, 0.0);

    for (int k = 1; k < numBins; ++k) {
        double freq = static_cast<double>(k) * m_sampleRate / fftN;
        if (freq < 20.0 || freq > 20000.0) continue;

        // 计算对应的 MIDI 音符号
        double midiDouble = 12.0 * qLn(freq / m_refFreq) / qLn(2.0) + 69.0;
        int midiNote = static_cast<int>(qRound(midiDouble));
        int chromaIdx = midiNote % 12;
        if (chromaIdx < 0) chromaIdx += 12;

        // 计算该 bin 到最近音名的偏移（用于加权）
        double fracDiff = qAbs(midiDouble - midiNote);
        double weight = qMax(0.0, 1.0 - fracDiff * 2.0); // 线性衰减

        chroma[chromaIdx] += magnitude[k] * weight;
    }

    // 步骤3：归一化
    m_energy = 0.0;
    for (int i = 0; i < 12; ++i) {
        m_energy += chroma[i];
    }

    if (m_energy > 1e-10) {
        for (int i = 0; i < 12; ++i) {
            chroma[i] /= m_energy;
        }
    }

    // 找到主导音符
    double maxChroma = 0.0;
    m_domNote = 0;
    for (int i = 0; i < 12; ++i) {
        if (chroma[i] > maxChroma) {
            maxChroma = chroma[i];
            m_domNote = i;
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit chromaComputed(m_domNote, m_energy);
    return chroma;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含计算次数、总帧数和平均耗时的Stats结构
 */
Chromagram2::Stats Chromagram2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void Chromagram2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
