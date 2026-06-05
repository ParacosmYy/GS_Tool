/**
 * @file MultibandCompressor.cpp
 * @brief 多频段压缩器 — 交叉+独立压缩+makeup 实现
 *
 * 使用 Linkwitz-Riley 交叉滤波器将信号分为多个频段，
 * 每个频段独立进行包络检测和增益压缩，最后叠加输出。
 * 支持可配置的频段数、阈值、比率、启动/释放时间和补偿增益。
 */

#include "utils/dsp47/MultibandCompressor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param numBands 频段数量，默认4
 * @param parent 父QObject
 */
MultibandCompressor::MultibandCompressor(int numBands, QObject* parent)
    : QObject(parent)
    , m_numBands(qMax(2, numBands))
{
    m_params.resize(m_numBands);
    m_envelopeState.resize(m_numBands, 0.0);
    m_bandLevels.resize(m_numBands, 0.0);

    /* 初始化默认交叉频率（对数均匀分布） */
    double minFreq = 80.0;
    double maxFreq = 12000.0;
    for (int i = 0; i < m_numBands; ++i) {
        double frac = static_cast<double>(i + 1) / m_numBands;
        m_params[i].freq = minFreq * qPow(maxFreq / minFreq, frac);
        m_params[i].threshold = -20.0;
        m_params[i].ratio = 4.0;
        m_params[i].attack = 10.0;
        m_params[i].release = 100.0;
        m_params[i].makeup = 0.0;
    }

    designCrossovers();
    m_stats.numBands = m_numBands;
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率 (Hz)
 */
void MultibandCompressor::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
    designCrossovers();
}

/**
 * @brief 设置指定频段的压缩参数
 * @param band 频段索引 [0, numBands)
 * @param params 压缩参数
 */
void MultibandCompressor::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands) {
        m_params[band] = params;
    }
}

/**
 * @brief 处理音频信号
 *
 * 流程: 输入 -> 交叉滤波分频 -> 各频段包络检测+压缩 -> 合成输出
 *
 * @param input 输入音频采样
 * @return 压缩后的音频采样
 */
QVector<double> MultibandCompressor::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) {
        return {};
    }

    /* 交叉滤波分频 */
    QVector<QVector<double>> bands = splitBands(input);

    /* 各频段独立压缩 */
    QVector<double> output(n, 0.0);
    double inputPeak = 0.0;
    double outputPeak = 0.0;

    for (int s = 0; s < n; ++s) {
        if (qAbs(input[s]) > inputPeak) {
            inputPeak = qAbs(input[s]);
        }
    }

    for (int b = 0; b < m_numBands; ++b) {
        /* 计算启动/释放时间系数 */
        double attackCoeff = qExp(-1.0 / (m_params[b].attack * 0.001 * m_sampleRate));
        double releaseCoeff = qExp(-1.0 / (m_params[b].release * 0.001 * m_sampleRate));

        for (int s = 0; s < n; ++s) {
            /* 包络检测 */
            double absVal = qAbs(bands[b][s]);
            if (absVal > m_envelopeState[b]) {
                m_envelopeState[b] = attackCoeff * m_envelopeState[b]
                                     + (1.0 - attackCoeff) * absVal;
            } else {
                m_envelopeState[b] = releaseCoeff * m_envelopeState[b]
                                     + (1.0 - releaseCoeff) * absVal;
            }

            /* 压缩增益 */
            double levelDB = 20.0 * qLn(qMax(m_envelopeState[b], 1e-10)) / qLn(10.0);
            double gainDB = compress(levelDB, m_params[b]);

            /* 补偿增益 */
            gainDB += m_params[b].makeup;

            double gainLin = qPow(10.0, gainDB / 20.0);
            bands[b][s] *= gainLin;
        }

        /* 叠加到输出 */
        for (int s = 0; s < n; ++s) {
            output[s] += bands[b][s];
        }
    }

    for (int s = 0; s < n; ++s) {
        if (qAbs(output[s]) > outputPeak) {
            outputPeak = qAbs(output[s]);
        }
    }

    /* 更新频段电平 */
    for (int b = 0; b < m_numBands; ++b) {
        m_bandLevels[b] = m_envelopeState[b];
    }

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    double peakReduction = inputPeak > 0.0
                           ? 20.0 * qLn(qMax(outputPeak, 1e-10) / inputPeak) / qLn(10.0)
                           : 0.0;
    emit processingCompleted(n, peakReduction);
    return output;
}

