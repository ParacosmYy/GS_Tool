/**
 * @file CuckooFilterV2.h
 * @brief 布谷鸟过滤器 — 有界假阳性率的集合成员查询
 *
 * 功能: 实现布谷鸟过滤器, 支持高效的插入、查询和删除操作,
 *       空间效率优于Bloom过滤器(支持删除), 假阳性率有界且可配置。
 *       使用部分键布谷鸟哈希(partial-key cuckoo hashing)实现。
 *
 * 协作: NetworkDedup(网络包去重) / CacheChecker(缓存检查) / ThreatDetector(威胁指纹匹配)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief 布谷鸟过滤器
 *
 * 数据结构: bucket数组, 每个bucket存储 b 个指纹。
 * 哈希策略: 部分键布谷鸟哈希, 指纹即部分键。
 * 插入冲突时执行踢出(kickout), 最多重试 maxKicks 次。
 * 查询: 检查两个候选bucket是否包含匹配指纹。
 * 删除: 仅删除一个匹配指纹(支持删除是相对于Bloom的核心优势)。
 */
class CuckooFilterV2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 过滤器配置 */
    struct Config {
        int bucketCount = 4096;             ///< bucket数量(建议2的幂)
        int entriesPerBucket = 4;           ///< 每个bucket的指纹槽位数
        int fingerprintBits = 8;            ///< 指纹位数(影响假阳性率)
        int maxKicks = 500;                 ///< 插入时最大踢出次数

        Config() = default;
    };

    /** @brief 过滤器容量信息 */
    struct CapacityInfo {
        int totalSlots = 0;                 ///< 总槽位数
        int usedSlots = 0;                  ///< 已用槽位数
        double loadFactor = 0.0;            ///< 负载因子
        double estimatedFPR = 0.0;          ///< 估计假阳性率
        int remainingCapacity = 0;          ///< 剩余容量
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;            ///< 累计插入次数
        int totalLookups = 0;               ///< 累计查询次数
        int totalDeletions = 0;             ///< 累计删除次数
        int totalFailedInsertions = 0;      ///< 累计插入失败次数(过滤器满)
        int totalFalsePositiveEstimates = 0; ///< 累计估计假阳性次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit CuckooFilterV2(QObject* parent = nullptr);

    /**
     * @brief 初始化过滤器
     * @param config 配置参数
     * @return true=初始化成功
     */
    bool initialize(const Config& config);

    /** @brief 初始化过滤器(使用默认配置) @return true=初始化成功 */
    bool initialize();

    /**
     * @brief 插入一个元素
     * @param data 元素数据(字节序列)
     * @return true=插入成功, false=过滤器已满
     */
    bool insert(const QByteArray& data);

    /**
     * @brief 查询元素是否可能在集合中
     * @param data 元素数据
     * @return true=可能存在(可能有假阳性), false=一定不存在
     */
    bool contains(const QByteArray& data) const;

    /**
     * @brief 删除一个元素(必须之前插入过)
     * @param data 元素数据
     * @return true=删除成功, false=未找到
     */
    bool remove(const QByteArray& data);

    /**
     * @brief 批量插入
     * @param items 元素列表
     * @return 成功插入的数量
     */
    int batchInsert(const QVector<QByteArray>& items);

    /**
     * @brief 清空过滤器(保留配置)
     */
    void clear();

    /**
     * @brief 获取容量信息
     */
    CapacityInfo capacityInfo() const;

    /**
     * @brief 获取当前配置
     */
    Config config() const;

    /**
     * @brief 估计给定配置的假阳性率上限
     * @param fingerprintBits 指纹位数
     * @param entriesPerBucket 每bucket槽位数
     * @return 假阳性率估计值
     */
    double estimateFalsePositiveRate(int fingerprintBits, int entriesPerBucket) const;

    /**
     * @brief 计算最优配置(给定目标容量和假阳性率)
     * @param targetCapacity 目标容量
     * @param targetFPR 目标假阳性率
     * @return 推荐配置
     */
    Config optimalConfig(int targetCapacity, double targetFPR = 0.01) const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief 计算元素的指纹(部分哈希)
     * @param data 元素数据
     * @return 指纹值
     */
    quint32 fingerprint(const QByteArray& data) const;

    /**
     * @brief 计算主哈希索引
     * @param data 元素数据
     * @return bucket索引
     */
    quint32 primaryIndex(const QByteArray& data) const;

    /**
     * @brief 从主索引和指纹计算备用索引
     * @param primary 主索引
     * @param fp 指纹
     * @return 备用bucket索引
     */
    quint32 alternateIndex(quint32 primary, quint32 fp) const;

    /**
     * @brief 在指定bucket中查找指纹
     * @param bucketIdx bucket索引
     * @param fp 指纹
     * @return 槽位索引, -1=未找到
     */
    int findInBucket(int bucketIdx, quint32 fp) const;

    /**
     * @brief 在指定bucket中插入指纹到空槽位
     * @param bucketIdx bucket索引
     * @param fp 指纹
     * @return true=成功
     */
    bool insertToBucket(int bucketIdx, quint32 fp);

    Config m_config;                            ///< 当前配置
    QVector<QVector<quint32>> m_buckets;        ///< bucket数组: [bucketIdx][slotIdx]
    int m_itemCount = 0;                        ///< 当前存储元素数

    Stats m_stats;
    double m_timeSum = 0.0;
};
