/**
 * @file RollingHash2.h
 * @brief 滚动哈希(Rabin-Karp式)增强版
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QString>

/**
 * @class RollingHash2
 * @brief 滚动哈希 — 支持双哈希和多模式匹配
 *
 * 基于Rabin指纹的双哈希滚动哈希，支持O(1)窗口滑动更新。
 * 用于字符串匹配、数据去重、内容分块(Chunking)等场景。
 */
class RollingHash2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalUpdates = 0;       /**< 总更新次数 */
        int totalMatches = 0;       /**< 总匹配次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param windowSize 窗口大小
     * @param parent 父对象
     */
    explicit RollingHash2(int windowSize = 48, QObject* parent = nullptr);

    /** @brief 初始化窗口内容 */
    void init(const QByteArray& data);

    /** @brief 滑动窗口: 移除最左字节，添加新字节 */
    void roll(char inByte);

    /** @brief 获取当前哈希值(哈希1) */
    quint64 hash1() const;

    /** @brief 获取当前哈希值(哈希2) */
    quint64 hash2() const;

    /** @brief 获取组合哈希(hash1 ^ hash2) */
    quint64 combinedHash() const;

    /** @brief 在文本中搜索模式 */
    QVector<int> search(const QByteArray& text, const QByteArray& pattern) const;

    /** @brief 基于哈希的内容分块(Content-Defined Chunking) */
    QVector<int> chunk(const QByteArray& data, int minChunk, int maxChunk, int mask) const;

    /** @brief 多模式搜索 */
    QVector<int> multiSearch(const QByteArray& text,
                              const QVector<QByteArray>& patterns) const;

    /** @brief 设置窗口大小 */
    void setWindowSize(int size);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 匹配发现信号 */
    void matchFound(int position, quint64 hashValue);

private:
    static constexpr quint64 BASE1 = 257;
    static constexpr quint64 BASE2 = 263;
    static constexpr quint64 MOD1 = (1ULL << 61) - 1;
    static constexpr quint64 MOD2 = (1ULL << 59) - 1;

    int m_windowSize;
    QByteArray m_window;
    int m_windowPos;
    quint64 m_hash1, m_hash2;
    quint64 m_basePow1, m_basePow2;

    void precomputePowers();
    quint64 modMul(quint64 a, quint64 b, quint64 mod) const;
};
