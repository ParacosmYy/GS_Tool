/**
 * @file SerialTimingAnalyzer.cpp
 * @brief 串口时序分析器 -- 核心实现
 *
 * 包含字节记录、帧标记、自动帧检测、时序统计计算。
 * 使用 QElapsedTimer::nsecsElapsed() 提供纳秒级时间戳。
 */

#include "serial/timing/SerialTimingAnalyzer.h"

#include <QtMath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 常量
// ═══════════════════════════════════════════════════════════

/// 纳秒到微秒的转换因子
static constexpr double kNsToUs = 1000.0;

// ═══════════════════════════════════════════════════════════
// 构造 / 生命周期
// ═══════════════════════════════════════════════════════════

/**
 * @brief 构造串口时序分析器
 *
 * 初始化所有采样缓冲区和统计计数器。
 * @param frameGapThresholdNs 自动帧检测阈值(纳秒)，默认1ms
 * @param parent 父对象
 */
SerialTimingAnalyzer::SerialTimingAnalyzer(qint64 frameGapThresholdNs, QObject* parent)
    : QObject(parent)
    , m_frameGapThresholdNs(frameGapThresholdNs)
    , m_stats{}
{
}

// ═══════════════════════════════════════════════════════════
// 数据记录
// ═══════════════════════════════════════════════════════════

/**
 * @brief 记录单个字节到达
 *
 * 获取当前纳秒时间戳，计算与前一字节的时间间隔，
 * 存入采样缓冲区，并检查是否触发自动帧边界。
 * @param byte 数据字节(0x00~0xFF)
 */
void SerialTimingAnalyzer::recordByte(uint8_t byte)
{
    if (!m_running) {
        return;
    }

    const qint64 nowNs = m_timer.nsecsElapsed();
    const qint64 deltaNs = (m_lastByteNs > 0) ? (nowNs - m_lastByteNs) : 0;

    TimingSample sample;
    sample.timestampNs = nowNs;
    sample.byte = byte;
    sample.deltaFromPrevNs = deltaNs;
    m_samples.append(sample);

    // 自动帧检测: 首个字节开始一个新帧，间隔超阈值则切帧
    if (deltaNs > 0) {
        checkAutoFrameBoundary(deltaNs);
    }
    if (!m_autoInFrame) {
        m_autoInFrame = true;
        m_autoFrameStartNs = nowNs;
        m_autoFrameBytes = 0;
    }
    m_autoFrameBytes++;

    // 空闲时间累积: 仅当有前一字节时计算
    if (m_lastByteNs > 0 && deltaNs > m_frameGapThresholdNs) {
        const qint64 idleNs = deltaNs - m_frameGapThresholdNs;
        m_totalIdleNs += static_cast<double>(idleNs);
    }

    m_lastByteNs = nowNs;
}

// ═══════════════════════════════════════════════════════════
// 帧标记
// ═══════════════════════════════════════════════════════════

/** @brief 手动标记帧起始位置，记录帧起始纳秒时间戳 */
void SerialTimingAnalyzer::markFrameStart()
{
    if (!m_running) {
        return;
    }
    m_inFrame = true;
    m_frameStartNs = m_timer.nsecsElapsed();
    m_frameByteCount = 0;
}

/**
 * @brief 手动标记帧结束位置
 *
 * 完成当前帧，记录帧时序(起止时间、持续时间、字节计数)，
 * 计算帧间间隔，发射 frameCompleted 信号。
 */
void SerialTimingAnalyzer::markFrameEnd()
{
    if (!m_running || !m_inFrame) {
        return;
    }
    const qint64 endNs = m_timer.nsecsElapsed();
    FrameTiming frame;
    frame.startNs = m_frameStartNs;
    frame.endNs = endNs;
    frame.durationNs = endNs - m_frameStartNs;
    frame.byteCount = m_frameByteCount;
    m_frames.append(frame);

    // 帧间间隔: 至少有两帧才能计算
    if (m_frames.size() >= 2) {
        const qint64 gap = frame.startNs - m_frames[m_frames.size() - 2].endNs;
        m_interFrameGapsNs.append(gap);
    }

    m_inFrame = false;
    emit frameCompleted(frame);
}

// ═══════════════════════════════════════════════════════════
// 采集控制
// ═══════════════════════════════════════════════════════════

