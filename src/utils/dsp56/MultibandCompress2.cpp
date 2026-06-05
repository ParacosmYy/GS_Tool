/**
 * @file MultibandCompress2.cpp
 * @brief 多频段压缩器实现 — 频段分割 + 独立压缩 + 信号重建
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现多频段动态范围压缩器。
 * 通过交叉滤波器将输入信号分割为多个频段，
 * 对每个频段独立应用压缩，最后叠加重建输出信号。
 */

#include "utils/dsp56/MultibandCompress2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认4频段压缩器
 * @param parent 父QObject对象
 */
MultibandCompress2::MultibandCompress2(QObject* parent)
    : QObject(parent)
    , m_numBands(4)
{
    setObjectName(QStringLiteral("MultibandCompress2"));

    // 默认交叉频率：120Hz, 1000Hz, 4000Hz
    m_crossovers = {120.0, 1000.0, 4000.0};
    // 默认阈值和比例
    m_thresholds = {-20.0, -18.0, -16.0, -14.0};
    m_ratios = {3.0, 3.0, 3.0, 3.0};
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置频段数量
 *
 * 交叉频率数量 = 频段数 - 1。
 * 自动调整阈值和比例数组大小。
 *
 * @param n 频段数量，范围 [2, 8]
 */
void MultibandCompress2::setNumBands(int n)
{
    m_numBands = qBound(2, n, 8);
    m_crossovers.resize(m_numBands - 1);
    m_thresholds.resize(m_numBands);
    m_ratios.resize(m_numBands);

    // 填充默认值
    for (int i = 0; i < m_numBands - 1; ++i) {
        if (m_crossovers[i] <= 0.0) {
            m_crossovers[i] = 100.0 * qPow(10.0, i);
        }
    }
    for (int i = 0; i < m_numBands; ++i) {
        if (m_thresholds[i] == 0.0) m_thresholds[i] = -20.0 + i * 2.0;
        if (m_ratios[i] == 0.0) m_ratios[i] = 3.0;
    }
}

/**
 * @brief 设置交叉频率
 *
 * 定义相邻频段之间的分界频率。
 * 数组长度应为 numBands - 1。
 *
 * @param freqs 交叉频率数组（Hz）
 */
void MultibandCompress2::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossovers = freqs;
}

/**
 * @brief 设置指定频段的压缩阈值
 * @param band 频段索引（0-based）
 * @param thresh 阈值（dB）
 */
void MultibandCompress2::setBandThreshold(int band, double thresh)
{
    if (band >= 0 && band < m_thresholds.size()) {
        m_thresholds[band] = thresh;
    }
}

/**
 * @brief 设置指定频段的压缩比例
 * @param band 频段索引（0-based）
 * @param ratio 压缩比例（1.0 = 不压缩，>1.0 = 压缩）
 */
void MultibandCompress2::setBandRatio(int band, double ratio)
{
    if (band >= 0 && band < m_ratios.size()) {
        m_ratios[band] = qMax(1.0, ratio);
    }
}

// ──────────────────────────────────────────────
// 核心处理接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行多频段压缩
 *
 * 处理流程：
 * 1. 通过交叉滤波器分割为多个频段
 * 2. 对每个频段独立计算增益并应用压缩
 * 3. 叠加所有频段重建输出信号
 *
 * @param input 输入音频采样数据
 * @return 经过多频段压缩后的音频数据
 */
QVector<double> MultibandCompress2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return {};

    // 步骤1：频段分割
    QVector<QVector<double>> bands = splitBands(input);

    // 步骤2：独立压缩各频段
    double peakGain = 0.0;
    QVector<QVector<double>> compressed(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        double thresh = (b < m_thresholds.size()) ? m_thresholds[b] : -20.0;
        double ratio  = (b < m_ratios.size()) ? m_ratios[b] : 3.0;
        compressed[b] = compressBand(bands[b], thresh, ratio);

        // 检测峰值增益
        for (int i = 0; i < compressed[b].size(); ++i) {
            double gain = qAbs(compressed[b][i]);
            if (gain > peakGain) peakGain = gain;
        }
    }

    // 步骤3：叠加重建
    QVector<double> output(n, 0.0);
    for (int b = 0; b < m_numBands; ++b) {
        for (int i = 0; i < n; ++i) {
            output[i] += compressed[b][i];
        }
    }

    // 步骤4：输出限幅（防止削波）
    for (int i = 0; i < n; ++i) {
        output[i] = qBound(-1.0, output[i], 1.0);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, peakGain);
    return output;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含处理次数、总采样数和平均耗时的Stats结构
 */
