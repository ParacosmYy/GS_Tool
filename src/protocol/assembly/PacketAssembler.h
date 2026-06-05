/**
 * @file PacketAssembler.h
 * @brief 数据包组装器 -- 从碎片化串口数据组装完整数据包
 *
 * 将通过序列号标识的碎片化串口数据重组为完整数据包。
 * 支持3种重组策略: 序列号/偏移量/时间窗口。
 * 具备重复检测、空缺检测、乱序重组、超时清理与内存防护能力。
 *
 * 协作: SerialConnection(数据源)/PacketReassembler(帧边界重组,上游)/ProtocolEngine(下游消费)
 */

#ifndef PACKET_ASSEMBLER_H
#define PACKET_ASSEMBLER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QMap>
#include <QSet>
#include <QList>
#include <QPair>
#include <QTimer>

/**
 * @brief 数据包组装器
 *
 * 嵌入式协议常将一个逻辑数据包拆分为多个碎片分批传输，本组装器负责:
 *   1. 按序列号/偏移量/时间窗口追踪分片归属
 *   2. 检测并丢弃重复分片
 *   3. 发现序列中的空缺并报告
 *   4. 支持乱序到达的分片重组
 *   5. 超时未完成的包自动清理
 *   6. 缓冲区上限防护，防止内存溢出
 */
class PacketAssembler : public QObject {
    Q_OBJECT

public:
    /** @brief 重组策略 */
    enum class ReassemblyStrategy {
        SequenceNumber,  ///< 按序列号重组: 分片携带顺序编号(0,1,2...)
        Offset,          ///< 按偏移量重组: 分片携带字节偏移(0,128,256...)
        TimestampWindow  ///< 按时间窗口重组: 在配置的时间窗口内收集的分片归为同一包
    };
    Q_ENUM(ReassemblyStrategy)

    /** @brief 组装器配置 */
    struct AssemblerConfig {
        ReassemblyStrategy strategy = ReassemblyStrategy::SequenceNumber; ///< 重组策略
        int fragmentTimeoutMs = 2000;    ///< 分片超时阈值(ms, 0=禁用超时)
        int maxBufferSize = 1048576;     ///< 最大缓冲区大小(字节, 防内存溢出)
        int maxConcurrentPackets = 64;   ///< 最大并发包数(防止大量未完成包占满内存)
        int timestampWindowMs = 500;     ///< 时间窗口大小(TimestampWindow策略, ms)
        bool dropOnGap = false;          ///< 检测到空缺时是否立即丢弃该包
    };

    /** @brief 未完成包的进度信息 */
    struct IncompletePacket {
        int packetId = 0;               ///< 包标识(首个分片的序列号)
        int totalFragments = 0;         ///< 期望总分片数
        int receivedFragments = 0;      ///< 已接收分片数
        qint64 firstFragmentTime = 0;   ///< 首分片到达时间(epoch ms)
        qint64 lastFragmentTime = 0;    ///< 最近分片到达时间(epoch ms)
        int bufferSize = 0;             ///< 已缓冲数据大小(字节)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFragmentsReceived = 0;  ///< 累计接收分片数
        quint64 totalPacketsAssembled = 0;   ///< 累计组装完成包数
        quint64 totalFragmentsDropped = 0;   ///< 累计丢弃分片数(重复/缓冲区满)
        quint64 totalTimeouts = 0;           ///< 累计超时丢弃包数
        quint64 totalGaps = 0;               ///< 累计检测到的序列空缺数
        double avgAssemblyTimeMs = 0.0;      ///< 平均组装耗时(ms)
        double bufferUtilization = 0.0;      ///< 缓冲区利用率(0.0~1.0)
    };

    //-- 构造/析构 --//
    explicit PacketAssembler(QObject* parent = nullptr); ///< 构造(初始化默认配置与超时定时器)
    ~PacketAssembler() override;                         ///< 析构(停止定时器)

    //-- 配置接口 --//
    void setConfig(const AssemblerConfig& config); ///< 设置组装器配置(验证后应用)
    const AssemblerConfig& config() const;          ///< 获取当前配置

    //-- 数据输入 --//
    /**
     * @brief 喂入一个分片
     * @param fragment 分片数据
     * @param sequenceNumber 分片序号(从0开始)
     * @param totalFragments 该包的总分片数
     *
     * 首个分片(seq=0)创建新的组装上下文，后续分片追加到对应包中。
     * 重复分片被静默丢弃。分片可以乱序到达。
     */
    void feed(const QByteArray& fragment, int sequenceNumber, int totalFragments);

