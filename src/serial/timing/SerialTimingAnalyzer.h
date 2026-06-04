/**
 * @file SerialTimingAnalyzer.h
 * @brief 串口时序分析器 -- 字节级精确时序测量与统计
 *
 * 使用 QElapsedTimer 提供纳秒级精度的串口时序分析:
 *   - 字节间延迟(inter-byte delay): 相邻两字节到达的时间间隔
 *   - 帧定时(frame timing): 通过 markFrameStart/markFrameEnd 标记的帧持续时间和帧间间隔
 *   - 空闲周期(idle period): 帧结束到下一帧开始之间的静默时间
 *   - 抖动(jitter): 字节间延迟的标准差，反映通信稳定性
 *   - 自动帧检测: 连续字节间隔超过阈值时自动切分帧边界
 *
 * 协作关系:
 *   - 数据源(recordByte) → SerialTimingAnalyzer → 统计查询/信号
 *   - markFrameStart/markFrameEnd 由上层协议解析器或手动调用
 *   - timingStats() 返回当前完整时序统计快照
 */
#ifndef SERIAL_TIMING_ANALYZER_H
#define SERIAL_TIMING_ANALYZER_H

#include <QObject>
#include <QElapsedTimer>
#include <QVector>
#include <cstdint>

/** @brief 单字节时序采样点 -- 记录每个到达字节的纳秒时间戳、字节值和与前一字节的间隔 */
struct TimingSample {
    qint64 timestampNs = 0;      ///< 字节到达的纳秒时间戳(相对于采集开始)
    uint8_t byte = 0;            ///< 字节值(0x00~0xFF)
    qint64 deltaFromPrevNs = 0;  ///< 与前一字节的纳秒间隔(首字节为0)
};

/** @brief 帧时序信息 -- 记录单个帧的起止时间和持续时间 */
struct FrameTiming {
    qint64 startNs = 0;          ///< 帧起始纳秒时间戳
    qint64 endNs = 0;            ///< 帧结束纳秒时间戳
    qint64 durationNs = 0;       ///< 帧持续时间(纳秒)
    int byteCount = 0;           ///< 帧内字节总数
};

/** @brief 时序统计摘要 -- 字节间延迟、帧定时、抖动等完整统计指标(时间单位: 微秒us) */
struct TimingStats {
    // ── 字节间延迟(inter-byte delay) ──
    double minInterByteUs = 0.0;   ///< 最小字节间延迟(us)
    double maxInterByteUs = 0.0;   ///< 最大字节间延迟(us)
    double avgInterByteUs = 0.0;   ///< 平均字节间延迟(us)
    double jitterUs = 0.0;         ///< 字节间延迟标准差/抖动(us)
    // ── 帧定时(frame timing) ──
    double minFrameUs = 0.0;       ///< 最短帧持续时间(us)
    double maxFrameUs = 0.0;       ///< 最长帧持续时间(us)
    double avgFrameUs = 0.0;       ///< 平均帧持续时间(us)
    int frameCount = 0;            ///< 已标记的帧总数
    // ── 帧间间隔(inter-frame gap) ──
    double minInterFrameUs = 0.0;  ///< 最小帧间间隔(us)
    double maxInterFrameUs = 0.0;  ///< 最大帧间间隔(us)
    double avgInterFrameUs = 0.0;  ///< 平均帧间间隔(us)
    // ── 空闲周期(idle period) ──
    double totalIdleUs = 0.0;      ///< 累计空闲时间(us)
    double maxIdleUs = 0.0;        ///< 最大单次空闲周期(us)
    // ── 总览 ──
    qint64 totalBytes = 0;         ///< 采集的总字节数
    qint64 totalDurationUs = 0.0;  ///< 总采集持续时间(us)
};

/** @brief 运行累计统计 -- 跨多次采集会话的累计计数器 */
struct TimingRuntimeStats {
    quint64 totalSessions = 0;       ///< 累计采集会话数
    quint64 totalBytesRecorded = 0;  ///< 累计记录的字节总数
    quint64 totalFramesMarked = 0;   ///< 累计标记的帧总数
    quint64 totalResets = 0;         ///< 累计重置次数
    double peakByteRate = 0.0;       ///< 历史最高字节速率(bytes/s)
};

/**
 * @brief 串口时序分析器 -- 字节级精确时序测量与统计引擎
 *
 * 使用方式:
 *   1. start() 开始时序采集(启动高精度计时器)
 *   2. recordByte() 逐字节记录到达时间
 *   3. markFrameStart()/markFrameEnd() 手动标记帧边界
 *   4. timingStats() 获取当前完整时序统计
 *   5. stop() 结束采集
 *   6. reset() 清空采样数据(不影响累计统计)
 */
