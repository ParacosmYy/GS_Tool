/**
 * @file MultibandCompress3.cpp
 * @brief 多频段动态压缩器实现 — 交叉滤波分割 + 独立压缩 + 包络跟随 + 信号重建
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 实现多频段动态范围压缩器。
 * 通过二阶 Linkwitz-Riley 交叉滤波器将输入信号分割为 N 个频段，
 * 对每个频段独立进行包络检测和增益计算，最后叠加重建。
 * 支持独立的 attack/release 时间控制和峰值检测。
 */

#include "utils/dsp76/MultibandCompress3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 常量定义
// ──────────────────────────────────────────────

/** @brief 默认采样率(Hz) */
static constexpr double kSampleRate = 44100.0;

/** @brief 最小dB值，用于避免log(0) */
static constexpr double kMinDb = -120.0;

// ──────────────────────────────────────────────
// 内部频段参数结构
// ──────────────────────────────────────────────

/**
 * @brief 频段参数，存储每个频段的独立压缩设定
 */
struct BandParams {
    double thresholdDb = -20.0;   ///< 压缩阈值(dB)
    double ratio       = 3.0;     ///< 压缩比例(1:1=无压缩)
    double attackMs    = 10.0;    ///< 攻击时间(ms)
    double releaseMs   = 100.0;   ///< 释放时间(ms)
    double kneeDb      = 6.0;     ///< 软拐点宽度(dB)
    double envelopeDb  = kMinDb;  ///< 当前包络电平(dB)
    double gainReductionDb = 0.0; ///< 当前增益衰减(dB)
};

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认4频段压缩器
 * @param parent 父QObject对象
 *
 * 默认交叉频率: 120Hz, 1000Hz, 4000Hz
 * 默认压缩参数: threshold=-20dB, ratio=3:1, attack=10ms, release=100ms
 */
MultibandCompress3::MultibandCompress3(QObject* parent)
    : QObject(parent)
    , m_bandCount(4)
    , m_gainReductions(4, 0.0)
    , m_bandParams(std::make_shared<std::vector<BandParams>>(4))
{
    setObjectName(QStringLiteral("MultibandCompress3"));

    // 默认交叉频率: 120Hz / 1000Hz / 4000Hz (3个交叉点 -> 4个频段)
    m_crossoverFreqs = {120.0, 1000.0, 4000.0};

    // 各频段默认参数 (低频段更激进，高频段更温和)
    auto& params = *m_bandParams;
    params[0] = {-24.0, 4.0, 15.0, 150.0, 6.0, kMinDb, 0.0};
    params[1] = {-20.0, 3.0, 10.0, 100.0, 6.0, kMinDb, 0.0};
    params[2] = {-18.0, 3.0,  5.0,  80.0, 6.0, kMinDb, 0.0};
    params[3] = {-16.0, 2.0,  2.0,  50.0, 6.0, kMinDb, 0.0};
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置频段分界频率(Hz)
 *
 * 交叉频率数量决定频段数: bands = crossovers.size() + 1。
 * 频率必须按升序排列，范围 [20, 20000] Hz。
 *
 * @param frequencies 交叉频率数组(Hz)，长度范围 [1, 7]
 */
void MultibandCompress3::setCrossoverFreqs(const QVector<double>& frequencies)
{
    m_crossoverFreqs = frequencies;
    m_bandCount = frequencies.size() + 1;
    m_gainReductions.resize(m_bandCount, 0.0);

    // 调整频段参数数组大小
    auto& params = *m_bandParams;
    if (static_cast<int>(params.size()) < m_bandCount) {
        params.resize(m_bandCount);
    }
}

/**
 * @brief 设置指定频段的压缩参数
 *
 * 每个频段可独立设置阈值、比例、攻击和释放时间。
 * 软拐点自动根据阈值计算。
 *
 * @param band 频段索引(0-based)
 * @param thresholdDb 压缩阈值(dB)，范围 [-60, 0]
 * @param ratio 压缩比例，范围 [1.0, 20.0]
 * @param attackMs 攻击时间(ms)，范围 [0.1, 200.0]
 * @param releaseMs 释放时间(ms)，范围 [1.0, 2000.0]
 */
void MultibandCompress3::setBandParams(int band, double thresholdDb, double ratio,
                                        double attackMs, double releaseMs)
{
    if (band < 0 || band >= m_bandCount) return;

    auto& bp = (*m_bandParams)[band];
    bp.thresholdDb = qBound(-60.0, thresholdDb, 0.0);
    bp.ratio       = qBound(1.0, ratio, 20.0);
    bp.attackMs    = qBound(0.1, attackMs, 200.0);
    bp.releaseMs   = qBound(1.0, releaseMs, 2000.0);
}

// ──────────────────────────────────────────────
// 核心处理
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行多频段压缩
 *
 * 完整处理流程:
 * 1. 使用二阶 IIR 交叉滤波器分割为 N 个频段
 * 2. 对每个频段独立计算包络(带 attack/release)
 * 3. 根据软拐点曲线计算增益衰减
 * 4. 应用增益后叠加所有频段重建输出
 * 5. 输出软限幅防止削波
 *
 * @param input 输入音频采样数据
 * @return 经过4频段压缩后的音频数据
 */
QVector<double> MultibandCompress3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) {
        m_stats.totalFramesProcessed++;
        const double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;
        return {};
    }

    // ── 步骤1: 频段分割 ──
    QVector<QVector<double>> bands = splitIntoBands(input, n);

    // ── 步骤2: 独立压缩各频段 ──
    QVector<QVector<double>> compressed(m_bandCount);
    int bandActivations = 0;

    for (int b = 0; b < m_bandCount; ++b) {
        compressed[b] = compressSingleBand(b, bands[b], n);
        // 统计本帧有效激活的频段数
        if (m_gainReductions[b] > 0.5) {
            bandActivations++;
        }
    }

    // ── 步骤3: 叠加重建 ──
    QVector<double> output(n, 0.0);
    for (int b = 0; b < m_bandCount; ++b) {
        for (int i = 0; i < n; ++i) {
            output[i] += compressed[b][i];
        }
    }

    // ── 步骤4: 软限幅（防止多频段叠加导致削波） ──
    for (int i = 0; i < n; ++i) {
        output[i] = softClip(output[i], 1.0);
    }

    // ── 步骤5: 更新统计 ──
    m_stats.totalFramesProcessed++;
    m_stats.totalBandActivations += bandActivations;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    // ── 步骤6: 发射各频段信号 ──
    for (int b = 0; b < m_bandCount; ++b) {
        if (m_gainReductions[b] > 0.1) {
            emit bandCompressed(b, m_gainReductions[b]);
        }
    }

    return output;
}

