/**
 * @file DataFlowMeter.cpp
 * @brief 数据流量计实现 -- 实时吞吐量测量/突发检测/空闲检测/流量画像
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计查询与重置方法见：@see DataFlowMeterStats.cpp
 */

#include "utils/flow/DataFlowMeter.h"

#include <QDateTime>
#include <QtMath>

// ──────────────────────────────────────────────
// 构造与析构
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，启动采样定时器
 * @param parent 父对象
 *
 * 初始化采样间隔为 1000ms，移动平均窗口为 5 秒。
 * 定时器启动后每 intervalMs 触发一次 onSampleTick()。
 */
DataFlowMeter::DataFlowMeter(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataFlowMeter"));

    connect(&m_sampleTimer, &QTimer::timeout,
            this, &DataFlowMeter::onSampleTick);
    m_sampleTimer.start(m_samplingIntervalMs);

    m_elapsed.start();
    m_lastDataTimeMs = 0;
}

/** @brief 析构函数 */
DataFlowMeter::~DataFlowMeter() = default;

// ──────────────────────────────────────────────
// 数据输入接口
// ──────────────────────────────────────────────

/**
 * @brief 喂入字节数据
 *
 * 将数据计入当前采样周期的累计值。每次调用视为一次数据包到达。
 * 如果之前处于空闲状态，标记为非空闲并记录数据到达时间。
 *
 * @param count 本次数据字节数
 * @param dir 数据流方向(Rx/Tx)
 */
void DataFlowMeter::feedBytes(int count, Direction dir)
{
    if (count <= 0) {
        return;
    }

    if (dir == Direction::Rx) {
        m_periodBytesRx += count;
        m_periodPacketsRx += 1;
        m_totalBytesRx += static_cast<quint64>(count);
        m_totalPacketsRx += 1;
    } else {
        m_periodBytesTx += count;
        m_periodPacketsTx += 1;
        m_totalBytesTx += static_cast<quint64>(count);
        m_totalPacketsTx += 1;
    }

    // 记录数据到达时间，用于空闲检测
    m_lastDataTimeMs = QDateTime::currentMSecsSinceEpoch();
    m_isIdle = false;
}

// ──────────────────────────────────────────────
// 吞吐量查询接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前合并(Rx+Tx)吞吐量
 * @return 移动平均平滑后的吞吐量(Bytes/s)
 */
double DataFlowMeter::getThroughput() const
{
    return m_movingAvg;
}

/**
 * @brief 获取所有当前指标的 QVariantMap 快照
 *
 * 输出字段说明：
 * - throughputBps: 合并吞吐量(Bytes/s)
 * - throughputRx: RX 吞吐量(Bytes/s)
 * - throughputTx: TX 吞吐量(Bytes/s)
 * - packetsPerSec: 合并包速率(包/秒)
 * - peakBps: 峰值吞吐量(Bytes/s)
 * - avgBps: 全局平均吞吐量(Bytes/s)
 * - totalBytesRx/Tx: 累计字节数
 * - totalPacketsRx/Tx: 累计包数
 * - burstCount: 突发次数
 * - idleCount: 空闲次数
 * - trafficProfile: 当前画像名称
 * - samplingCount: 采样次数
 *
 * @return 指标映射表
 */
QVariantMap DataFlowMeter::getMetrics() const
{
    // 计算包速率：使用最近采样点的包数 / 采样间隔
    double packetsPerSec = 0.0;
    if (!m_samples.isEmpty()) {
        const Sample &latest = m_samples.last();
        const double intervalSec = static_cast<double>(m_samplingIntervalMs) / 1000.0;
        if (intervalSec > 0.0) {
            packetsPerSec = static_cast<double>(latest.packets) / intervalSec;
        }
    }

    // 全局平均吞吐量
    double avgBps = 0.0;
    const qint64 elapsedMs = m_elapsed.elapsed();
    if (elapsedMs > 0) {
        const quint64 totalBytes = m_totalBytesRx + m_totalBytesTx;
        avgBps = static_cast<double>(totalBytes) / (static_cast<double>(elapsedMs) / 1000.0);
    }

    QVariantMap map;
    map[QStringLiteral("throughputBps")]    = m_movingAvg;
    map[QStringLiteral("throughputRx")]     = m_throughputRx;
    map[QStringLiteral("throughputTx")]     = m_throughputTx;
    map[QStringLiteral("packetsPerSec")]    = packetsPerSec;
    map[QStringLiteral("peakBps")]          = m_peakBps;
    map[QStringLiteral("avgBps")]           = avgBps;
    map[QStringLiteral("totalBytesRx")]     = QVariant::fromValue(m_totalBytesRx);
    map[QStringLiteral("totalBytesTx")]     = QVariant::fromValue(m_totalBytesTx);
    map[QStringLiteral("totalPacketsRx")]   = QVariant::fromValue(m_totalPacketsRx);
    map[QStringLiteral("totalPacketsTx")]   = QVariant::fromValue(m_totalPacketsTx);
    map[QStringLiteral("burstCount")]       = QVariant::fromValue(m_totalBursts);
    map[QStringLiteral("idleCount")]        = QVariant::fromValue(m_totalIdlePeriods);
    map[QStringLiteral("trafficProfile")]   = profileToString(m_currentProfile);
    map[QStringLiteral("samplingCount")]    = QVariant::fromValue(m_samplingCount);
    return map;
}