class SerialTimingAnalyzer : public QObject {
    Q_OBJECT

public:
    /** @brief 构造串口时序分析器 @param frameGapThresholdNs 自动帧检测的字节间隔阈值(纳秒)，默认1ms @param parent 父对象 */
    explicit SerialTimingAnalyzer(qint64 frameGapThresholdNs = 1'000'000,
                                  QObject* parent = nullptr);

    /** @brief 记录单个字节到达，更新字节间延迟和自动帧检测 @param byte 数据字节 */
    void recordByte(uint8_t byte);

    /** @brief 手动标记帧起始位置 */
    void markFrameStart();

    /** @brief 手动标记帧结束位置，完成当前帧并记录帧时序 */
    void markFrameEnd();

    /** @brief 开始时序采集，启动计时器并重置采样缓冲区 */
    void start();

    /** @brief 停止时序采集，更新累计统计 */
    void stop();

    /** @brief 重置当前采样数据(不影响累计统计)，计时器保持运行 */
    void reset();

    /** @brief 查询是否正在采集 @return true 表示正在采集 */
    bool isRunning() const;

    /** @brief 获取当前时序统计摘要 @return TimingStats快照 */
    TimingStats timingStats() const;

    /** @brief 获取所有采样点(只读) @return TimingSample向量引用 */
    const QVector<TimingSample>& samples() const;

    /** @brief 获取帧时序列表(只读) @return FrameTiming向量引用 */
    const QVector<FrameTiming>& frameTimings() const;

    /** @brief 获取运行累计统计 @return TimingRuntimeStats常量引用 */
    const TimingRuntimeStats& stats() const;

    /** @brief 重置累计统计计数器(不影响当前采样数据) */
    void resetStatistics();

    /** @brief 设置自动帧检测的字节间隔阈值 @param thresholdNs 阈值(纳秒)，相邻字节间隔超过此值自动切帧 */
    void setFrameGapThreshold(qint64 thresholdNs);

    /** @brief 获取当前帧间隔阈值(纳秒) */
    qint64 frameGapThreshold() const;

signals:
    /** @brief 帧完成信号(markFrameEnd或自动帧检测触发) @param frame 帧时序信息 */
    void frameCompleted(const FrameTiming& frame);

    /** @brief 采集会话开始信号 */
    void sessionStarted();

    /** @brief 采集会话结束信号 @param stats 最终时序统计 */
    void sessionStopped(const TimingStats& stats);

private:
    /** @brief 检测自动帧边界 -- 字节间隔超过阈值时自动完成上一帧 */
    void checkAutoFrameBoundary(qint64 deltaNs);

    /** @brief 计算时序统计 -- 从采样数据聚合min/max/avg/stddev */
    TimingStats computeStats() const;

    // ── 配置 ──
    qint64 m_frameGapThresholdNs = 1'000'000;  ///< 自动帧检测阈值(纳秒)
    // ── 采样数据 ──
    QVector<TimingSample> m_samples;   ///< 字节时序采样缓冲区
    QVector<FrameTiming> m_frames;     ///< 帧时序列表
    QElapsedTimer m_timer;             ///< 高精度计时器
    bool m_running = false;            ///< 是否正在采集
    // ── 帧追踪状态 ──
    bool m_inFrame = false;            ///< 是否正在帧内(用于手动标记)
    qint64 m_frameStartNs = 0;         ///< 当前手动帧起始时间(纳秒)
    int m_frameByteCount = 0;          ///< 当前帧内字节数
    // ── 自动帧追踪 ──
    bool m_autoInFrame = false;        ///< 自动帧是否活跃
    qint64 m_autoFrameStartNs = 0;     ///< 自动帧起始时间(纳秒)
    int m_autoFrameBytes = 0;          ///< 自动帧内字节数
    // ── 空闲追踪 ──
    qint64 m_lastByteNs = 0;           ///< 上一字节到达时间(纳秒)
    double m_totalIdleNs = 0.0;        ///< 累计空闲时间(纳秒)
    // ── 帧间间隔 ──
    QVector<qint64> m_interFrameGapsNs;  ///< 帧间间隔列表(纳秒)
    // ── 运行统计 ──
    TimingRuntimeStats m_stats;        ///< 跨会话累计统计
};

#endif // SERIAL_TIMING_ANALYZER_H
