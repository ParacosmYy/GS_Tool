/**
 * @file Chromagram3.cpp
 * @brief 色度图(Chromagram)分析器实现
 *
 * 将频谱映射到12个音高类别(C, C#, D, ..., B):
 * 1. 对输入时域帧进行DFT计算幅度谱
 * 2. 构建音符-FFT bin映射表(覆盖C1~B8共8个八度)
 * 3. 将各八度中同一音高类的能量累加得到12维色度向量
 * 4. 归一化色度向量用于和弦检测
 *
 * 和弦检测策略:
 * - 构建24个标准和弦模板(12大调+12小调)
 * - 使用余弦相似度匹配最佳和弦
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/signal79/Chromagram3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化色度图分析器
 * @param parent 父QObject指针
 */
Chromagram3::Chromagram3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率和参考频率(A4)
 *
 * 参考频率默认440Hz(标准调音A4)，改变后需重新计算
 * 音高类映射关系。
 *
 * @param sampleRate 采样率(Hz)
 * @param refFreq A4参考频率(Hz)，默认440.0
 */
void Chromagram3::initialize(double sampleRate, double refFreq)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_refFreq = qMax(1.0, refFreq);
}

/**
 * @brief 计算一帧的12维色度向量
 *
 * 处理流程:
 * 1. 对时域帧执行DFT计算幅度谱
 * 2. 遍历12个音高类(C=0, C#=1, ..., B=11)
 * 3. 对每个音高类，计算所有八度中该音高的能量总和
 * 4. 能量计算使用高斯加权窗口(中心频率附近权重高)
 * 5. 归一化到[0, 1]范围
 *
 * 音高类频率计算:
 * f(note, octave) = refFreq * 2^((octave*12 + note - 57) / 12)
 * 其中57对应A4(MIDI编号69)相对于C0的半音数
 *
 * @param frame 时域音频帧
 * @return 12维色度向量，索引0=C, 1=C#, ..., 11=B
 */
QVector<double> Chromagram3::computeChroma(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> chroma(12, 0.0);

    if (frame.isEmpty()) {
        m_stats.totalFramesAnalyzed++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalFramesAnalyzed > 0)
            ? m_timeSum / m_stats.totalFramesAnalyzed : 0.0;
        emit chromaComputed(chroma, "Unknown");
        return chroma;
    }

    const int N = frame.size();

    /* 步骤1: 计算DFT幅度谱(正频率部分) */
    const int specLen = N / 2 + 1;
    QVector<double> magnitude(specLen, 0.0);

    for (int k = 0; k < specLen; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }

    /* 步骤2: 计算每个音高类的能量 */
    const int numOctaves = 8;  ///< 覆盖C1~B8
    const double binWidth = m_sampleRate / N;  ///< FFT bin频率分辨率

    for (int note = 0; note < 12; ++note) {
        double energy = 0.0;

        for (int oct = 0; oct < numOctaves; ++oct) {
            /* 计算该音高类在此八度中的频率 */
            double freq = m_refFreq * qPow(2.0, (oct * 12 + note - 57) / 12.0);

            /* 频率超出有效范围则跳过 */
            if (freq < binWidth || freq > m_sampleRate / 2.0) {
                continue;
            }

            /* 计算对应的FFT bin中心 */
            double centerBin = freq / binWidth;
            int binLow = qMax(0, static_cast<int>(qFloor(centerBin - 1.5)));
            int binHigh = qMin(specLen - 1, static_cast<int>(qCeil(centerBin + 1.5)));

            /* 高斯加权累加附近bin的能量 */
            for (int b = binLow; b <= binHigh; ++b) {
                double dist = (b - centerBin);
                double weight = qExp(-0.5 * dist * dist);
                energy += magnitude[b] * weight;
            }
        }

        chroma[note] = energy;
    }

    /* 步骤3: 归一化到[0, 1] */
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-10) {
        for (double& v : chroma) {
            v /= maxVal;
        }
    }

    /* 检测和弦 */
    QString chord = detectChord(chroma);

    /* 更新统计 */
    m_stats.totalFramesAnalyzed++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalFramesAnalyzed > 0)
        ? m_timeSum / m_stats.totalFramesAnalyzed : 0.0;

    emit chromaComputed(chroma, chord);
    return chroma;
}