MultibandCompress2::Stats MultibandCompress2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void MultibandCompress2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 频段分割
// ──────────────────────────────────────────────

/**
 * @brief 使用二阶 IIR 交叉滤波器分割频段
 *
 * 实现简化的 Linkwitz-Riley 交叉滤波器。
 * 低通输出 = 输入 - 高通输出（确保全通特性）。
 *
 * @param sig 输入信号
 * @return 各频段信号数组
 */
QVector<QVector<double>> MultibandCompress2::splitBands(const QVector<double>& sig)
{
    const int n = sig.size();
    QVector<QVector<double>> bands(m_numBands, QVector<double>(n, 0.0));

    if (m_crossovers.size() < m_numBands - 1 || n == 0) {
        // 不足交叉频率时，所有信号归入第一频段
        bands[0] = sig;
        return bands;
    }

    // 简化实现：使用一阶低通/高通滤波器级联
    QVector<double> remaining = sig;

    for (int b = 0; b < m_numBands - 1; ++b) {
        double fc = m_crossovers[b];
        // 归一化截止频率（假设 44100Hz 采样率）
        double omega = 2.0 * M_PI * fc / 44100.0;
        double alpha = qSin(omega) / (2.0 * 0.707);

        double a0 = 1.0 + alpha;
        double b0 = (1.0 - qCos(omega)) / 2.0;
        double b1 = 1.0 - qCos(omega);
        double b2 = (1.0 - qCos(omega)) / 2.0;
        double a1 = -2.0 * qCos(omega);
        double a2 = 1.0 - alpha;

        // 低通滤波器
        QVector<double> lowpass(n, 0.0);
        double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
        for (int i = 0; i < n; ++i) {
            double x0 = remaining[i];
            lowpass[i] = (b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
            x2 = x1; x1 = x0;
            y2 = y1; y1 = lowpass[i];
        }

        // 高通 = 原始 - 低通
        QVector<double> highpass(n, 0.0);
        for (int i = 0; i < n; ++i) {
            highpass[i] = remaining[i] - lowpass[i];
        }

        bands[b] = lowpass;
        remaining = highpass;
    }

    bands[m_numBands - 1] = remaining;
    return bands;
}

// ──────────────────────────────────────────────
// 私有方法 — 频段压缩
// ──────────────────────────────────────────────

/**
 * @brief 对单个频段应用压缩
 *
 * 压缩公式（dB域）：
 *   if level > threshold:
 *     gain = threshold + (level - threshold) / ratio - level
 *   else:
 *     gain = 0
 *
 * 使用包络跟随器平滑增益变化，避免失真。
 *
 * @param band 单频段信号
 * @param thresh 压缩阈值（dB）
 * @param ratio 压缩比例
 * @return 压缩后的频段信号
 */
QVector<double> MultibandCompress2::compressBand(const QVector<double>& band,
                                                   double thresh, double ratio)
{
    const int n = band.size();
    QVector<double> output(n, 0.0);

    const double attackCoeff  = 0.01;   // 攻击系数（快速）
    const double releaseCoeff = 0.001;  // 释放系数（慢速）
    double envelopeDb = -120.0;         // 当前包络电平（dB）

    for (int i = 0; i < n; ++i) {
        // 计算输入电平（dB）
        double absVal = qAbs(band[i]);
        double inputDb = (absVal > 1e-10) ? 20.0 * qLn(absVal) / qLn(10.0) : -120.0;

        // 包络跟随
        double coeff = (inputDb > envelopeDb) ? attackCoeff : releaseCoeff;
        envelopeDb = envelopeDb + coeff * (inputDb - envelopeDb);

        // 计算增益（dB）
        double gainDb = 0.0;
        if (envelopeDb > thresh) {
            double overDb = envelopeDb - thresh;
            double compressedDb = thresh + overDb / ratio;
            gainDb = compressedDb - envelopeDb;
        }

        // 转为线性增益并应用
        double gainLin = qPow(10.0, gainDb / 20.0);
        output[i] = band[i] * gainLin;
    }

    return output;
}
