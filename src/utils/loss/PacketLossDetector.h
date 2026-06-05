/**
 * @file PacketLossDetector.h
 * @brief 丢包检测器 -- 基于序列号追踪检测串口数据流中的丢包、乱序和重复
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 通过比较期望序列号与实际接收序列号，检测丢包(Gap)、
 * 乱序(Out-of-Order)和重复(Duplicate)事件。
 * 支持 8/16/32 位序列号宽度，自动处理回绕。
 * 协作: FrameParser(帧解析) / ProtocolEngine(协议引擎) / DataLogger(日志记录)
 */
#ifndef PACKETLOSSDETECTOR_H
#define PACKETLOSSDETECTOR_H

#include <QList>
#include <QObject>

/**
 * @class PacketLossDetector
 * @brief 丢包检测器，追踪序列号连续性并报告异常
 *
 * 核心设计：
 * - 8/16/32 位序列号宽度可配置，自动检测回绕
 * - Gap: received > expected，中间差值为丢失包数
 * - Out-of-Order: received < expected 且非回绕
 * - Duplicate: received == 上一次值
 * - 运行时丢包率(丢包数 / 已接收总数)
 * - CSV 导出完整 Gap 历史记录
 */
class PacketLossDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 序列号位宽 */
    enum class SequenceWidth {
        Width8  = 8,   ///< 8 位序列号(0~255)
        Width16 = 16,  ///< 16 位序列号(0~65535)
        Width32 = 32   ///< 32 位序列号(0~4294967295)
    };
    Q_ENUM(SequenceWidth)

    /** @brief 单次 Gap 事件信息 */
    struct GapInfo {
        quint64 expectedSeq = 0;  ///< 期望的序列号
        quint64 receivedSeq = 0;  ///< 实际接收的序列号
        quint64 gapSize     = 0;  ///< 丢失包数量(gap 大小)
        qint64  timestamp   = 0;  ///< Gap 发生时刻的时间戳(epoch 毫秒)
    };

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalPacketsReceived  = 0; ///< 总接收包数
        quint64 totalGapsDetected     = 0; ///< 检测到的 Gap 总次数
        quint64 totalPacketsLost      = 0; ///< 总丢失包数(所有 Gap 的 gapSize 之和)
        quint64 totalOutOfOrder       = 0; ///< 乱序包总数
        quint64 totalDuplicates       = 0; ///< 重复包总数
        quint64 maxConsecutiveLoss    = 0; ///< 最大连续丢包数
        quint64 currentConsecutiveLoss = 0;///< 当前连续丢包计数(用于实时追踪)
        double  lossRate              = 0.0; ///< 丢包率(丢失/接收)
        quint64 sequenceRollovers     = 0; ///< 序列号回绕次数
    };

    /** @brief 构造丢包检测器 @param parent 父对象 */
    explicit PacketLossDetector(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~PacketLossDetector() override;

    // ── 配置接口 ──

    /**
     * @brief 设置序列号位宽
     *
     * 切换位宽会自动重置检测状态(期望序列号清零)。
     * 不影响已有的 Gap 历史和统计信息。
     * @param width 序列号位宽枚举(8/16/32)
     */
    void setSequenceWidth(SequenceWidth width);

    /** @brief 获取当前序列号位宽 @return 位宽枚举 */
    SequenceWidth sequenceWidth() const { return m_width; }

    // ── 数据处理接口 ──

    /**
     * @brief 处理一个收到的序列号
     *
     * 核心检测逻辑入口。首次调用初始化期望值，后续调用执行检测：
     * - received == expected: 正常，expected++
     * - received > expected: Gap，发射 gapDetected
     * - received < expected(非回绕): 乱序，发射 outOfOrder
     * - received == m_lastReceived: 重复，发射 duplicateDetected
     * @param seqNum 收到的序列号(原始值，0~2^width-1)
     */
    void processPacket(quint64 seqNum);

    /**
     * @brief 完全重置检测器
     * 清空 Gap 历史、统计信息和期望序列号，恢复到初始状态。
     */
    void reset();

    // ── 查询接口 ──

    /** @brief 获取所有 Gap 事件历史 @return GapInfo 列表(时间升序) */
    QList<GapInfo> gaps() const { return m_gaps; }

    /** @brief 获取当前丢包率 @return 丢包率(0.0~1.0) */
    double lossRate() const { return m_stats.lossRate; }

    /** @brief 获取统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有统计计数器(不影响期望序列号和 Gap 历史) */
    void resetStatistics();

    // ── 导出接口 ──

    /**
     * @brief 导出 Gap 历史到 CSV 文件
     * CSV 列: Timestamp, Expected_Seq, Received_Seq, Gap_Size
     * @param filePath 目标文件路径
     * @return true 导出成功，false 文件打开失败
     */
    bool exportReport(const QString &filePath) const;

signals:
    /** @brief Gap(丢包)事件信号 @param gap Gap 详细信息 */
    void gapDetected(const PacketLossDetector::GapInfo &gap);

    /** @brief 乱序包信号 @param receivedSeq 实际接收的序列号 @param expectedSeq 期望的序列号 */
    void outOfOrder(quint64 receivedSeq, quint64 expectedSeq);

    /** @brief 重复包信号 @param seqNum 重复的序列号 */
    void duplicateDetected(quint64 seqNum);

private:
    /**
     * @brief 判断是否为序列号回绕
     * 当 received 远小于 expected 且差值超过位宽一半时判定为回绕。
     * @param received 收到的序列号 @param expected 期望的序列号
     * @return true 判定为回绕
     */
    bool isRollover(quint64 received, quint64 expected) const;

    /**
     * @brief 计算回绕场景下的 Gap 大小: (maxSeq - expected + 1) + received
     * @param received 收到的序列号 @param expected 期望的序列号
     * @return Gap 大小(丢失包数)
     */
    quint64 rolloverGapSize(quint64 received, quint64 expected) const;

    /** @brief 更新运行时丢包率: lossRate = totalPacketsLost / totalPacketsReceived */
    void updateLossRate();

    // ── 配置 ──
    SequenceWidth m_width = SequenceWidth::Width16; ///< 当前序列号位宽

    // ── 追踪状态 ──
    quint64 m_maxSeqValue  = 0;   ///< 当前位宽下的最大序列号(2^width - 1)
    quint64 m_expectedSeq  = 0;   ///< 下一个期望的序列号
    quint64 m_lastReceived = 0;   ///< 上一次收到的序列号(用于重复检测)
    bool    m_initialized  = false; ///< 是否已收到第一个包

    // ── 历史记录与统计 ──
    QList<GapInfo> m_gaps;        ///< Gap 事件历史列表
    Stats m_stats;                ///< 运行时统计信息

    static constexpr int kMaxGapsStored = 10000; ///< 最多存储的 Gap 事件数量
};

#endif // PACKETLOSSDETECTOR_H
