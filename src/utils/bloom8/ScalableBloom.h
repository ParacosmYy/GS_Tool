/**
 * @file ScalableBloom.h
 * @brief 可扩展布隆过滤器 — 自动扩容的概率型集合查询
 *
 * 功能: 实现可扩展布隆过滤器，当填充率超过阈值时自动添加新的过滤器层，
 *       支持动态插入、存在性查询、误判率估算、容量扩展，
 *       适用于数据去重、缓存过滤、协议包检测等场景。
 *
 * 协作: DataCache(数据缓存) / PacketLossDetector(丢包检测)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief 可扩展布隆过滤器 — 分层自动扩容
 *
 * 典型用法:
 * @code
 *   ScalableBloom bloom;
 *   bloom.setInitialCapacity(10000);
 *   bloom.insert("key1");
 *   bool exists = bloom.contains("key1");
 * @endcode
 */
class ScalableBloom : public QObject {
    Q_OBJECT

public:
    /** @brief 过滤器层 */
    struct FilterLayer {
        QByteArray bitArray;            ///< 位数组
        int bitCount = 0;               ///< 位数组长度
        int hashCount = 0;              ///< 哈希函数数量
        int itemCount = 0;              ///< 已插入元素数
        int capacity = 0;               ///< 预期容量
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalInsertions = 0;                ///< 累计插入次数
        int totalQueries = 0;                   ///< 累计查询次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        int totalFalsePositives = 0;            ///< 已知误判次数
        int totalLayers = 0;                    ///< 当前层数
    };

    explicit ScalableBloom(QObject* parent = nullptr);

    /**
     * @brief 设置初始容量
     * @param capacity 第一层预期元素数量
     */
    void setInitialCapacity(int capacity);

    /**
     * @brief 设置初始误判率
     * @param rate 目标误判率(0.0~1.0)
     */
    void setFalsePositiveRate(double rate);

    /**
     * @brief 设置扩展因子(每层容量乘以此值)
     * @param factor 扩展因子(>1.0)
     */
    void setScaleFactor(double factor);

    /**
     * @brief 插入字节串
     * @param data 待插入数据
     */
    void insert(const QByteArray& data);

    /**
     * @brief 插入字符串
     * @param key 待插入键
     */
    void insert(const QString& key);

    /**
     * @brief 查询字节串是否存在
     * @param data 待查询数据
     * @return true=可能存在, false=一定不存在
     */
    bool contains(const QByteArray& data) const;

    /**
     * @brief 查询字符串是否存在
     * @param key 待查询键
     * @return true=可能存在, false=一定不存在
     */
    bool contains(const QString& key) const;

    /**
     * @brief 估算当前误判率
     * @return 误判率上界
     */
    double estimatedFalsePositiveRate() const;

    /**
     * @brief 获取总内存占用(字节)
     * @return 内存大小
     */
    qint64 memoryUsage() const;

    /**
     * @brief 获取当前层信息
     * @return 过滤器层列表
     */
    QVector<FilterLayer> layers() const;

    /** @brief 清除所有数据 */
    void clear();

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 新层添加 @param layerIndex 层索引 @param capacity 新层容量 */
    void layerAdded(int layerIndex, int capacity);

    /** @brief 数据插入 @param totalItems 总元素数 */
    void itemInserted(int totalItems);

private:
    /** @brief 哈希函数(MurmurHash3变体) */
    static quint32 murmurHash(const QByteArray& data, quint32 seed);

    /** @brief 双重哈希生成多个哈希值 */
    QVector<int> computeHashes(const QByteArray& data, int bitCount,
                                int hashCount) const;

    /** @brief 计算最优位数组大小和哈希数 */
    QPair<int, int> optimalSize(int capacity, double fpRate) const;

    /** @brief 检查是否需要扩展 */
    void checkExpand();

    /** @brief 添加新层 */
    void addLayer();

    /** @brief 获取位数组中某位 */
    static bool getBit(const QByteArray& bits, int index);

    /** @brief 设置位数组中某位 */
    static void setBit(QByteArray& bits, int index);

    QVector<FilterLayer> m_layers;     ///< 过滤器层列表
    int m_initialCapacity = 1000;      ///< 初始容量
    double m_fpRate = 0.01;            ///< 目标误判率
    double m_scaleFactor = 2.0;        ///< 扩展因子
    int m_totalItems = 0;              ///< 总插入元素数
    Stats m_stats;                     ///< 统计数据
    double m_timeSum = 0.0;           ///< 时间累加器
};
