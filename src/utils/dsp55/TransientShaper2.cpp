/**
 * @file TransientShaper2.cpp
 * @brief 瞬态塑形器实现 — 瞬态检测 + 增益调制
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 通过分析信号包络中的瞬态成分，对瞬态进行增强或衰减。
 * 使用差分包络检测瞬态，根据 attack/sustain 灵敏度和
 * amount 参数计算增益曲线，应用到原始信号上。
 */

#include "utils/dsp55/TransientShaper2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象
 */
TransientShaper2::TransientShaper2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("TransientShaper2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置 Attack 灵敏度
 *
 * 控制对信号上升沿（瞬态）的检测灵敏度。
 * 值越大，越容易检测到快速变化的瞬态成分。
 *
 * @param sens 灵敏度系数，范围 [0.0, 2.0]
 */
void TransientShaper2::setAttackSensitivity(double sens)
{
    m_attackSens = qBound(0.0, sens, 2.0);
}

/**
 * @brief 设置 Sustain 灵敏度
 *
 * 控制对信号持续成分的抑制程度。
 * 值越大，持续成分被抑制得越多，瞬态越突出。
 *
 * @param sens 灵敏度系数，范围 [0.0, 2.0]
 */
void TransientShaper2::setSustainSensitivity(double sens)
{
    m_sustainSens = qBound(0.0, sens, 2.0);
}

/**
 * @brief 设置瞬态增益量
 *
 * 正值增强瞬态，负值衰减瞬态。
 * 0.0 表示不改变原始信号。
 *
 * @param amt 增益量，范围 [-2.0, 2.0]
 */
void TransientShaper2::setAmount(double amt)
{
    m_amount = qBound(-2.0, amt, 2.0);
}

// ──────────────────────────────────────────────
// 核心处理接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行瞬态塑形处理
 *
 * 处理流程：
 * 1. 计算信号包络（使用快速和慢速 RMS 检测器）
 * 2. 提取瞬态成分（差分包络）
 * 3. 根据参数计算增益调制曲线
 * 4. 将增益应用到原始信号
 *
 * @param input 输入音频采样数据
 * @return 经过瞬态塑形后的音频数据
 */
QVector<double> TransientShaper2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) {
        return {};
    }

    // 步骤1：计算快速包络和慢速包络
    QVector<double> fastEnv(n, 0.0);
    QVector<double> slowEnv(n, 0.0);

    const double fastCoeff = 0.99;  // 快速攻击/释放系数
    const double slowCoeff = 0.999; // 慢速攻击/释放系数
    const double fastRelease = 0.995;
    const double slowRelease = 0.9995;

    double fastVal = 0.0;
    double slowVal = 0.0;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(input[i]);

        // 快速包络（短时RMS近似）
        if (absVal > fastVal) {
            fastVal = fastCoeff * fastVal + (1.0 - fastCoeff) * absVal;
        } else {
            fastVal = fastRelease * fastVal + (1.0 - fastRelease) * absVal;
        }
        fastEnv[i] = fastVal;

        // 慢速包络（长时RMS近似）
        if (absVal > slowVal) {
            slowVal = slowCoeff * slowVal + (1.0 - slowCoeff) * absVal;
        } else {
            slowVal = slowRelease * slowVal + (1.0 - slowRelease) * absVal;
        }
        slowEnv[i] = slowVal;
    }

    // 步骤2：提取瞬态成分
    m_env = detectTransient(input);

    // 步骤3：计算增益调制曲线
    QVector<double> gain(n, 1.0);
    double peakTransient = 0.0;

    for (int i = 0; i < n; ++i) {
        // 瞬态强度 = 快速包络 - 慢速包络（差分）
        double transient = (fastEnv[i] - slowEnv[i]) * m_attackSens;

        // 持续成分抑制
        double sustain = slowEnv[i] * m_sustainSens;

        // 综合调制量
        double modulation = transient - sustain;

        // 用tanh进行软限幅
        modulation = qTanh(modulation * 2.0) * m_amount;

        // 计算增益
        gain[i] = qPow(10.0, modulation * 0.5); // 转为线性增益

        // 限制增益范围
        gain[i] = qBound(0.1, gain[i], 10.0);

        if (qAbs(transient) > peakTransient) {
            peakTransient = qAbs(transient);
        }
    }

    // 步骤4：应用增益
    QVector<double> output(n);
    for (int i = 0; i < n; ++i) {
        output[i] = input[i] * gain[i];
    }

    // 平滑过渡：对增益曲线进行低通滤波，避免咔嗒声
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 1; i < n - 1; ++i) {
            output[i] = output[i] * 0.5 + (output[i - 1] + output[i + 1]) * 0.25;
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, peakTransient);
    return output;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含处理次数、总采样数和平均耗时的Stats结构
 */
TransientShaper2::Stats TransientShaper2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void TransientShaper2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 瞬态检测
// ──────────────────────────────────────────────

/**
 * @brief 检测信号中的瞬态成分
 *
 * 通过对信号进行微分（差分）并取绝对值来检测瞬态。
 * 使用一阶差分和高斯平滑来提取包络。
 *
 * @param sig 输入信号
 * @return 瞬态包络，与输入等长
 */
QVector<double> TransientShaper2::detectTransient(const QVector<double>& sig)
{
    const int n = sig.size();
    if (n < 3) {
        return QVector<double>(n, 0.0);
    }

    // 一阶差分（微分近似）
    QVector<double> diff(n, 0.0);
    for (int i = 1; i < n; ++i) {
        diff[i] = qAbs(sig[i] - sig[i - 1]);
    }

    // 半波整流：只保留正差分（上升沿）
    QVector<double> rectified(n, 0.0);
    for (int i = 1; i < n; ++i) {
        double d = sig[i] - sig[i - 1];
        rectified[i] = (d > 0.0) ? d : 0.0;
    }

    // 高斯平滑
    const int kernelRadius = 8;
    QVector<double> kernel(2 * kernelRadius + 1);
    double kernelSum = 0.0;
    const double sigma = 3.0;
    for (int k = -kernelRadius; k <= kernelRadius; ++k) {
        double val = qExp(-(k * k) / (2.0 * sigma * sigma));
        kernel[k + kernelRadius] = val;
        kernelSum += val;
    }
    for (int k = 0; k < kernel.size(); ++k) {
        kernel[k] /= kernelSum;
    }

    // 卷积平滑
    QVector<double> envelope(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = -kernelRadius; k <= kernelRadius; ++k) {
            int idx = i + k;
            if (idx >= 0 && idx < n) {
                sum += rectified[idx] * kernel[k + kernelRadius];
            }
        }
        envelope[i] = sum;
    }

    return envelope;
}