/**
 * @brief 识别当前帧的主和弦
 *
 * 使用余弦相似度将色度向量与24个标准三和弦模板
 * (12大调 + 12小调)进行匹配，返回最佳匹配。
 *
 * 大调模板: 根音(1.0) + 大三度(1.0) + 纯五度(1.0)
 * 小调模板: 根音(1.0) + 小三度(1.0) + 纯五度(1.0)
 *
 * @param chroma 12维色度向量
 * @return 和弦名称(如 "C", "Am", "F#m")
 */
QString Chromagram3::detectChord(const QVector<double>& chroma) const
{
    if (chroma.size() != 12) {
        return QStringLiteral("Unknown");
    }

    static const QStringList noteNames = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    /* 大调/小调三和弦的半音间隔 */
    static const int majorIntervals[3] = {0, 4, 7};   ///< 根音 大三度 纯五度
    static const int minorIntervals[3] = {0, 3, 7};   ///< 根音 小三度 纯五度

    double bestScore = -1.0;
    QString bestChord = "Unknown";

    for (int root = 0; root < 12; ++root) {
        /* 大调三和弦 */
        double majScore = 0.0;
        double majNorm1 = 0.0;
        double majNorm2 = 0.0;
        for (int i = 0; i < 3; ++i) {
            int idx = (root + majorIntervals[i]) % 12;
            majScore += chroma[idx] * 1.0;
            majNorm1 += chroma[idx] * chroma[idx];
            majNorm2 += 1.0;
        }
        /* 扩展: 考虑色度向量其他位置的衰减 */
        for (int i = 0; i < 12; ++i) {
            majNorm1 += chroma[i] * chroma[i];
        }
        double denom = qSqrt(majNorm1) * qSqrt(majNorm2);
        majScore = (denom > 1e-15) ? majScore / denom : 0.0;

        if (majScore > bestScore) {
            bestScore = majScore;
            bestChord = noteNames[root];
        }

        /* 小调三和弦 */
        double minScore = 0.0;
        double minNorm1 = 0.0;
        double minNorm2 = 0.0;
        for (int i = 0; i < 3; ++i) {
            int idx = (root + minorIntervals[i]) % 12;
            minScore += chroma[idx] * 1.0;
            minNorm1 += chroma[idx] * chroma[idx];
            minNorm2 += 1.0;
        }
        for (int i = 0; i < 12; ++i) {
            minNorm1 += chroma[i] * chroma[i];
        }
        denom = qSqrt(minNorm1) * qSqrt(minNorm2);
        minScore = (denom > 1e-15) ? minScore / denom : 0.0;

        if (minScore > bestScore) {
            bestScore = minScore;
            bestChord = noteNames[root] + "m";
        }
    }

    m_stats.totalChromaChanges++;
    return bestChord;
}

/**
 * @brief 获取色度向量中的主导音高类索引
 *
 * 返回色度向量中最大值对应的索引:
 * 0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F,
 * 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B
 *
 * @param chroma 12维色度向量
 * @return 主导音高类索引(0~11)，-1表示无效输入
 */
int Chromagram3::dominantPitchClass(const QVector<double>& chroma) const
{
    if (chroma.size() != 12) {
        return -1;
    }

    int maxIdx = 0;
    double maxVal = chroma[0];
    for (int i = 1; i < 12; ++i) {
        if (chroma[i] > maxVal) {
            maxVal = chroma[i];
            maxIdx = i;
        }
    }

    return maxIdx;
}

/**
 * @brief 重置所有统计数据
 *
 * 清零帧计数、色度变化计数和平均处理时间。
 */
void Chromagram3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
