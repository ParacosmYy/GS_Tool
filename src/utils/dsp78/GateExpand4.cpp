/**
 * @file GateExpand4.cpp
 * @brief 门限扩展处理器实现 — 平滑门控 + 保持计时 + 自适应增益包络
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 实现带有 attack/hold/release 参数的门限扩展器。
 * 当信号低于阈值时平滑降低增益，当信号恢复时快速打开。
 * hold 参数防止阈值附近抖动，release 控制关闭速度以避免咔嗒声。
 */

#include "utils/dsp78/GateExpand4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 常量定义
// ──────────────────────────────────────────────

/** @brief 默认采样率(Hz) */
static constexpr double kSampleRate = 44100.0;

/** @brief 最大衰减范围(线性增益)，相当于 -80dB */
static constexpr double kFloorGain = 0.0001;

/** @brief 最小dB值 */
static constexpr double kMinDb = -120.0;

/** @brief 滞后阈值(dB)，防止阈值附近快速抖动 */
static constexpr double kHysteresisDb = 3.0;

// ──────────────────────────────────────────────
// 内部状态枚举
// ──────────────────────────────────────────────

/**
 * @brief 门状态枚举，描述门控处理器的当前阶段
 */
enum class GateState {
    Closed,   ///< 门关闭（信号被衰减）
    Opening,  ///< 门正在打开（attack阶段）
    Open,     ///< 门打开（信号正常通过）
    Holding,  ///< 门保持打开（hold阶段）
    Closing   ///< 门正在关闭（release阶段）
};

// ──────────────────────────────────────────────
// 内部状态结构
// ──────────────────────────────────────────────

/**
 * @brief 门控处理器运行时状态
 */
struct GateRuntime {
    GateState state = GateState::Closed;  ///< 当前门状态
    double currentGain = kFloorGain;       ///< 当前线性增益
    double holdCounter = 0.0;              ///< hold剩余采样数
    double envelopeDb = kMinDb;            ///< 信号包络(dB)
};

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化门限扩展处理器
 * @param parent 父QObject对象
 *
 * 默认参数: threshold=-40dB, attack=1ms, hold=50ms, release=100ms
 */
