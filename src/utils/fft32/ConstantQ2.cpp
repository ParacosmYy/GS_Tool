/**
 * @file ConstantQ2.cpp
 * @brief 常数Q变换增强实现 — 对数频率/多分辨率/逆CQT
 */

#include "utils/fft32/ConstantQ2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ConstantQ2::ConstantQ2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void ConstantQ2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置频率范围 @param minFreq 最低频率 @param maxFreq 最高频率 */
void ConstantQ2::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = qMax(1.0, minFreq);
    m_maxFreq = qMax(m_minFreq, maxFreq);
}

/** @brief 设置每倍频程的频率分辨率 @param bins 每倍频程bin数 */
void ConstantQ2::setBinsPerOctave(int bins)
{
    m_binsPerOctave = qMax(1, bins);
}

/** @brief 设置阈值(用于稀疏化) @param thresh 阈值 */
void ConstantQ2::setThreshold(double thresh)
{
    m_thresh = qBound(0.0, thresh, 1.0);
}

/** @brief 正向常数Q变换 @param input 时域信号 @return CQT系数 */
QVector<double> ConstantQ2::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return {};

    /* 计算总bin数 */
    double ratio = m_maxFreq / m_minFreq;
    double octaves = qLn(ratio) / qLn(2.0);
    m_totalBins = qCeil(octaves * m_binsPerOctave);

    /* Q值: 决定每个频段的品质因数 */
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);

    QVector<double> cqtCoeffs(m_totalBins, 0.0);

    /* 对每个频率bin计算CQT系数 */
    for (int k = 0; k < m_totalBins; ++k) {
        /* 第k个bin的中心频率 */
        double freq = m_minFreq * qPow(2.0,
            static_cast<double>(k) / m_binsPerOctave);

        /* 对应的窗口长度(采样点数) */
        int Nk = qCeil(Q * m_sampleRate / freq);
        Nk = qMin(Nk, input.size());
        if (Nk <= 0) continue;

        /* 计算该频段的CQT: x[n] * exp(-j*2*pi*freq*n/sr) */
        double realPart = 0.0;
        double imagPart = 0.0;
        double windowSum = 0.0;

        for (int n = 0; n < Nk && n < input.size(); ++n) {
            /* Hann窗 */
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (Nk - 1)));
            double phase = 2.0 * M_PI * freq * n / m_sampleRate;

            realPart += input[n] * w * qCos(phase);
            imagPart += input[n] * w * qSin(phase);
            windowSum += w;
        }

        /* 归一化 */
        if (windowSum > 1e-10) {
            cqtCoeffs[k] = qSqrt(realPart * realPart + imagPart * imagPart)
                / windowSum;
        }

        /* 阈值稀疏化 */
        if (cqtCoeffs[k] < m_thresh) cqtCoeffs[k] = 0.0;
    }

    m_stats.totalTransforms++;
    m_stats.totalBinsComputed += m_totalBins;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalTransforms));

    emit transformComplete(m_totalBins);
    return cqtCoeffs;
}

/** @brief 逆常数Q变换 @param cqtCoeffs CQT系数 @return 时域信号 */
QVector<double> ConstantQ2::inverse(const QVector<double>& cqtCoeffs)
{
    QElapsedTimer timer;
    timer.start();

    int totalBins = cqtCoeffs.size();
    if (totalBins == 0) return {};

    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);

    /* 计算输出长度: 基于最低频率的窗口长度 */
    int outputLen = qCeil(Q * m_sampleRate / m_minFreq);
    outputLen = qMin(outputLen, 65536);

    QVector<double> output(outputLen, 0.0);
    QVector<double> windowSum(outputLen, 0.0);

    /* 叠加每个频率bin的贡献 */
    for (int k = 0; k < totalBins; ++k) {
        double freq = m_minFreq * qPow(2.0,
            static_cast<double>(k) / m_binsPerOctave);
        int Nk = qCeil(Q * m_sampleRate / freq);
        Nk = qMin(Nk, outputLen);
        if (Nk <= 0) continue;

        double amp = cqtCoeffs[k];
        if (amp < m_thresh) continue;

        for (int n = 0; n < Nk; ++n) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (Nk - 1)));
            output[n] += amp * w * qCos(2.0 * M_PI * freq * n / m_sampleRate);
            windowSum[n] += w;
        }
    }

    /* 归一化 */
    for (int n = 0; n < outputLen; ++n) {
        if (windowSum[n] > 1e-10) {
            output[n] /= windowSum[n];
        }
    }

    m_timeSum += timer.elapsed();
    return output;
}

/** @brief 获取各bin的中心频率 @return 频率列表(Hz) */
QVector<double> ConstantQ2::frequencies() const
{
    double octaves = qLn(m_maxFreq / m_minFreq) / qLn(2.0);
    int bins = qCeil(octaves * m_binsPerOctave);

    QVector<double> freqs(bins);
    for (int k = 0; k < bins; ++k) {
        freqs[k] = m_minFreq * qPow(2.0,
            static_cast<double>(k) / m_binsPerOctave);
    }
    return freqs;
}

/** @brief 获取总bin数 @return bin数 */
int ConstantQ2::totalBins() const
{
    double octaves = qLn(m_maxFreq / m_minFreq) / qLn(2.0);
    return qCeil(octaves * m_binsPerOctave);
}

/**
 * @brief 获取指定bin的窗口长度
 * @param k bin索引
 * @return 窗口长度(采样点数)
 *
 * CQT的核心特性: 低频bin有较长窗口(高频率分辨率)，
 * 高频bin有较短窗口(高时间分辨率)。
 */
int ConstantQ2::windowLength(int k) const
{
    if (k < 0) return 0;
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    double freq = m_minFreq * qPow(2.0,
        static_cast<double>(k) / m_binsPerOctave);
    return qCeil(Q * m_sampleRate / freq);
}

/**
 * @brief 获取指定bin的频率分辨率(Hz)
 * @param k bin索引
 * @return 频率分辨率
 *
 * 频率分辨率 = bin中心频率 / Q。
 * 低频段分辨率高，高频段分辨率低，但每个倍频程内bin数恒定。
 */
double ConstantQ2::frequencyResolution(int k) const
{
    if (k < 0) return 0.0;
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
    double freq = m_minFreq * qPow(2.0,
        static_cast<double>(k) / m_binsPerOctave);
    return freq / Q;
}

/**
 * @brief 计算CQT系数的色度特征(Chroma特征)
 * @param cqtCoeffs CQT系数
 * @return 12维色度向量(对应12个半音)
 *
 * 将CQT系数按音高类(pitch class)折叠为12维向量，
 * 常用于和弦识别和音乐分析。
 */
QVector<double> ConstantQ2::chromaFeatures(const QVector<double>& cqtCoeffs) const
{
    QVector<double> chroma(12, 0.0);
    if (cqtCoeffs.isEmpty()) return chroma;

    for (int k = 0; k < cqtCoeffs.size(); ++k) {
        /* 将bin索引映射到色度(0~11) */
        int chromaIdx = k % 12;
        chroma[chromaIdx] += cqtCoeffs[k] * cqtCoeffs[k];
    }

    /* 归一化 */
    double maxVal = *std::max_element(chroma.begin(), chroma.end());
    if (maxVal > 1e-10) {
        for (int i = 0; i < 12; ++i) {
            chroma[i] /= maxVal;
        }
    }
    return chroma;
}

/** @brief 重置统计 */
void ConstantQ2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
