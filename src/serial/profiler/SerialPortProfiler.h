/**
 * @file SerialPortProfiler.h
 * @brief 串口流量分析器 -- 通信模式分析/流量画像
 *
 * 实时分析串口通信流量特征：字节值分布(256桶直方图)、Shannon信息熵、
 * 数据包大小分布、包间到达间隔分布、突发流量检测(突发 = 间隔 < 平均间隔*0.5)、
 * 数据可压缩比估算。生成文本流量画像报告，帮助理解嵌入式通信模式。
 *
 * 协作关系:
 *   - 数据源(feedByte/feedBytes) → SerialPortProfiler → 报告/信号
 *   - markPacketBoundary() 由上层协议解析器在帧边界处调用
 */
#ifndef SERIALPORTPROFILER_H
#define SERIALPORTPROFILER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QVector>
#include <QPair>

/**
 * @brief 字节值分布统计 -- 256桶直方图 + Shannon信息熵
 */
struct ByteDistribution {
    int byteCounts[256] = {};       ///< 每个字节值(0x00~0xFF)的出现次数
    double entropy = 0.0;           ///< Shannon信息熵(0.0~8.0 bits)
    int mostCommonByte = 0;         ///< 出现频率最高的字节值
    int leastCommonByte = 0;        ///< 出现频率最低的字节值
};

/**
 * @brief 流量画像 -- 对一次profiling会话的完整统计摘要
 */
struct TrafficProfile {
    quint64 totalBytes = 0;                 ///< 分析的总字节数
    quint64 totalPackets = 0;               ///< 分析的总数据包数(由markPacketBoundary划分)
    double avgPacketSize = 0.0;             ///< 平均数据包大小(bytes)
    double avgInterArrivalMs = 0.0;         ///< 平均包间到达间隔(ms)
    double peakRateBytesPerSec = 0.0;       ///< 峰值吞吐率(bytes/s)
    qint64 profileDurationMs = 0;           ///< 分析会话持续时间(ms)
    ByteDistribution byteDist;              ///< 字节值分布
    double compressionRatio = 0.0;          ///< 数据可压缩比(0.0~1.0, 越高越可压缩)
};

/**
 * @brief 运行统计数据 -- 跨多次profiling会话的累计统计
 */
struct ProfilerStats {
    quint64 totalProfilesGenerated = 0;     ///< 累计生成的画像报告数
    quint64 totalBytesAnalyzed = 0;         ///< 累计分析的字节总数
    quint64 totalPacketsAnalyzed = 0;       ///< 累计分析的数据包总数
    quint64 totalBurstsDetected = 0;        ///< 累计检测到的突发流量次数
    quint64 totalReportsExported = 0;       ///< 累计导出报告的次数
    double peakAnalysisRate = 0.0;          ///< 历史最高分析吞吐率(bytes/s)
};

/**
 * @brief 串口流量分析器 -- 实时通信模式分析与流量画像
 *
 * 使用方式:
 *   1. startProfiling() 开始采集
 *   2. feedByte()/feedBytes() 喂入串口数据
 *   3. markPacketBoundary() 在帧/包边界处调用
 *   4. currentProfile() 随时获取当前画像
 *   5. stopProfiling() 结束采集，最终画像可用
 *   6. generateTextReport() / exportReport() 生成报告
 */
class SerialPortProfiler : public QObject {
    Q_OBJECT

public:
    /** @brief 构造串口流量分析器 @param parent 父对象 */
    explicit SerialPortProfiler(QObject* parent = nullptr);

    /** @brief 喂入单个字节，更新字节分布统计和时间追踪 @param byte 数据字节 */
    void feedByte(uint8_t byte);

    /** @brief 批量喂入字节数据 @param data 字节数组 */
    void feedBytes(const QByteArray& data);

    /** @brief 标记数据包边界，记录当前包的时间和大小 */
    void markPacketBoundary();

    /** @brief 开始流量分析采集 */
    void startProfiling();

    /** @brief 停止流量分析采集 */
    void stopProfiling();

    /** @brief 重置当前画像数据(不影响累计统计) */
    void resetProfile();

    /** @brief 查询是否正在采集 @return true 表示正在分析中 */
    bool isProfiling() const;

    /** @brief 获取当前实时流量画像 @return TrafficProfile快照 */
    TrafficProfile currentProfile() const;

    /** @brief 获取当前字节值分布统计 @return ByteDistribution快照 */
    ByteDistribution byteDistribution() const;

    /**
     * @brief 获取包间到达间隔直方图
     * @param bins 分桶数量(默认20)
     * @return QVector<(间隔ms下界, 频次)> 的分桶直方图
     */
    QVector<QPair<qint64, int>> interArrivalHistogram(int bins = 20) const;

    /**
     * @brief 获取数据包大小直方图
     * @param bins 分桶数量(默认20)
     * @return QVector<(大小下界bytes, 频次)> 的分桶直方图
     */
    QVector<QPair<int, int>> packetSizeHistogram(int bins = 20) const;

    /**
     * @brief 获取突发流量时间线
     * @return QVector<(时间戳ms, 突发字节数)>
     */
    QVector<QPair<qint64, int>> burstTimeline() const;

    /** @brief 生成文本格式的流量分析报告 @return 完整报告文本 */
    QString generateTextReport();

    /**
     * @brief 将分析报告导出到文件
     * @param filePath 目标文件路径
     * @return true 表示导出成功
     */
    bool exportReport(const QString& filePath);

    /** @brief 获取运行累计统计数据 @return Stats常量引用 */
    const ProfilerStats& stats() const;

    /** @brief 重置累计统计计数器(不影响当前画像数据) */
    void resetStatistics();

signals:
    /** @brief 画像数据更新信号(每次markPacketBoundary后发射) @param profile 当前画像 */
    void profileUpdated(const TrafficProfile& profile);

    /** @brief 检测到突发流量 @param timestamp 突发时刻(ms) @param bytes 突发字节数 */
    void burstDetected(qint64 timestamp, int bytes);

    /** @brief 报告已导出 @param filePath 导出文件路径 */
    void reportGenerated(const QString& filePath);

private:
    /** @brief 从字节分布计算Shannon信息熵 @return 熵值(0.0~8.0 bits) */
    double calculateEntropy() const;

    /** @brief 估算突发阈值 -- 平均间隔的50% @return 间隔阈值(ms) */
    double burstThreshold() const;

    ByteDistribution m_byteDist;                          ///< 字节值分布统计
    QVector<qint64> m_packetTimes;                        ///< 每个包的到达时间戳(ms)
    QVector<int> m_packetSizes;                           ///< 每个包的大小(bytes)
    QVector<QPair<qint64, int>> m_bursts;                 ///< 突发事件列表(时间, 字节数)
    qint64 m_lastByteTime = 0;                            ///< 上一字节到达时间(ms)
    qint64 m_currentPacketBytes = 0;                      ///< 当前正在累加的包字节数
    qint64 m_startTime = 0;                               ///< 分析开始时间(ms)
    bool m_profiling = false;                             ///< 是否正在采集
    QElapsedTimer m_timer;                                ///< 高精度计时器
    ProfilerStats m_stats;                                ///< 运行累计统计
};

#endif // SERIALPORTPROFILER_H
