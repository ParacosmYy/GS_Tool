/**
 * @file HarmonicProduct3.cpp
 * @brief 谐波积谱3 — 多基频+泛音跟踪 实现
 *
 * 实现谐波积谱（HPS）算法用于基频检测。
 * 将频谱按 1~N 倍下采样后逐点相乘，峰值对应基频。
 * 支持多基频检测和泛音频率序列提取。
 */

#include "signal50/HarmonicProduct3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
HarmonicProduct3::HarmonicProduct3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率（Hz）
 */
void HarmonicProduct3::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置 FFT 大小
 * @param fftSize FFT 大小
 */
void HarmonicProduct3::setFFTSize(int fftSize)
{
    m_fftSize = qMax(4, fftSize);
}

/**
 * @brief 设置谐波数量
 * @param n 谐波数，>= 2
 */
void HarmonicProduct3::setNumHarmonics(int n)
{
    m_numHarmonics = qMax(2, n);
    m_stats.numHarmonics = m_numHarmonics;
}

/**
 * @brief 计算谐波积谱
 *
 * 将输入频谱按 2, 3, ..., N 倍下采样，然后
 * 与原频谱逐点相乘得到 HPS。
 * HPS 的峰值位置对应基频的 FFT bin。
 *
 * @param spectrum 输入幅度谱（FFT 结果）
 * @return 谐波积谱向量
 */
QVector<double> HarmonicProduct3::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int halfN = spectrum.size() / 2;
    if (halfN < m_numHarmonics) {
        return {};
    }

    /* 计算最大可用 bin 数（考虑下采样） */
    int maxBin = halfN;
    for (int h = 2; h <= m_numHarmonics; ++h) {
        maxBin = qMin(maxBin, halfN / h);
    }

    /* 初始化 HPS 为原频谱的拷贝 */
    QVector<double> hps(maxBin);
    for (int i = 0; i < maxBin; ++i) {
        hps[i] = qMax(spectrum[i], 1e-10);
    }

    /* 逐谐波下采样相乘 */
    for (int h = 2; h <= m_numHarmonics; ++h) {
        QVector<double> ds = downsample(spectrum, h);
        for (int i = 0; i < maxBin && i < ds.size(); ++i) {
            hps[i] *= qMax(ds[i], 1e-10);
        }
    }

    /* 寻找基频：HPS 最大值对应的 bin */
    int peakBin = 0;
    double peakVal = hps[0];
    /* 从 bin=1 开始搜索，避免直流分量 */
    for (int i = 1; i < maxBin; ++i) {
        if (hps[i] > peakVal) {
            peakVal = hps[i];
            peakBin = i;
        }
    }

    /* 精细化峰值位置（抛物线插值） */
    double refinedBin = refinePeak(hps, peakBin);
    m_fundamental = refinedBin * m_sampleRate / m_fftSize;

    /* 计算泛音频率序列 */
    m_harmonics.clear();
    for (int h = 1; h <= m_numHarmonics; ++h) {
        double freq = m_fundamental * h;
        if (freq < m_sampleRate / 2.0) {
            m_harmonics.append(freq);
        }
    }

    /* 统计更新 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit pitchDetected(m_fundamental, peakVal);
    return hps;
}

/**
 * @brief 对频谱进行整数倍下采样
 *
 * 每隔 factor 个采样取一个值，模拟谐波频率对应的位置。
 *
 * @param spec 原始频谱
 * @param factor 下采样因子
 * @return 下采样后的频谱
 */
QVector<double> HarmonicProduct3::downsample(const QVector<double>& spec, int factor) const
{
    int outSize = spec.size() / factor;
    QVector<double> result(outSize);
    for (int i = 0; i < outSize; ++i) {
        result[i] = spec[i * factor];
    }
    return result;
}

/**
 * @brief 使用抛物线插值精细化峰值位置
 *
 * 在离散峰值 bin 附近使用三点抛物线插值，
 * 获得亚 bin 精度的频率估计。
 *
 * @param spec 频谱数据
 * @param bin 粗峰值 bin 索引
 * @return 精细化后的 bin 位置（浮点数）
 */