/**
 * @brief 获取各频段当前增益 reduction
 * @return 各频段增益衰减(dB)，正值表示衰减量
 */
QVector<double> MultibandCompress3::bandGainReductions() const
{
    return m_gainReductions;
}

/**
 * @brief 重置所有累计统计信息
 */
void MultibandCompress3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    // 重置各频段包络状态
    for (auto& bp : *m_bandParams) {
        bp.envelopeDb = kMinDb;
        bp.gainReductionDb = 0.0;
    }
}

// ──────────────────────────────────────────────
// 私有方法 — 频段分割
// ──────────────────────────────────────────────

/**
 * @brief 使用二阶 IIR 交叉滤波器将信号分割为多个频段
 *
 * 实现 Linkwitz-Riley 风格的级联分割:
 * 每一级用二阶低通提取当前频段，残余信号传递到下一级。
 * 低通输出 = IIR 低通滤波，高通 = 输入 - 低通（全通互补）。
 *
 * @param input 输入信号
 * @param n 采样数
 * @return 各频段信号数组
 */
QVector<QVector<double>> MultibandCompress3::splitIntoBands(
    const QVector<double>& input, int n) const
{
    QVector<QVector<double>> bands(m_bandCount, QVector<double>(n, 0.0));

    if (m_crossoverFreqs.size() < m_bandCount - 1 || n == 0) {
        // 交叉频率不足时所有信号归入第一频段
        bands[0] = input;
        return bands;
    }

    QVector<double> remaining = input;

    for (int b = 0; b < m_bandCount - 1; ++b) {
        double fc = m_crossoverFreqs[b];
        double omega = 2.0 * M_PI * fc / kSampleRate;
        double alpha = qSin(omega) / (2.0 * 0.707);

        double a0 = 1.0 + alpha;
        double b0 = (1.0 - qCos(omega)) / 2.0;
        double b1 = 1.0 - qCos(omega);
        double b2 = (1.0 - qCos(omega)) / 2.0;
        double a1 = -2.0 * qCos(omega);
        double a2 = 1.0 - alpha;

        // 低通滤波器（直接 II 型转置）
        QVector<double> lowpass(n, 0.0);
        double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

        for (int i = 0; i < n; ++i) {
            double x0 = remaining[i];
            lowpass[i] = (b0 * x0 + b1 * x1 + b2 * x2
                          - a1 * y1 - a2 * y2) / a0;
            x2 = x1; x1 = x0;
            y2 = y1; y1 = lowpass[i];
        }

        // 高通 = 原始 - 低通（全通互补）
        QVector<double> highpass(n, 0.0);
        for (int i = 0; i < n; ++i) {
            highpass[i] = remaining[i] - lowpass[i];
        }

        bands[b] = lowpass;
        remaining = highpass;
    }

    bands[m_bandCount - 1] = remaining;
    return bands;
}

// ──────────────────────────────────────────────
// 私有方法 — 单频段压缩
// ──────────────────────────────────────────────