/** @brief 开始时序采集 -- 启动计时器，重置采样数据，进入采集状态 */
void SerialTimingAnalyzer::start()
{
    if (m_running) {
        return;
    }
    m_samples.clear();
    m_frames.clear();
    m_interFrameGapsNs.clear();
    m_lastByteNs = 0;
    m_totalIdleNs = 0.0;
    m_inFrame = false;
    m_autoInFrame = false;
    m_autoFrameStartNs = 0;
    m_autoFrameBytes = 0;

    m_timer.start();
    m_running = true;
    emit sessionStarted();
}

/** @brief 停止时序采集 -- 完成未关闭的帧，更新累计统计 */
void SerialTimingAnalyzer::stop()
{
    if (!m_running) {
        return;
    }

    // 完成未关闭的手动帧
    if (m_inFrame) {
        markFrameEnd();
    }

    // 完成未关闭的自动帧
    if (m_autoInFrame && m_autoFrameBytes > 0) {
        const qint64 endNs = m_timer.nsecsElapsed();
        FrameTiming frame;
        frame.startNs = m_autoFrameStartNs;
        frame.endNs = endNs;
        frame.durationNs = endNs - m_autoFrameStartNs;
        frame.byteCount = m_autoFrameBytes;
        m_frames.append(frame);
        m_autoInFrame = false;
    }

    m_running = false;

    // 更新累计统计
    m_stats.totalSessions++;
    m_stats.totalBytesRecorded += static_cast<quint64>(m_samples.size());
    m_stats.totalFramesMarked += static_cast<quint64>(m_frames.size());

    // 峰值字节率: 总字节/总时间
    const qint64 totalNs = m_timer.nsecsElapsed();
    if (totalNs > 0) {
        const double rate = static_cast<double>(m_samples.size()) * 1e9
                            / static_cast<double>(totalNs);
        if (rate > m_stats.peakByteRate) {
            m_stats.peakByteRate = rate;
        }
    }

    emit sessionStopped(timingStats());
}

/** @brief 重置当前采样数据(不影响累计统计)，计时器保持运行 */
void SerialTimingAnalyzer::reset()
{
    m_samples.clear();
    m_frames.clear();
    m_interFrameGapsNs.clear();
    m_lastByteNs = 0;
    m_totalIdleNs = 0.0;
    m_inFrame = false;
    m_autoInFrame = false;
    m_autoFrameStartNs = 0;
    m_autoFrameBytes = 0;

    if (m_running) {
        m_timer.restart();
    }
    m_stats.totalResets++;
}

/** @brief 查询是否正在采集 @return true 表示正在采集 */
bool SerialTimingAnalyzer::isRunning() const
{
    return m_running;
}

// ═══════════════════════════════════════════════════════════
// 统计查询
// ═══════════════════════════════════════════════════════════

/** @brief 获取当前时序统计摘要 @return TimingStats快照 */
TimingStats SerialTimingAnalyzer::timingStats() const
{
    if (!m_running && m_samples.isEmpty()) {
        return TimingStats{};
    }
    return computeStats();
}

/** @brief 获取所有采样点(只读) @return TimingSample向量引用 */
const QVector<TimingSample>& SerialTimingAnalyzer::samples() const
{
    return m_samples;
}

/** @brief 获取帧时序列表(只读) @return FrameTiming向量引用 */
const QVector<FrameTiming>& SerialTimingAnalyzer::frameTimings() const
{
    return m_frames;
}

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

/** @brief 设置自动帧检测的字节间隔阈值 @param thresholdNs 阈值(纳秒) */
void SerialTimingAnalyzer::setFrameGapThreshold(qint64 thresholdNs)
{
    m_frameGapThresholdNs = qMax(0LL, thresholdNs);
}

