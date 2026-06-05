/**
 * @file CountingBloomFilter.h
 * @brief 计数型布隆过滤器 — 支持删除操作的概率集合
 *
 * 功能:
 *   - 标准布隆过滤器的计数扩展，每个槽位使用计数器而非比特位
 *   - 支持insert/remove/contains三要素操作
 *   - 可配置预期容量与误判率
 *   - 统计插入/删除/查询/误判次数
 *   - 自动扩容保护: 溢出检测与告警
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class CountingBloomFilter
 * @brief 计数型布隆过滤器 — 每个槽位维护计数器，支持元素删除
 *
 * 与标准布隆过滤器不同，计数布隆过滤器在每个位位置使用计数器
 * (通常4-bit)，因此支持remove操作。适合需要动态增删的场景，
 * 如网络路由表、缓存去重、重复检测等。
 */
class CountingBloomFilter : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;       /**< 总插入次数 */
        int totalRemovals = 0;         /**< 总删除次数 */
        int totalQueries = 0;          /**< 总查询次数 */
        int totalFalsePositives = 0;   /**< 估计误判次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param expectedItems 预期元素数量
     * @param falsePositiveRate 目标误判率
     * @param counterBits 每个计数器的位数(默认4位，最大计数15)
     * @param parent 父对象
     */
    explicit CountingBloomFilter(quint64 expectedItems = 10000,
                                  double falsePositiveRate = 0.01,
                                  int counterBits = 4,
                                  QObject* parent = nullptr);

    /**
     * @brief 插入元素
     * @param data 待插入数据
     */
    void insert(const QByteArray& data);

    /**
     * @brief 删除元素
     * @param data 待删除数据
     * @return 是否成功删除(若计数器为0则无法删除)
     */
    bool remove(const QByteArray& data);

    /**
     * @brief 查询元素是否存在
     * @param data 待查询数据
     * @return 可能存在(true)/一定不存在(false)
     */
    bool contains(const QByteArray& data) const;

    /**
     * @brief 清空过滤器
     */
    void clear();

    /**
     * @brief 获取当前估计误判率
     */
    double estimatedFalsePositiveRate() const;

    /**
     * @brief 获取当前元素计数
     */
    quint64 itemCount() const { return m_itemCount; }

    /**
     * @brief 获取统计信息
     */
    Stats stats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /** @brief 元素插入信号 */
    void elementInserted(quint64 totalCount);

    /** @brief 元素删除信号 */
    void elementRemoved(quint64 remainingCount);

    /** @brief 计数器溢出告警 */
    void counterOverflow(quint32 position);

private:
    /** @brief MurmurHash3变体 */
    quint32 murmurHash(const QByteArray& data, quint32 seed) const;

    /** @brief 计算第n个哈希位置 */
    quint32 hashN(const QByteArray& data, int n) const;

    /** @brief 获取指定位置的计数器值 */
    quint8 getCounter(quint64 position) const;

    /** @brief 设置指定位置的计数器值 */
    void setCounter(quint64 position, quint8 value);

    /** @brief 递增计数器，返回是否溢出 */
    bool incrementCounter(quint64 position);

    /** @brief 递减计数器，返回是否下溢 */
    bool decrementCounter(quint64 position);

    QVector<quint8> m_counters;    /**< 计数器存储(打包存储) */
    quint64 m_slotCount;           /**< 槽位总数 */
    int m_hashCount;               /**< 哈希函数数量 */
    int m_counterBits;             /**< 每个计数器位数 */
    quint64 m_itemCount;           /**< 当前元素计数 */
    mutable Stats m_stats;         /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
