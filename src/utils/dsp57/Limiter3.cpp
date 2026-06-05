/**
 * @file Limiter3.cpp
 * @brief 限幅器实现 — 阈值限幅 + 前视 + 释放时间
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现带前视（lookahead）的峰值限幅器。
 * 前视缓冲区允许限幅器在峰值到达之前预先降低增益，
 * 避免信号削波。释放时间控制增益恢复的速度。
 */

#include "utils/dsp57/Limiter3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认限幅参数
 * @param parent 父QObject对象
 */
Limiter3::Limiter3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("Limiter3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置限幅阈值（线性幅度）
 *
 * 信号超过此阈值时会被限幅。
 * 例如 -0.3 对应约 0.74 的线性幅度。
 *
 * @param thresh 阈值（线性幅度，范围 [-1.0, 1.0]）
 */
void Limiter3::setThreshold(double thresh)
{
    m_threshold = qBound(-1.0, thresh, 1.0);
}

/**
 * @brief 设置释放时间
 *
 * 释放时间控制增益从限幅状态恢复到正常的速度。
 * 较长的释放时间使得增益变化更平滑，
 * 较短的释放时间响应更快但可能产生失真。
 *
 * @param ms 释放时间（毫秒），范围 [1, 1000]
 */
void Limiter3::setReleaseTime(double ms)
{
    m_release = qBound(1.0, ms, 1000.0);
}

/**
 * @brief 设置前视缓冲区大小
 *
 * 前视允许限幅器提前感知即将到来的峰值。
 * 更大的前视值提供更好的峰值控制，但增加延迟。
 *
 * @param samples 前视采样数
 */
void Limiter3::setLookahead(int samples)
{
    m_lookahead = qMax(0, samples);
}

// ──────────────────────────────────────────────
// 核心处理接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行限幅处理
 *
 * 处理流程：
 * 1. 将信号通过前视延迟线
 * 2. 对每个采样计算所需的增益衰减量
 * 3. 使用攻击/释放包络跟随器平滑增益
 * 4. 应用增益到延迟后的信号
 *
 * @param input 输入音频采样数据
 * @return 限幅后的音频数据
 */
QVector<double> Limiter3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return {};

    // 阈值转换为线性幅度
    double threshLin = qPow(10.0, m_threshold / 20.0);
    threshLin = qBound(0.0, threshLin, 1.0);

    // 步骤1：前视延迟线
    QVector<double> delayed(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int srcIdx = i - m_lookahead;
        if (srcIdx >= 0 && srcIdx < n) {
            delayed[i] = input[srcIdx];
        }
    }

    // 步骤2：计算增益衰减曲线
    QVector<double> gainReductionCurve(n, 0.0);
    double envelope = 0.0;

    // 释放系数（基于采样率 44100Hz）
    const double sampleRate = 44100.0;
    double releaseCoeff = qExp(-1.0 / (m_release * sampleRate / 1000.0));
    double attackCoeff = 0.0; // 瞬时攻击

    m_peak = 0.0;
    m_gainReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        // 前视：检查当前和未来样本中的最大值
        double peakVal = qAbs(input[i]);
        for (int la = 1; la <= m_lookahead && (i + la) < n; ++la) {
            peakVal = qMax(peakVal, qAbs(input[i + la]));
        }

        // 更新包络（攻击：即时跟随，释放：指数衰减）
        if (peakVal > envelope) {
            envelope = peakVal; // 瞬时攻击
        } else {
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * peakVal;
        }

        // 计算所需增益
        double gain = 1.0;
        if (envelope > threshLin && envelope > 1e-10) {
            gain = threshLin / envelope;
        }

        gainReductionCurve[i] = gain;

        // 跟踪峰值和增益衰减
        if (qAbs(input[i]) > m_peak) {
            m_peak = qAbs(input[i]);
        }
        double reductionDb = 20.0 * qLn(qMax(gain, 1e-10)) / qLn(10.0);
        if (reductionDb < m_gainReduction) {
            m_gainReduction = reductionDb;
        }
    }

    // 步骤3：应用增益
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        output[i] = delayed[i] * gainReductionCurve[i];
    }

    // 步骤4：平滑增益曲线（减少量化噪声）
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 1; i < n - 1; ++i) {
            gainReductionCurve[i] = gainReductionCurve[i] * 0.7
                + (gainReductionCurve[i - 1] + gainReductionCurve[i + 1]) * 0.15;
        }
    }

    // 步骤5：应用增益
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        output[i] = delayed[i] * gainReductionCurve[i];
    }

    // 步骤6：最终硬限幅（安全网）
    for (int i = 0; i < n; ++i) {
        output[i] = qBound(-threshLin, output[i], threshLin);
    }

    // 步骤7：淡入淡出处理（避免首尾咔嗒声）
    const int fadeLen = qMin(32, n / 4);
    for (int i = 0; i < fadeLen; ++i) {
        double fade = static_cast<double>(i) / fadeLen;
        output[i] *= fade;
        output[n - 1 - i] *= fade;
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, m_peak);
    return output;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含处理次数、总采样数和平均耗时的Stats结构
 */
Limiter3::Stats Limiter3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void Limiter3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