// ──────────────────────────────────────────────
// 配置接口
// ──────────────────────────────────────────────

/**
 * @brief 设置移动平均窗口大小
 *
 * 变更窗口大小不会清除已有采样数据，
 * 下次采样时将按新窗口大小计算移动平均。
 *
 * @param window 窗口大小枚举值
 */
void DataFlowMeter::setAverageWindow(AverageWindow window)
{
    m_avgWindow = window;
}

/**
 * @brief 设置采样间隔(毫秒)
 *
 * 变更间隔会重启采样定时器。有效范围 [100, 10000] ms。
 *
 * @param intervalMs 采样间隔，自动裁剪到有效范围
 */
void DataFlowMeter::setSamplingInterval(int intervalMs)
{
    m_samplingIntervalMs = qBound(100, intervalMs, 10000);
    m_sampleTimer.setInterval(m_samplingIntervalMs);
}

// ──────────────────────────────────────────────
// 定时器回调 — 核心采样与检测逻辑
// ──────────────────────────────────────────────

/**
 * @brief 采样定时器回调
 *
 * 工作流程：
 * 1. 将当前周期累计字节数转换为瞬时吞吐量
 * 2. 推入采样缓冲区并计算移动平均
 * 3. 执行突发检测(瞬时 > 移动平均 * kBurstRatio)
 * 4. 执行空闲检测(无数据超过 kIdleThresholdMs)
 * 5. 更新流量画像分类
 * 6. 发射 throughputChanged 信号
 */
void DataFlowMeter::onSampleTick()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    ++m_samplingCount;

    // ── 计算本周期瞬时吞吐量 ──
    const double intervalSec = static_cast<double>(m_samplingIntervalMs) / 1000.0;
    m_throughputRx = static_cast<double>(m_periodBytesRx) / intervalSec;
    m_throughputTx = static_cast<double>(m_periodBytesTx) / intervalSec;
    const double instantBps = m_throughputRx + m_throughputTx;
    const int periodPackets = m_periodPacketsRx + m_periodPacketsTx;

    // ── 创建采样点并推入缓冲区 ──
    Sample sample;
    sample.timestampMs = nowMs;
    sample.bytesPerSec = instantBps;
    sample.packets     = periodPackets;
    pushSample(sample);

    // ── 重置周期累计 ──
    m_periodBytesRx   = 0;
    m_periodBytesTx   = 0;
    m_periodPacketsRx = 0;
    m_periodPacketsTx = 0;

    // ── 计算移动平均 ──
    m_movingAvg = calcMovingAverage(nowMs);

    // ── 峰值追踪 ──
    if (m_movingAvg > m_peakBps) {
        m_peakBps = m_movingAvg;
    }

    // ── 突发检测 ──
    if (detectBurst(instantBps, m_movingAvg)) {
        ++m_totalBursts;
        emit burstDetected(instantBps);
    }

    // ── 空闲检测 ──
    if (!m_isIdle && m_lastDataTimeMs > 0) {
        const qint64 idleMs = nowMs - m_lastDataTimeMs;
        if (idleMs >= kIdleThresholdMs) {
            m_isIdle = true;
            ++m_totalIdlePeriods;
            emit idleDetected();
        }
    } else if (m_lastDataTimeMs == 0) {
        // 从未收到过数据，视为空闲
        if (!m_isIdle) {
            m_isIdle = true;
        }
    }

    // ── 更新流量画像 ──
    updateTrafficProfile(nowMs);

    // ── 发射吞吐量变更信号 ──
    emit throughputChanged(m_movingAvg);
}

// ──────────────────────────────────────────────
// 采样缓冲区操作
// ──────────────────────────────────────────────

/**
 * @brief 向采样缓冲区推入采样点，满时覆盖最旧数据
 * @param sample 要推入的采样数据
 */
void DataFlowMeter::pushSample(const Sample &sample)
{
    if (m_samples.size() >= kMaxSamples) {
        m_samples.removeFirst();
    }
    m_samples.append(sample);
}

/**
 * @brief 计算移动平均吞吐量
 *
 * 在配置的 AverageWindow 时间窗口内，
 * 对所有采样点的 bytesPerSec 取算术平均值。
 * 如果窗口内无采样点，返回 0.0。
 *
 * @param nowMs 当前时间戳(ms)
 * @return 窗口内平均吞吐量(Bytes/s)
 */
