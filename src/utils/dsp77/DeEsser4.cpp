/**
 * @file DeEsser4.cpp
 * @brief 去齿音处理器实现 — 频段检测 + 自适应增益 + 包络平滑
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 实现去齿音（De-Esser）处理器，用于检测和抑制语音中的齿音(sibilance)。
 * 使用带通滤波器提取齿音频段（4~9kHz），通过包络检测器跟踪齿音强度，
 * 当超过阈值时自适应降低增益，实现自然去齿音效果。
 */

#include "utils/dsp77/DeEsser4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 常量定义
// ──────────────────────────────────────────────

/** @brief 默认采样率(Hz) */
static constexpr double kSampleRate = 44100.0;

/** @brief 默认攻击系数(快速响应齿音) */
static constexpr double kDefaultAttackMs = 0.5;

/** @brief 默认释放系数(平滑恢复) */
static constexpr double kDefaultReleaseMs = 50.0;

/** @brief 最大衰减量(dB) */
static constexpr double kMaxReductionDb = -24.0;

/** @brief 最小信号幅值，用于避免log(0) */
static constexpr double kEpsilon = 1e-10;

// ──────────────────────────────────────────────
// 内部状态结构
// ──────────────────────────────────────────────

/**
 * @brief 滤波器延迟线状态，存储IIR滤波器的历史采样
 */
struct FilterState {
    double x1 = 0.0; ///< 上一个输入采样
    double x2 = 0.0; ///< 上上一个输入采样
    double y1 = 0.0; ///< 上一个输出采样
    double y2 = 0.0; ///< 上上一个输出采样
};

/**
 * @brief 包络跟随器状态
 */
struct EnvelopeState {
    double level     = 0.0;  ///< 当前包络电平(线性)
    double attackCoeff  = 0.0; ///< 攻击平滑系数
    double releaseCoeff = 0.0; ///< 释放平滑系数
};

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认去齿音参数
 * @param parent 父QObject对象
 *
 * 默认检测范围: 4000~9000 Hz
 * 默认阈值: -20 dB
 */
DeEsser4::DeEsser4(QObject* parent)
    : QObject(parent)
    , m_filterState(std::make_shared<FilterState>())
    , m_envelopeState(std::make_shared<EnvelopeState>())
    , m_currentReduction(0.0)
    , m_prevGain(1.0)
{
    setObjectName(QStringLiteral("DeEsser4"));

    // 初始化包络系数
    m_envelopeState->attackCoeff  = qExp(-1.0 / (kDefaultAttackMs  * 0.001 * kSampleRate));
    m_envelopeState->releaseCoeff = qExp(-1.0 / (kDefaultReleaseMs * 0.001 * kSampleRate));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置齿音检测频率范围(Hz)
 *
 * 齿音(sibilance)通常分布在 4kHz~9kHz 范围。
 * 该范围用于构造带通滤波器，提取齿音频段。
 *
 * @param minHz 最低频率(Hz)，范围 [2000, 10000]
 * @param maxHz 最高频率(Hz)，范围 [3000, 16000]，必须大于 minHz
 */
void DeEsser4::setFrequencyRange(double minHz, double maxHz)
{
    m_minHz = qBound(2000.0, minHz, 10000.0);
    m_maxHz = qBound(m_minHz + 500.0, maxHz, 16000.0);
}

/**
 * @brief 设置抑制阈值(dB)
 *
 * 当齿音频段能量超过此阈值时开始衰减。
 * 较低的值更敏感（更多齿音被抑制）。
 *
 * @param thresholdDb 阈值(dB)，范围 [-40, 0]
 */
void DeEsser4::setThreshold(double thresholdDb)
{
    m_threshold = qBound(-40.0, thresholdDb, 0.0);
}

// ──────────────────────────────────────────────
// 核心处理
// ──────────────────────────────────────────────

/**
 * @brief 对输入信号执行去齿音处理
 *
 * 完整处理流程:
 * 1. 使用二阶 IIR 带通滤波器提取齿音频段信号
 * 2. 通过包络跟随器跟踪齿音频段能量变化
 * 3. 将包络电平转换为dB并与阈值比较
 * 4. 超过阈值时按比例计算增益衰减
 * 5. 对增益曲线进行多遍平滑（避免咔嗒声）
 * 6. 应用增益: 输出 = 原始 + (gain-1) * 齿音成分
 *
 * @param input 输入音频采样数据
 * @return 去齿音后的音频数据
 */
QVector<double> DeEsser4::process(const QVector<double>& input)
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

    // ── 步骤1: 带通滤波提取齿音频段 ──
    QVector<double> sibilant = applyBandpass(input);

    // ── 步骤2: 包络检测 + 增益计算 ──
    QVector<double> gain(n, 1.0);
    double peakReduction = 0.0;
    int sibilanceEvents = 0;

    double threshLin = qPow(10.0, m_threshold / 20.0);
    auto& env = *m_envelopeState;

    for (int i = 0; i < n; ++i) {
        double absSib = qAbs(sibilant[i]);

        // 包络跟随（攻击快，释放慢）
        double coeff = (absSib > env.level)
            ? env.attackCoeff : env.releaseCoeff;
        env.level += coeff * (absSib - env.level);
        env.level = qMax(0.0, env.level);

        // 与阈值比较
        if (env.level > threshLin && env.level > kEpsilon) {
            // 计算超过阈值的dB数
            double overDb = 20.0 * qLn(env.level / threshLin) / qLn(10.0);
            // 衰减曲线: 衰减80%的超出部分（保留自然感）
            double reductionDb = overDb * 0.8;
            reductionDb = qBound(0.0, reductionDb, qAbs(kMaxReductionDb));

            gain[i] = qPow(10.0, -reductionDb / 20.0);
            gain[i] = qBound(0.1, gain[i], 1.0);

            if (reductionDb > peakReduction) {
                peakReduction = reductionDb;
            }
            sibilanceEvents++;
        }
    }

    // ── 步骤3: 增益平滑（3遍移动平均，消除突变） ──
    smoothGainCurve(gain, n, 3);

    // ── 步骤4: 与前一帧增益平滑衔接 ──
    for (int i = 0; i < qMin(8, n); ++i) {
        double blend = static_cast<double>(i) / 8.0;
        gain[i] = m_prevGain * (1.0 - blend) + gain[i] * blend;
    }
    m_prevGain = (n > 0) ? gain[n - 1] : 1.0;

    // ── 步骤5: 应用增益 ──
    // 输出 = 原始信号 + (gain - 1) * 齿音成分
    // gain < 1 时减少齿音，gain = 1 时不受影响
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        output[i] = input[i] + (gain[i] - 1.0) * sibilant[i];
    }

    // ── 步骤6: 更新统计 ──
    m_currentReduction = peakReduction;
    m_stats.totalFramesProcessed++;
    m_stats.totalSibilanceEvents += sibilanceEvents;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    // ── 步骤7: 发射信号 ──
    if (peakReduction > 0.5) {
        double centerFreq = (m_minHz + m_maxHz) * 0.5;
        emit sibilanceDetected(centerFreq, peakReduction);
    }

    return output;
}

