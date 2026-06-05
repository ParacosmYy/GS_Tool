/**
 * @file DeEsser3.cpp
 * @brief 去齿音处理器实现 — 齿音检测 + 带通滤波 + 自适应增益
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现去齿音（De-Esser）处理器，用于衰减人声中的齿音（sibilance）。
 * 通过带通滤波器提取齿音频段，检测齿音强度，
 * 当超过阈值时自适应降低该频段的增益。
 */

#include "utils/dsp58/DeEsser3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认去齿音参数
 * @param parent 父QObject对象
 */
DeEsser3::DeEsser3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DeEsser3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────
\
/**
 * @brief 设置齿音检测阈值
 *
 * 当齿音频段的能量超过此阈值时触发衰减。
 * 使用 dB 单位，越低的值越敏感。
 *
 * @param thresh 阈值（dB），范围 [-40, 0]
 */
void DeEsser3::setThreshold(double thresh)
{
    m_threshold = qBound(-40.0, thresh, 0.0);
}

/**
 * @brief 设置齿音中心频率
 *
 * 齿音主要分布在 4kHz~8kHz 范围内，
 * 默认中心频率为 6kHz。
 *
 * @param freq 中心频率（Hz）
 */
void DeEsser3::setFrequency(double freq)
{
    m_freq = qBound(1000.0, freq, 16000.0);
}

/**
 * @brief 设置齿音检测带宽
 *
 * 带宽决定了检测频段的宽度。
 * 较宽的带宽覆盖更多齿音成分。
 *
 * @param bw 带宽（Hz）
 */
void DeEsser3::setBandwidth(double bw)
{
    m_bw = qBound(100.0, bw, 10000.0);
}

// ──────────────────────────────────────────────
// 核心处理接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行去齿音处理
 *
 * 处理流程：
 * 1. 用带通滤波器提取齿音频段信号
 * 2. 计算齿音频段的包络电平
 * 3. 与阈值比较，超过阈值时计算衰减量
 * 4. 对齿音频段信号施加衰减
 * 5. 从原始信号中减去齿音成分并加回衰减后的齿音
 *
 * @param input 输入音频采样数据
 * @return 去齿音后的音频数据
 */
QVector<double> DeEsser3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return {};

    // 步骤1：带通滤波提取齿音频段
    QVector<double> sibilant = bandpassFilter(input, m_freq, m_bw);

    // 步骤2：计算齿音频段包络
    m_sibLevel = 0.0;
    const double sampleRate = 44100.0;
    const double attackCoeff = 0.01;
    const double releaseCoeff = 0.001;
    double envelope = 0.0;

    QVector<double> envelopeArr(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(sibilant[i]);
        if (absVal > envelope) {
            envelope = attackCoeff * envelope + (1.0 - attackCoeff) * absVal;
        } else {
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absVal;
        }
        envelopeArr[i] = envelope;
        if (envelope > m_sibLevel) m_sibLevel = envelope;
    }

    // 步骤3：计算增益衰减
    double threshLin = qPow(10.0, m_threshold / 20.0);
    double maxReduction = 0.0;

    QVector<double> gain(n, 1.0);
    for (int i = 0; i < n; ++i) {
        if (envelopeArr[i] > threshLin && envelopeArr[i] > 1e-10) {
            // 超过阈值：计算衰减量
            double overDb = 20.0 * qLn(envelopeArr[i] / threshLin) / qLn(10.0);
            double reductionDb = overDb * 0.8; // 衰减 80% 的超出部分
            gain[i] = qPow(10.0, -reductionDb / 20.0);
            gain[i] = qBound(0.1, gain[i], 1.0);

            if (reductionDb > maxReduction) maxReduction = reductionDb;
        }
    }

    // 步骤4：平滑增益曲线，避免咔嗒声
    for (int pass = 0; pass < 3; ++pass) {
        for (int i = 1; i < n - 1; ++i) {
            gain[i] = gain[i] * 0.6 + (gain[i - 1] + gain[i + 1]) * 0.2;
        }
    }

    // 步骤5：应用增益
    // 输出 = 原始信号 - 未衰减的齿音 + 衰减后的齿音
    //      = 原始信号 + (gain - 1) * 齿音成分
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        output[i] = input[i] + (gain[i] - 1.0) * sibilant[i];
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, maxReduction);
    return output;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含处理次数、总采样数和平均耗时的Stats结构
 */
DeEsser3::Stats DeEsser3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void DeEsser3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 带通滤波器
// ──────────────────────────────────────────────

/**
 * @brief 二阶 IIR 带通滤波器
 *
 * 实现双二阶（biquad）带通滤波器，
 * 从输入信号中提取指定频段的成分。
 *
 * @param sig 输入信号
 * @param freq 中心频率（Hz）
 * @param bw 带宽（Hz）
 * @return 滤波后的信号
 */
QVector<double> DeEsser3::bandpassFilter(const QVector<double>& sig, double freq, double bw)
{
    const int n = sig.size();
    if (n == 0) return {};

    const double sampleRate = 44100.0;

    // 计算滤波器系数
    double omega = 2.0 * M_PI * freq / sampleRate;
    double alpha = qSin(omega) * bw / (2.0 * freq);

    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * qCos(omega);
    double a2 = 1.0 - alpha;

    // 归一化
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    // 应用滤波器（直接 II 型）
    QVector<double> output(n, 0.0);
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

    for (int i = 0; i < n; ++i) {
        double x0 = sig[i];
        output[i] = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = output[i];
    }

    return output;
}