/**
 * @brief 对单个频段应用带软拐点的增益压缩
 *
 * 压缩曲线(带软拐点):
 * - level < threshold - knee/2: 无压缩
 * - threshold - knee/2 <= level <= threshold + knee/2: 平滑过渡
 * - level > threshold + knee/2: 标准比例压缩
 *
 * 包络跟随器使用一阶 IIR，attack/release 独立可调。
 *
 * @param bandIdx 频段索引
 * @param band 单频段信号
 * @param n 采样数
 * @return 压缩后的频段信号
 */
QVector<double> MultibandCompress3::compressSingleBand(
    int bandIdx, const QVector<double>& band, int n)
{
    QVector<double> output(n, 0.0);
    if (bandIdx < 0 || bandIdx >= static_cast<int>(m_bandParams->size())) return output;

    auto& bp = (*m_bandParams)[bandIdx];

    // 计算包络跟随系数
    double attackCoeff  = qExp(-1.0 / (bp.attackMs  * 0.001 * kSampleRate));
    double releaseCoeff = qExp(-1.0 / (bp.releaseMs * 0.001 * kSampleRate));

    double halfKnee = bp.kneeDb * 0.5;
    double maxReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        // 计算输入电平(dB)
        double absVal = qAbs(band[i]);
        double inputDb = (absVal > 1e-10)
            ? 20.0 * qLn(absVal) / qLn(10.0)
            : kMinDb;

        // 包络跟随（attack快，release慢）
        double coeff = (inputDb > bp.envelopeDb) ? attackCoeff : releaseCoeff;
        bp.envelopeDb += coeff * (inputDb - bp.envelopeDb);

        // 计算增益(dB) — 带软拐点的压缩曲线
        double gainDb = computeSoftKneeGain(bp.envelopeDb, bp.thresholdDb,
                                             bp.ratio, halfKnee);
        bp.gainReductionDb = -gainDb; // 正值表示衰减量

        if (bp.gainReductionDb > maxReduction) {
            maxReduction = bp.gainReductionDb;
        }

        // 转为线性增益并应用
        double gainLin = qPow(10.0, gainDb / 20.0);
        output[i] = band[i] * gainLin;
    }

    m_gainReductions[bandIdx] = maxReduction;
    return output;
}

// ──────────────────────────────────────────────
// 私有方法 — 软拐点增益计算
// ──────────────────────────────────────────────

/**
 * @brief 计算软拐点压缩曲线的增益(dB)
 *
 * 软拐点公式:
 * - 低于拐点区: gain = 0
 * - 在拐点区内: 二次插值平滑过渡
 * - 高于拐点区: gain = (level - threshold) * (1/ratio - 1)
 *
 * @param level 当前电平(dB)
 * @param threshold 压缩阈值(dB)
 * @param ratio 压缩比例
 * @param halfKnee 半拐点宽度(dB)
 * @return 增益偏移(dB)，负值表示衰减
 */
double MultibandCompress3::computeSoftKneeGain(double level, double threshold,
                                                double ratio, double halfKnee) const
{
    if (level < threshold - halfKnee) {
        // 低于拐点区：不压缩
        return 0.0;
    }

    if (level > threshold + halfKnee) {
        // 高于拐点区：标准压缩
        double overDb = level - threshold;
        return threshold + overDb / ratio - level;
    }

    // 在拐点区内：二次插值平滑过渡
    double x = level - threshold + halfKnee;
    double knee = 2.0 * halfKnee;
    double t = x / knee; // 归一化 [0, 1]
    // 过渡增益 = (1/ratio - 1) * t^2 * knee / 2
    double transitionGain = (1.0 / ratio - 1.0) * t * t * knee * 0.5;
    return transitionGain;
}

// ──────────────────────────────────────────────
// 私有方法 — 软限幅
// ──────────────────────────────────────────────

/**
 * @brief 软限幅函数 (tanh近似)
 *
 * 使用多项式逼近 tanh 曲线，避免硬削波失真。
 * 在阈值以下线性通过，超过阈值后平滑压缩。
 *
 * @param x 输入采样值
 * @param threshold 限幅阈值
 * @return 限幅后的采样值
 */
double MultibandCompress3::softClip(double x, double threshold) const
{
    if (qAbs(x) <= threshold) return x;

    // 三次多项式软限幅: y = threshold * sign(x) * (1 - |1 - |x|/threshold|^2)
    double normalized = qAbs(x) / threshold;
    if (normalized > 2.0) {
        // 超过2倍阈值时硬限幅
        return (x > 0) ? threshold * 1.5 : -threshold * 1.5;
    }

    double clipped = threshold * (1.5 - 0.5 * (2.0 - normalized) * (2.0 - normalized));
    return (x > 0) ? clipped : -clipped;
}
