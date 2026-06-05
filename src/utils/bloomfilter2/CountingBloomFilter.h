/**
 * @file CountingBloomFilter.h
 * @brief 计数型布隆过滤器 — 支持删除/饱和计数器
 *
 * 功能: 基于多重哈希的概率数据结构，支持add/remove/contains，
 *       饱和计数器防止溢出，误判率估计，统计操作次数/元素数/耗时。
 */
#ifndef COUNTINGBLOOMFILTER_H
#define COUNTINGBLOOMFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief 计数型布隆过滤器
 */
class CountingBloomFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalAdds = 0;         ///< 累计添加次数
        quint64 totalRemoves = 0;      ///< 累计删除次数
        quint64 totalLookups = 0;      ///< 累计查询次数
        quint64 falsePositives = 0;    ///< 累计误判次数(需外部反馈)
        int     estimatedElements = 0; ///< 估计元素数
        double  estimatedFpRate = 0.0; ///< 估计误判率
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /**
     * @brief 构造函数
     * @param expectedElements 预期元素数
     * @param falsePositiveRate 期望误判率
     * @param parent 父对象
     */
    explicit CountingBloomFilter(int expectedElements = 10000,
                                 double falsePositiveRate = 0.01,
                                 QObject* parent = nullptr);

    /** @brief 添加元素 @param value 元素值 */
    void add(int value);

    /** @brief 移除元素 @param value 元素值 @return 是否成功(计数器>0) */
    bool remove(int value);

    /** @brief 查询是否可能包含 @param value 元素值 @return true=可能包含(可能误判), false=一定不包含 */
    bool contains(int value);

    /** @brief 批量添加 @param values 元素列表 */
    void addBatch(const QVector<int>& values);

    /** @brief 清空过滤器 */
    void clear();

    /** @brief 当前估计元素数 @return 元素数 */
    int estimatedElementCount() const;

    /** @brief 当前估计误判率 @return 误判率 */
    double estimatedFalsePositiveRate() const;

    /** @brief 过滤器容量 @return 位数组大小 */
    int capacity() const;

    /** @brief 哈希函数数 @return k值 */
    int hashCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 元素添加 @param value 元素 */
    void elementAdded(int value);
    /** @brief 元素移除 @param value 元素 @param success 是否成功 */
    void elementRemoved(int value, bool success);

private:
    /** @brief 计算k个哈希位置 @param value 元素 @return 位置列表 */
    QVector<int> hashPositions(int value) const;
    /** @brief MurmurHash3变体 @param value 输入 @param seed 种子 @return 哈希值 */
    quint32 murmurHash(int value, int seed) const;
    /** @brief 计算最优参数 @param n 元素数 @param fpRate 误判率 */
    void computeOptimalParams(int n, double fpRate);

    QVector<quint8> m_counters;        ///< 饱和计数器数组(0-255)
    int m_bitSize;                      ///< 位数组大小
    int m_numHashes;                    ///< 哈希函数数
    int m_elementCount;                ///< 当前元素数估计
    static constexpr quint8 MAX_COUNTER = 255; ///< 计数器最大值(饱和)
    Stats m_stats;
    double m_timeSum;
};

#endif // COUNTINGBLOOMFILTER_H