    //-- 结果查询 --//
    /** @brief 取出所有已组装完成的数据包并清空完成队列 @return 完整数据包列表 */
    QList<QByteArray> getCompletePackets();
    /** @brief 获取当前未完成包的进度列表 @return 未完成包信息列表 */
    QList<IncompletePacket> getIncompletePackets() const;

    //-- 状态控制 --//
    void reset();               ///< 重置组装状态(清空所有缓冲, 不清零统计)
    bool isAssembling() const;  ///< 是否有正在组装的包
    int activePacketCount() const; ///< 当前正在组装的包数

    //-- 统计接口 --//
    const Stats& stats() const; ///< 获取运行统计
    void resetStatistics();     ///< 重置所有统计计数器

signals:
    /** @brief 数据包组装完成 @param packet 完整数据包内容 */
    void packetAssembled(const QByteArray& packet);
    /** @brief 接收到分片 @param seq 分片序列号 */
    void fragmentReceived(int seq);
    /** @brief 组装超时,未完成包被清理 @param packetId 包标识 */
    void assemblyTimeout(int packetId);

private:
    //-- 内部数据结构 --//
    /**
     * @brief 单个正在组装的包的上下文
     *
     * 追踪每个逻辑包的所有分片收集状态，包括已收到的分片、
     * 缺失的分片、组装计时等信息。
     */
    struct AssemblyContext {
        int packetId = 0;               ///< 包标识(首个分片序列号)
        int totalFragments = 0;         ///< 期望总分片数
        QMap<int, QByteArray> fragments;///< 已收到的分片(序号->数据)
        QSet<int> receivedSeqs;         ///< 已收到的序列号集合(快速查重)
        qint64 firstFragmentTime = 0;   ///< 首分片到达时间(epoch ms)
        qint64 lastFragmentTime = 0;    ///< 最近分片到达时间(epoch ms)
        QElapsedTimer assemblyTimer;    ///< 组装计时器
        int bufferSize = 0;            ///< 已缓冲数据总大小(字节)
    };

    //-- 核心处理方法 --//
    void feedBySequenceNumber(const QByteArray& fragment, int seq, int total); ///< 序列号策略处理
    void feedByOffset(const QByteArray& fragment, int seq, int total);         ///< 偏移量策略处理
    void feedByTimestampWindow(const QByteArray& fragment, int seq, int total);///< 时间窗口策略处理
    void tryCompletePacket(int packetId); ///< 检查并完成指定包的组装
    void checkForGaps(int packetId);      ///< 检测指定包的序列空缺
    void emitCompletedPacket(AssemblyContext& ctx); ///< 组装并发射完成包
    void dropPacket(int packetId, bool emitSignal); ///< 清理指定包的缓冲区

    //-- 定时器处理 --//
    void setupTimeoutTimer();  ///< 初始化超时检测定时器
    void checkTimeouts();      ///< 扫描所有未完成包并清理超时包

    //-- 缓冲区管理 --//
    bool checkBufferLimits(int additionalBytes); ///< 检查缓冲区是否超出限制
    void evictOldestPacket();                    ///< 淘汰最旧的未完成包

    //-- 统计辅助 --//
    void updateStats(); ///< 更新派生统计值(平均耗时/利用率)

    //-- 成员变量 --//
    AssemblerConfig m_config;                        ///< 组装器配置
    QMap<int, AssemblyContext> m_activePackets;       ///< 正在组装的包(packetId->上下文)
    QList<QByteArray> m_completedPackets;             ///< 已完成待取出的包
    QTimer* m_timeoutTimer = nullptr;                 ///< 超时扫描定时器
    Stats m_stats;                                    ///< 运行统计
    int m_nextPacketId = 0;                           ///< 下一个可用包ID(TimestampWindow策略)
    qint64 m_timestampWindowStart = 0;               ///< 当前时间窗口起始(epoch ms)

    //-- 累计统计辅助 --//
    quint64 m_sumAssemblyTimeMs = 0;                  ///< 组装时间总和(用于计算平均值)

    static constexpr int kDefaultMaxBufferSize = 1048576; ///< 默认最大缓冲区(1MB)
    static constexpr int kHardMaxBufferSize = 16777216;   ///< 硬性缓冲区上限(16MB)
    static constexpr int kMaxTotalFragments = 65536;      ///< 单包最大分片数
};

#endif // PACKET_ASSEMBLER_H
