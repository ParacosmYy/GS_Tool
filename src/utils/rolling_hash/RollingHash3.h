/**
 * @file RollingHash3.h
 * @brief 滚动哈希 — 可配置基数和模数的高效字符串/字节流哈希
 *
 * 功能: 支持O(1)时间窗口滑动哈希计算，用于字符串匹配、
 *       重复检测、数据去重等场景。支持多哈希降低碰撞率。
 *
 * 协作: BytePatternAnalyzer(模式分析) / DataDeduplicator(数据去重)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QPair>

#include <cstdint>

/**
 * @brief 滚动哈希 — 可配置基数和模数
 */
class RollingHash3 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalRolls             = 0;   ///< 累计滚动次数
        quint64 totalHashesComputed    = 0;   ///< 累计哈希计算数
        quint64 totalCollisions        = 0;   ///< 累计碰撞次数
        double  avgProcessingTimeMs    = 0.0; ///< 平均处理时间(ms)
        quint64 totalBytesProcessed    = 0;   ///< 累计处理字节数
    };

    /**
     * @brief 构造函数
     * @param windowSize 滑动窗口大小
     * @param base 哈希基数(默认257)
     * @param modulus 哈希模数(默认1e9+7)
     * @param parent 父对象
     */
    explicit RollingHash3(int windowSize = 32,
                          quint64 base = 257,
                          quint64 modulus = 1000000007,
                          QObject* parent = nullptr);

    /**
     * @brief 初始化窗口(填充前windowSize字节)
     * @param data 初始数据(≥windowSize字节)
     * @return true=成功初始化
     */
    bool initialize(const QByteArray& data);

    /**
     * @brief 滑动窗口: 移除最旧字节，添加新字节
     * @param newByte 新进入窗口的字节
     * @return 新哈希值
     */
    quint64 roll(char newByte);

    /**
     * @brief 获取当前窗口的哈希值
     * @return 当前哈希值
     */
    quint64 currentHash() const;

    /**
     * @brief 批量计算哈希(返回所有窗口位置的哈希)
     * @param data 完整数据(≥windowSize字节)
     * @return 哈希值列表(每个窗口位置一个)
     */
    QVector<quint64> computeAll(const QByteArray& data);

    /**
     * @brief 查找所有匹配位置
     * @param data 数据
     * @param targetHash 目标哈希
     * @return 匹配位置列表(0-based)
     */
    QVector<int> findMatches(const QByteArray& data, quint64 targetHash);

    /**
     * @brief 双哈希(降低碰撞率)
     * @param data 数据
     * @return 双哈希列表 (hash1, hash2)
     */
    QVector<QPair<quint64, quint64>> computeAllDual(const QByteArray& data);

    /** @brief 窗口大小 @return 窗口大小 */
    int windowSize() const { return m_windowSize; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 滚动完成 @param hash 新哈希值 */
    void hashRolled(quint64 hash);

    /** @brief 批量计算完成 @param count 哈希数量 */
    void batchCompleted(int count);

    /** @brief 匹配找到 @param position 匹配位置 */
    void matchFound(int position);

private:
    int     m_windowSize;   ///< 窗口大小
    quint64 m_base;         ///< 基数
    quint64 m_modulus;      ///< 模数
    quint64 m_hash;         ///< 当前哈希值
    quint64 m_basePow;      ///< base^(windowSize-1) % modulus (预计算)
    int     m_count;        ///< 窗口内字节数
    bool    m_initialized;  ///< 是否已初始化

    QVector<quint8> m_buffer; ///< 环形缓冲区
    int  m_head;              ///< 缓冲区头指针

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