double DataFlowMeter::calcMovingAverage(qint64 nowMs) const
{
    const qint64 windowMs = static_cast<qint64>(static_cast<int>(m_avgWindow)) * 1000LL;
    const qint64 cutoffMs = nowMs - windowMs;

    double sum = 0.0;
    int count  = 0;

    // 从后向前遍历，跳过窗口外的采样点
    for (int i = m_samples.size() - 1; i >= 0; --i) {
        if (m_samples[i].timestampMs < cutoffMs) {
            break;
        }
        sum += m_samples[i].bytesPerSec;
        ++count;
    }

    if (count == 0) {
        return 0.0;
    }

    return sum / static_cast<double>(count);
}

// ──────────────────────────────────────────────
// 检测算法
// ──────────────────────────────────────────────

/**
 * @brief 突发检测
 *
 * 判定条件：瞬时吞吐量超过移动平均的 kBurstRatio 倍，
 * 且移动平均本身大于 0（避免无数据时误判）。
 *
 * @param instantBps 瞬时吞吐量(Bytes/s)
 * @param movingAvg 移动平均吞吐量(Bytes/s)
 * @return true 检测到突发
 */
bool DataFlowMeter::detectBurst(double instantBps, double movingAvg) const
{
    if (movingAvg <= 0.0 || instantBps <= 0.0) {
        return false;
    }
    return instantBps > (movingAvg * kBurstRatio);
}

/**
 * @brief 更新流量画像分类
 *
 * 分析最近 kProfileWindowSec 秒内的采样数据，
 * 基于吞吐量方差和模式进行分类：
 * - Idle: 窗口内平均吞吐量接近 0
 * - Constant: 吞吐量方差/均值 < 0.2（波动小）
 * - Bursty: 存在突发采样点（瞬时 > 3倍均值）占比 > 20%
 * - Periodic: 不满足以上条件时的默认分类
 *
 * 画像变更时发射 trafficProfileChanged 信号。
 *
 * @param nowMs 当前时间戳(ms)
 */
void DataFlowMeter::updateTrafficProfile(qint64 nowMs)
{
    const qint64 windowMs = static_cast<qint64>(kProfileWindowSec) * 1000LL;
    const qint64 cutoffMs = nowMs - windowMs;

    // 收集窗口内采样点
    double sum    = 0.0;
    int    count  = 0;
    double maxBps = 0.0;

    for (int i = m_samples.size() - 1; i >= 0; --i) {
        if (m_samples[i].timestampMs < cutoffMs) {
            break;
        }
        const double bps = m_samples[i].bytesPerSec;
        sum += bps;
        if (bps > maxBps) {
            maxBps = bps;
        }
        ++count;
    }

    TrafficProfile newProfile = TrafficProfile::Idle;

    if (count < 2 || sum < 1.0) {
        // 不足数据或几乎无流量 → Idle
        newProfile = TrafficProfile::Idle;
    } else {
        const double mean = sum / static_cast<double>(count);

        if (mean < 1.0) {
            // 平均吞吐量极低 → Idle
            newProfile = TrafficProfile::Idle;
        } else {
            // 计算方差
            double variance = 0.0;
            int burstSamples = 0;
            for (int i = m_samples.size() - 1; i >= 0; --i) {
                if (m_samples[i].timestampMs < cutoffMs) {
                    break;
                }
                const double diff = m_samples[i].bytesPerSec - mean;
                variance += diff * diff;

                // 突发采样计数
                if (m_samples[i].bytesPerSec > mean * kBurstRatio) {
                    ++burstSamples;
                }
            }
            variance /= static_cast<double>(count);

            const double coefficientOfVariation = qSqrt(variance) / mean;
            const double burstRatio = static_cast<double>(burstSamples) / static_cast<double>(count);

            if (coefficientOfVariation < 0.2) {
                // 波动小 → Constant
                newProfile = TrafficProfile::Constant;
            } else if (burstRatio > 0.2) {
                // 突发采样占比高 → Bursty
                newProfile = TrafficProfile::Bursty;
            } else {
                // 其他 → Periodic
                newProfile = TrafficProfile::Periodic;
            }
        }
    }

    // 仅在画像变更时发射信号
    if (newProfile != m_currentProfile) {
        m_currentProfile = newProfile;
        emit trafficProfileChanged(profileToString(newProfile));
    }
}

/**
 * @brief 将 TrafficProfile 枚举转为字符串
 * @param profile 画像枚举值
 * @return 画像名称字符串
 */
QString DataFlowMeter::profileToString(TrafficProfile profile)
{
    switch (profile) {
    case TrafficProfile::Constant: return QStringLiteral("constant");
    case TrafficProfile::Bursty:   return QStringLiteral("bursty");
    case TrafficProfile::Periodic: return QStringLiteral("periodic");
    case TrafficProfile::Idle:     return QStringLiteral("idle");
    }
    return QStringLiteral("idle");
}

// ──────────────────────────────────────────────
// 统计查询 — 见 DataFlowMeterStats.cpp
// ──────────────────────────────────────────────