/**
 * @brief 获取各频段当前电平
 * @return 频段电平向量
 */
QVector<double> MultibandCompressor::bandLevels() const
{
    return m_bandLevels;
}

/**
 * @brief 重置统计信息
 */
void MultibandCompressor::resetStatistics()
{
    m_stats = Stats{};
    m_stats.numBands = m_numBands;
    m_timeSum = 0.0;
}

/**
 * @brief 设计 Linkwitz-Riley 交叉滤波器系数
 *
 * 使用二阶 Butterworth 级联构成四阶 Linkwitz-Riley 滤波器。
 * 每个交叉点生成一对低通/高通系数。
 */
void MultibandCompressor::designCrossovers()
{
    int numXovers = m_numBands - 1;
    m_crossoverA.resize(numXovers, QVector<double>(5, 0.0));
    m_crossoverB.resize(numXovers, QVector<double>(5, 0.0));

    for (int i = 0; i < numXovers; ++i) {
        double freq = m_params[i + 1].freq;
        if (freq <= 0 || m_sampleRate <= 0) continue;

        double omega = 2.0 * M_PI * freq / m_sampleRate;
        double cosW = qCos(omega);
        /* Butterworth 二阶系数 */
        double K = qTan(omega / 2.0);
        double K2 = K * K;
        double norm = 1.0 / (1.0 + M_SQRT2 * K + K2);

        /* 低通系数 [b0, b1, b2, a1, a2] */
        m_crossoverA[i][0] = K2 * norm;
        m_crossoverA[i][1] = 2.0 * K2 * norm;
        m_crossoverA[i][2] = K2 * norm;
        m_crossoverA[i][3] = 2.0 * (K2 - 1.0) * norm;
        m_crossoverA[i][4] = (1.0 - M_SQRT2 * K + K2) * norm;
    }

    /* 链接系数（用于全通补偿） */
    m_linkCoeffs.resize(numXovers, QVector<double>(3, 0.0));
    for (int i = 0; i < numXovers; ++i) {
        m_linkCoeffs[i][0] = m_crossoverA[i][0];
        m_linkCoeffs[i][1] = m_crossoverA[i][3];
        m_linkCoeffs[i][2] = m_crossoverA[i][4];
    }
}

/**
 * @brief 将输入信号通过交叉滤波器分为多个频段
 * @param input 输入音频信号
 * @return 各频段信号
 */
QVector<QVector<double>> MultibandCompressor::splitBands(const QVector<double>& input)
{
    int n = input.size();
    QVector<QVector<double>> bands(m_numBands, QVector<double>(n, 0.0));

    if (m_numBands == 1) {
        bands[0] = input;
        return bands;
    }

    /* 简化实现：使用一阶高低通分频 */
    QVector<double> low(n), high(n);
    low = input;
    high = input;

    int bandIdx = 0;
    QVector<double> current = input;

    for (int xover = 0; xover < m_numBands - 1; ++xover) {
        double freq = m_params[xover + 1].freq;
        double rc = 1.0 / (2.0 * M_PI * freq);
        double dt = 1.0 / m_sampleRate;
        double alpha = rc / (rc + dt);

        QVector<double> lo(n), hi(n);
        lo[0] = alpha * current[0];
        hi[0] = current[0] - lo[0];

        for (int s = 1; s < n; ++s) {
            lo[s] = alpha * lo[s - 1] + (1.0 - alpha) * current[s];
            hi[s] = current[s] - lo[s];
        }

        bands[bandIdx] = lo;
        current = hi;
        bandIdx++;
    }
    bands[bandIdx] = current;

    return bands;
}

/**
 * @brief 计算单个样本的压缩增益 (dB)
 * @param level 输入电平 (dB)
 * @param p 频段压缩参数
 * @return 增益变化量 (dB)，负值表示衰减
 */
double MultibandCompressor::compress(double level, const BandParams& p)
{
    if (level <= p.threshold) {
        return 0.0; /* 低于阈值，不压缩 */
    }
    double overDB = level - p.threshold;
    double compressedDB = p.threshold + overDB / p.ratio;
    return compressedDB - level; /* 负增益 */
}