/** @brief 获取当前帧间隔阈值(纳秒) */
qint64 SerialTimingAnalyzer::frameGapThreshold() const
{
    return m_frameGapThresholdNs;
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

/**
 * @brief 检测自动帧边界
 *
 * 当相邻字节间隔超过阈值时，自动完成上一帧并开始新帧。
 * @param deltaNs 当前字节与前一字节的间隔(纳秒)
 */
void SerialTimingAnalyzer::checkAutoFrameBoundary(qint64 deltaNs)
{
    if (deltaNs > m_frameGapThresholdNs && m_autoInFrame && m_autoFrameBytes > 0) {
        // 完成上一帧
        const qint64 prevByteNs = m_lastByteNs - deltaNs;
        FrameTiming frame;
        frame.startNs = m_autoFrameStartNs;
        frame.endNs = prevByteNs;
        frame.durationNs = prevByteNs - m_autoFrameStartNs;
        frame.byteCount = m_autoFrameBytes;
        m_frames.append(frame);

        // 帧间间隔
        if (m_frames.size() >= 2) {
            const qint64 gap = m_autoFrameStartNs - m_frames[m_frames.size() - 2].endNs;
            m_interFrameGapsNs.append(gap);
        }

        emit frameCompleted(frame);
        m_autoInFrame = false;
    }
}

/**
 * @brief 计算完整时序统计
 *
 * 从采样数据中聚合: 字节间延迟 min/max/avg/stddev、帧定时统计、
 * 帧间间隔统计、空闲周期统计。
 * @return TimingStats快照
 */
TimingStats SerialTimingAnalyzer::computeStats() const
{
    TimingStats stats;
    stats.totalBytes = m_samples.size();

    if (m_samples.isEmpty()) {
        return stats;
    }

    // 总持续时间
    stats.totalDurationUs = static_cast<double>(
        m_samples.last().timestampNs - m_samples.first().timestampNs) / kNsToUs;

    // ── 字节间延迟统计 ──
    if (m_samples.size() >= 2) {
        double minDelta = std::numeric_limits<double>::max();
        double maxDelta = 0.0;
        double sumDelta = 0.0;
        int count = 0;

        for (int i = 1; i < m_samples.size(); ++i) {
            const double deltaUs = static_cast<double>(m_samples[i].deltaFromPrevNs) / kNsToUs;
            if (deltaUs < minDelta) minDelta = deltaUs;
            if (deltaUs > maxDelta) maxDelta = deltaUs;
            sumDelta += deltaUs;
            count++;
        }

        stats.minInterByteUs = minDelta;
        stats.maxInterByteUs = maxDelta;
        stats.avgInterByteUs = (count > 0) ? sumDelta / count : 0.0;

        // 抖动(标准差)
        if (count > 1) {
            double sumSqDiff = 0.0;
            for (int i = 1; i < m_samples.size(); ++i) {
                const double deltaUs = static_cast<double>(m_samples[i].deltaFromPrevNs) / kNsToUs;
                const double diff = deltaUs - stats.avgInterByteUs;
                sumSqDiff += diff * diff;
            }
            stats.jitterUs = qSqrt(sumSqDiff / count);
        }
    }

    // ── 帧定时统计 ──
    stats.frameCount = m_frames.size();
    if (!m_frames.isEmpty()) {
        double minDur = std::numeric_limits<double>::max();
        double maxDur = 0.0;
        double sumDur = 0.0;

        for (const auto& frame : m_frames) {
            const double durUs = static_cast<double>(frame.durationNs) / kNsToUs;
            if (durUs < minDur) minDur = durUs;
            if (durUs > maxDur) maxDur = durUs;
            sumDur += durUs;
        }

        stats.minFrameUs = minDur;
        stats.maxFrameUs = maxDur;
        stats.avgFrameUs = sumDur / m_frames.size();
    }

    // ── 帧间间隔统计 ──
    if (!m_interFrameGapsNs.isEmpty()) {
        double minGap = std::numeric_limits<double>::max();
        double maxGap = 0.0;
        double sumGap = 0.0;

        for (qint64 gapNs : m_interFrameGapsNs) {
            const double gapUs = static_cast<double>(gapNs) / kNsToUs;
            if (gapUs < minGap) minGap = gapUs;
            if (gapUs > maxGap) maxGap = gapUs;
            sumGap += gapUs;
        }

        stats.minInterFrameUs = minGap;
        stats.maxInterFrameUs = maxGap;
        stats.avgInterFrameUs = sumGap / m_interFrameGapsNs.size();
    }

    // ── 空闲周期统计 ──
    stats.totalIdleUs = m_totalIdleNs / kNsToUs;

    // 最大空闲: 从帧间间隔中取最大值(帧间的间隔即为空闲)
    if (!m_interFrameGapsNs.isEmpty()) {
        const qint64 maxGapNs = *std::max_element(m_interFrameGapsNs.begin(),
                                                    m_interFrameGapsNs.end());
        stats.maxIdleUs = static_cast<double>(maxGapNs) / kNsToUs;
    } else if (m_samples.size() >= 2) {
        // 无帧标记时，最大字节间间隔即为最大空闲
        qint64 maxDelta = 0;
        for (int i = 1; i < m_samples.size(); ++i) {
            if (m_samples[i].deltaFromPrevNs > maxDelta) {
                maxDelta = m_samples[i].deltaFromPrevNs;
            }
        }
        stats.maxIdleUs = static_cast<double>(maxDelta) / kNsToUs;
    }

    return stats;
}