GateExpand4::GateExpand4(QObject* parent)
    : QObject(parent)
    , m_runtime(std::make_shared<GateRuntime>())
    , m_isOpen(false)
{
    setObjectName(QStringLiteral("GateExpand4"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置门限阈值(dB)
 *
 * 信号电平低于此阈值时门开始关闭。
 * 使用滞后机制: 打开阈值 = threshold，关闭阈值 = threshold - hysteresis。
 *
 * @param thresholdDb 阈值(dB)，范围 [-80, 0]
 */
void GateExpand4::setThreshold(double thresholdDb)
{
    m_threshold = qBound(-80.0, thresholdDb, 0.0);
}

/**
 * @brief 设置攻击/保持/释放时间(ms)
 *
 * attack: 信号超过阈值后增益恢复的速度（越小越快）
 * hold: 信号低于阈值后增益保持不变的时间（防止抖动）
 * release: hold结束后增益下降的速度（越大越平滑）
 *
 * @param attackMs 攻击时间(ms)，范围 [0.01, 200]
 * @param holdMs 保持时间(ms)，范围 [0, 2000]
 * @param releaseMs 释放时间(ms)，范围 [1, 5000]
 */
void GateExpand4::setTiming(double attackMs, double holdMs, double releaseMs)
{
    m_attackMs  = qBound(0.01, attackMs, 200.0);
    m_holdMs    = qBound(0.0, holdMs, 2000.0);
    m_releaseMs = qBound(1.0, releaseMs, 5000.0);
}

// ──────────────────────────────────────────────
// 核心处理
// ──────────────────────────────────────────────

/**
 * @brief 处理音频帧，应用门限扩展
 *
 * 完整处理流程:
 * 1. 计算每采样的瞬时电平(dB)并用包络跟随器平滑
 * 2. 与阈值比较，使用滞后机制确定目标增益
 * 3. hold计时器: 信号低于阈值后保持门打开一段时间
 * 4. 使用 attack/release 系数平滑增益变化
 * 5. 将增益应用到输出信号
 *
 * 状态机转换:
 *   Closed  --信号超阈值--> Opening --增益到1.0--> Open
 *   Open    --信号低阈值--> Holding --hold结束--> Closing
 *   Closing --信号超阈值--> Opening
 *   Closing --增益到floor--> Closed
 *
 * @param input 输入音频采样
 * @return 经门限处理后的音频采样
 */
QVector<double> GateExpand4::process(const QVector<double>& input)
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

    // 计算 attack/release 平滑系数
    double attackCoeff  = qExp(-1.0 / (m_attackMs  * 0.001 * kSampleRate));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * kSampleRate));

    // hold 计数器转换为采样数
    double holdSamples = m_holdMs * 0.001 * kSampleRate;

    // 打开/关闭阈值（滞后）
    double openThresholdDb  = m_threshold;
    double closeThresholdDb = m_threshold - kHysteresisDb;

    // 包络跟随器系数（中等速度，平滑信号）
    double envAttackCoeff  = qExp(-1.0 / (0.5 * 0.001 * kSampleRate));
    double envReleaseCoeff = qExp(-1.0 / (20.0 * 0.001 * kSampleRate));

    auto& rt = *m_runtime;
    QVector<double> output(n, 0.0);
    int gateOpenings = 0;
    bool wasOpen = (rt.state == GateState::Open || rt.state == GateState::Holding);

    for (int i = 0; i < n; ++i) {
        // ── 步骤1: 计算信号电平(dB) ──
        double absSample = qAbs(input[i]);
        double levelDb = (absSample > 1e-10)
            ? 20.0 * qLn(absSample) / qLn(10.0)
            : kMinDb;

        // ── 步骤2: 包络跟随 ──
        double envCoeff = (levelDb > rt.envelopeDb)
            ? envAttackCoeff : envReleaseCoeff;
        rt.envelopeDb += envCoeff * (levelDb - rt.envelopeDb);

        // ── 步骤3: 状态机更新 ──
        double targetGain = kFloorGain;

        switch (rt.state) {
        case GateState::Closed:
            // 等待信号超过打开阈值
            if (rt.envelopeDb >= openThresholdDb) {
                rt.state = GateState::Opening;
                gateOpenings++;
                targetGain = 1.0;
            } else {
                targetGain = kFloorGain;
            }
            break;

        case GateState::Opening:
            // 增益上升中
            targetGain = 1.0;
            if (rt.currentGain >= 0.99) {
                rt.state = GateState::Open;
            }
            if (rt.envelopeDb < closeThresholdDb) {
                rt.state = GateState::Closing;
            }
            break;

        case GateState::Open:
            // 门打开，信号正常通过
            targetGain = 1.0;
            if (rt.envelopeDb < closeThresholdDb) {
                rt.state = GateState::Holding;
                rt.holdCounter = holdSamples;
            }
            break;

        case GateState::Holding:
            // 保持阶段，信号低于阈值但仍保持门打开
            targetGain = 1.0;
            rt.holdCounter -= 1.0;
            if (rt.envelopeDb >= openThresholdDb) {
                // 信号恢复，回到打开状态
                rt.state = GateState::Open;
                rt.holdCounter = 0.0;
            } else if (rt.holdCounter <= 0.0) {
                // hold时间结束，开始关闭
                rt.state = GateState::Closing;
            }
            break;

        case GateState::Closing:
            // 增益下降中
            targetGain = kFloorGain;
            if (rt.envelopeDb >= openThresholdDb) {
                // 信号恢复，重新打开
                rt.state = GateState::Opening;
                gateOpenings++;
                targetGain = 1.0;
            } else if (rt.currentGain <= kFloorGain * 1.1) {
                rt.state = GateState::Closed;
            }
            break;
        }

        // ── 步骤4: 平滑增益过渡 ──
        double smoothCoeff = (targetGain > rt.currentGain)
            ? attackCoeff : releaseCoeff;
        rt.currentGain += smoothCoeff * (targetGain - rt.currentGain);

        // 限制增益范围
        rt.currentGain = qBound(kFloorGain, rt.currentGain, 1.0);

        // ── 步骤5: 应用增益 ──
        output[i] = input[i] * rt.currentGain;
    }

    // 更新门状态标志
    m_isOpen = (rt.state == GateState::Open
                || rt.state == GateState::Holding
                || rt.state == GateState::Opening);

    // ── 更新统计 ──
    m_stats.totalFramesProcessed++;
    m_stats.totalGateOpenings += gateOpenings;
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesProcessed;

    // ── 发射状态变化信号 ──
    bool isOpenNow = m_isOpen;
    if (isOpenNow != wasOpen) {
        emit gateStateChanged(isOpenNow);
    }

    return output;
}

/**
 * @brief 获取门限当前状态
 * @return true=门打开(信号通过)，false=门关闭(信号衰减)
 */
bool GateExpand4::isOpen() const
{
    return m_isOpen;
}

/**
 * @brief 重置所有累计统计信息
 */
void GateExpand4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_isOpen = false;
    m_runtime = std::make_shared<GateRuntime>();
}
