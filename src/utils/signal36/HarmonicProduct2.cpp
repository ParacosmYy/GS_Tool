/**
 * @file HarmonicProduct2.cpp
 * @brief 谐波乘积谱增强实现 — 基频检测/多谐波/置信度/频域压缩
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal36/HarmonicProduct2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
HarmonicProduct2::HarmonicProduct2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("HarmonicProduct2"));
}

/**
 * @brief 设置使用的谐波数量
 *
 * 更多谐波数可以更准确地检测基频，但计算量增加。
 * 典型值3~8。
 *
 * @param n 谐波数量（最小为2）
 */
void HarmonicProduct2::setHarmonics(int n)
{
    m_harmonics = qMax(2, n);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void HarmonicProduct2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置FFT大小
 * @param size FFT大小
 */
void HarmonicProduct2::setFFTSize(int size)
{
    m_fftSize = qMax(64, size);
}

/**
 * @brief 估计基频
 *
 * 谐波乘积谱(HPS)方法:
 * 1. 取幅度谱的正频率部分
 * 2. 对每个谐波h，将频谱下采样h倍
 * 3. 将所有下采样频谱逐点相乘得到HPS
 * 4. HPS最大值对应的频率即为基频
 *
 * 原理: 如果信号有基频f0，则在f0, 2*f0, 3*f0, ... 处都有能量。
 * 下采样后的乘积在f0处产生峰值。
 *
 * @param spectrum FFT幅度谱（长度 >= fftSize/2+1）
 * @return 估计的基频(Hz)，检测失败返回0
 */
double HarmonicProduct2::estimate(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    if (spectrum.isEmpty()) {
        m_confidence = 0.0;
        return 0.0;
    }

    int halfN = qMin(spectrum.size(), m_fftSize / 2 + 1);
    int minBin = qMax(2, static_cast<int>(m_sampleRate / 20000.0)); /* 最低频率限制 */

    /* 构建HPS: 逐谐波下采样并相乘 */
    QVector<double> hps(halfN / m_harmonics, 1.0);

    for (int k = 0; k < static_cast<int>(hps.size()); ++k) {
        double product = 1.0;
        for (int h = 1; h <= m_harmonics; ++h) {
            int idx = k * h;
            if (idx < halfN) {
                product *= spectrum[idx];
            } else {
                product = 0.0;
                break;
            }
        }
        hps[k] = product;
    }

    /* 寻找HPS最大值 */
    int bestBin = minBin;
    double bestVal = 0.0;
    for (int k = minBin; k < static_cast<int>(hps.size()); ++k) {
        if (hps[k] > bestVal) {
            bestVal = hps[k];
            bestBin = k;
        }
    }

    /* 计算置信度: 峰值与次大值的比值 */
    double secondVal = 0.0;
    for (int k = minBin; k < static_cast<int>(hps.size()); ++k) {
        if (k != bestBin && hps[k] > secondVal) {
            secondVal = hps[k];
        }
    }

    m_confidence = 0.0;
    if (bestVal > 1e-10) {
        double ratio = (secondVal > 1e-10) ? bestVal / secondVal : 10.0;
        m_confidence = qMin(1.0, qLn(ratio) / qLn(10.0));
    }

    /* 抛物线插值精确化峰值位置 */
    double refinedBin = bestBin;
    if (bestBin > 0 && bestBin < static_cast<int>(hps.size()) - 1) {
        double y0 = hps[bestBin - 1];
        double y1 = hps[bestBin];
        double y2 = hps[bestBin + 1];
        double denom = y0 - 2.0 * y1 + y2;
        if (qFabs(denom) > 1e-20) {
            double delta = (y0 - y2) / (2.0 * denom);
            refinedBin = bestBin + qBound(-0.5, delta, 0.5);
        }
    }

    double frequency = refinedBin * m_sampleRate / static_cast<double>(m_fftSize);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimates++;
    m_stats.totalFramesProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit pitchEstimated(frequency, m_confidence);
    return frequency;
}

/**
 * @brief 多基频估计
 *
 * 在HPS中找到多个峰值，按强度排序返回多个候选基频。
 * 适用于多音信号(pitch mixture)。
 *
 * @param spectrum FFT幅度谱
 * @param maxF0 最多返回的基频数量
 * @return (频率, 置信度) 列表，按置信度降序排列
 */
QVector<QPair<double,double>> HarmonicProduct2::estimateMulti(const QVector<double>& spectrum, int maxF0)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double,double>> results;

    if (spectrum.isEmpty() || maxF0 <= 0) {
        return results;
    }

    int halfN = qMin(spectrum.size(), m_fftSize / 2 + 1);
    int minBin = qMax(2, static_cast<int>(m_sampleRate / 20000.0));

    /* 构建HPS */
    int hpsLen = halfN / m_harmonics;
    QVector<double> hps(hpsLen, 1.0);

    for (int k = 0; k < hpsLen; ++k) {
        double product = 1.0;
        for (int h = 1; h <= m_harmonics; ++h) {
            int idx = k * h;
            if (idx < halfN) {
                product *= spectrum[idx];
            } else {
                product = 0.0;
                break;
            }
        }
        hps[k] = product;
    }

    /* 寻找所有局部峰值 */
    struct Peak { int bin; double val; };
    QVector<Peak> peaks;

    for (int k = minBin; k < hpsLen - 1; ++k) {
        if (hps[k] > hps[k - 1] && hps[k] >= hps[k + 1] && hps[k] > 1e-10) {
            peaks.append({k, hps[k]});
        }
    }

    /* 按值降序排序 */
    std::sort(peaks.begin(), peaks.end(),
              [](const Peak& a, const Peak& b) { return a.val > b.val; });

    /* 计算全局最大值用于归一化置信度 */
    double maxVal = peaks.isEmpty() ? 0.0 : peaks[0].val;

    /* 提取前maxF0个峰值 */
    int count = qMin(maxF0, peaks.size());
    for (int i = 0; i < count; ++i) {
        int bin = peaks[i].bin;

        /* 抛物线插值 */
        double refinedBin = bin;
        if (bin > 0 && bin < hpsLen - 1) {
            double y0 = hps[bin - 1];
            double y1 = hps[bin];
            double y2 = hps[bin + 1];
            double denom = y0 - 2.0 * y1 + y2;
            if (qFabs(denom) > 1e-20) {
                double delta = (y0 - y2) / (2.0 * denom);
                refinedBin = bin + qBound(-0.5, delta, 0.5);
            }
        }

        double freq = refinedBin * m_sampleRate / m_fftSize;
        double conf = (maxVal > 1e-10) ? peaks[i].val / maxVal : 0.0;

        results.append({freq, conf});
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimates++;
    m_stats.totalFramesProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    return results;
}

/**
 * @brief 获取最近一次估计的置信度
 * @return 置信度 [0, 1]
 */
double HarmonicProduct2::confidence() const
{
    return m_confidence;
}

/**
 * @brief 重置所有累积统计信息
 */
void HarmonicProduct2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
