/**
 * @file MetroHash.h
 * @brief MetroHash — 高性能64位整数操作哈希
 *
 * 功能: 实现MetroHash算法，使用64位整数运算实现高速哈希，
 *       支持32/64位输出，适用于实时数据流指纹场景。
 *
 * 协作: DataDeduplicator(数据去重) / DataTrigger(触发检测)
 */
#ifndef METROHASH_H
#define METROHASH_H

#include <QObject>
#include <QByteArray>

/**
 * @brief MetroHash高速哈希计算器
 */
class MetroHash : public QObject {
    Q_OBJECT

public:
    /** @brief 运行时统计 */
    struct Stats {
        quint64 totalHashes       = 0;   ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理字节数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MetroHash(QObject* parent = nullptr);

    /**
     * @brief 设置哈希种子
     * @param seed 64位种子值
     */
    void setSeed(quint64 seed = 0);

    /**
     * @brief 计算64位MetroHash
     * @param data 输入数据
     * @return 64位哈希值
     */
    quint64 hash64(const QByteArray& data);

    /**
     * @brief 计算32位MetroHash
     * @param data 输入数据
     * @return 32位哈希值
     */
    quint32 hash32(const QByteArray& data);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief MetroHash内部轮函数 */
    static quint64 metroRound(quint64 v);

    /** @brief 从字节数组读取64位小端值 */
    static quint64 readU64(const char* p);

    /** @brief 从字节数组读取32位小端值 */
    static quint32 readU32(const char* p);

    Stats   m_stats;
    quint64 m_seed;      ///< 哈希种子
    double  m_timeSumMs; ///< 累计耗时(用于计算均值)
};

#endif // METROHASH_H