double HarmonicProduct3::refinePeak(const QVector<double>& spec, int bin) const
{
    if (bin <= 0 || bin >= spec.size() - 1) return static_cast<double>(bin);

    double alpha = qMax(spec[bin - 1], 1e-10);
    double beta = qMax(spec[bin], 1e-10);
    double gamma = qMax(spec[bin + 1], 1e-10);

    /* 抛物线插值: p = 0.5 * (alpha - gamma) / (alpha - 2*beta + gamma) */
    double denom = alpha - 2.0 * beta + gamma;
    if (qFuzzyIsNull(denom)) return static_cast<double>(bin);

    double p = 0.5 * (alpha - gamma) / denom;
    p = qBound(-0.5, p, 0.5); /* 限制偏移范围 */

    return bin + p;
}

/**
 * @brief 多基频检测
 *
 * 在频谱中检测多个基频候选。首先找到前 N 个 HPS 峰值，
 * 然后通过抑制已检测峰值邻域来发现次级基频。
 *
 * @param spectrum 输入幅度谱
 * @return 检测到的基频列表，每项为 (频率, 置信度)
 */
QVector<QPair<double, double>> HarmonicProduct3::detectMultiPitch(const QVector<double>& spectrum) const
{
    QElapsedTimer timer;
    timer.start();

    const int halfN = spectrum.size() / 2;
    if (halfN < m_numHarmonics) return {};

    /* 计算 HPS */
    int maxBin = halfN;
    for (int h = 2; h <= m_numHarmonics; ++h) {
        maxBin = qMin(maxBin, halfN / h);
    }

    QVector<double> hps(maxBin);
    for (int i = 0; i < maxBin; ++i) {
        hps[i] = qMax(spectrum[i], 1e-10);
    }
    for (int h = 2; h <= m_numHarmonics; ++h) {
        QVector<double> ds = downsample(spectrum, h);
        for (int i = 0; i < maxBin && i < ds.size(); ++i) {
            hps[i] *= qMax(ds[i], 1e-10);
        }
    }

    /* 寻找多个峰值 */
    QVector<double> workingHps = hps;
    QVector<QPair<double, double>> pitches;
    int maxPitches = 5;

    /* 全局最大值作为归一化因子 */
    double globalMax = *std::max_element(hps.begin(), hps.end());
    if (globalMax < 1e-10) globalMax = 1.0;

    for (int p = 0; p < maxPitches; ++p) {
        /* 找当前最大峰值 */
        int peakBin = 1;
        double peakVal = workingHps[1];
        for (int i = 2; i < maxBin; ++i) {
            if (workingHps[i] > peakVal) {
                peakVal = workingHps[i];
                peakBin = i;
            }
        }

        /* 置信度阈值 */
        double confidence = peakVal / globalMax;
        if (confidence < 0.05 || peakBin < 1) break;

        /* 抛物线插值精细化 */
        double refinedBin = refinePeak(workingHps, peakBin);
        double freq = refinedBin * m_sampleRate / m_fftSize;

        /* 避免重复检测（频率间隔过小） */
        bool duplicate = false;
        for (const auto& existing : pitches) {
            if (qAbs(existing.first - freq) < 10.0) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            pitches.append(qMakePair(freq, confidence));
        }

        /* 抑制已检测峰值邻域 */
        int suppressRadius = qMax(5, static_cast<int>(peakBin * 0.1));
        for (int i = qMax(0, peakBin - suppressRadius);
             i <= qMin(maxBin - 1, peakBin + suppressRadius); ++i) {
            workingHps[i] = 0.0;
        }
    }

    /* 按置信度降序排列 */
    std::sort(pitches.begin(), pitches.end(),
              [](const QPair<double, double>& a, const QPair<double, double>& b) {
                  return a.second > b.second;
              });

    return pitches;
}

/**
 * @brief 重置所有统计数据
 */
void HarmonicProduct3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.numHarmonics = m_numHarmonics;
    m_timeSum = 0.0;
}