/**
 * @brief 获取当前齿音增益 reduction
 * @return 当前最大衰减量(dB)，正值表示衰减量
 */
double DeEsser4::currentReduction() const
{
    return m_currentReduction;
}

/**
 * @brief 重置所有累计统计信息
 */
void DeEsser4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_currentReduction = 0.0;
    m_prevGain = 1.0;
    m_envelopeState->level = 0.0;
    m_filterState = std::make_shared<FilterState>();
}

// ──────────────────────────────────────────────
// 私有方法 — 带通滤波
// ──────────────────────────────────────────────

/**
 * @brief 二阶 IIR 带通滤波器
 *
 * 使用双二阶(biquad)结构实现带通滤波，
 * 中心频率 = sqrt(minHz * maxHz)，带宽 = maxHz - minHz。
 * 滤波器状态跨帧保持，确保处理连续性。
 *
 * @param input 输入信号
 * @return 滤波后的齿音频段信号
 */
QVector<double> DeEsser4::applyBandpass(const QVector<double>& input) const
{
    const int n = input.size();
    if (n == 0) return {};

    // 带通滤波器参数
    double centerFreq = qSqrt(m_minHz * m_maxHz);  // 几何中心频率
    double bandwidth  = m_maxHz - m_minHz;

    double omega = 2.0 * M_PI * centerFreq / kSampleRate;
    double alpha = qSin(omega) * bandwidth / (2.0 * centerFreq);

    // 双二阶系数
    double b0 =  alpha;
    double b1 =  0.0;
    double b2 = -alpha;
    double a0 =  1.0 + alpha;
    double a1 = -2.0 * qCos(omega);
    double a2 =  1.0 - alpha;

    // 归一化
    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;

    // 应用滤波器（直接 II 型，保持跨帧状态）
    auto& st = *m_filterState;
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double x0 = input[i];
        output[i] = b0 * x0 + b1 * st.x1 + b2 * st.x2
                     - a1 * st.y1 - a2 * st.y2;
        st.x2 = st.x1; st.x1 = x0;
        st.y2 = st.y1; st.y1 = output[i];
    }

    return output;
}

// ──────────────────────────────────────────────
// 私有方法 — 增益平滑
// ──────────────────────────────────────────────

/**
 * @brief 对增益曲线进行多遍移动平均平滑
 *
 * 使用3点加权平均 [0.2, 0.6, 0.2] 进行平滑，
 * 执行多遍以获得更平滑的过渡。
 * 边界采样保持不变，避免端点效应。
 *
 * @param gain 增益曲线（就地修改）
 * @param n 采样数
 * @param passes 平滑遍数
 */
void DeEsser4::smoothGainCurve(QVector<double>& gain, int n, int passes) const
{
    for (int pass = 0; pass < passes; ++pass) {
        for (int i = 1; i < n - 1; ++i) {
            gain[i] = gain[i] * 0.6 + (gain[i - 1] + gain[i + 1]) * 0.2;
        }
    }
}
