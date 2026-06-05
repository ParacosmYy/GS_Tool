/**
 * @file RabinKarpMulti.h
 * @brief Rabin-Karp字符串匹配 — 滚动哈希
 *
 * 功能: 使用多项式滚动哈希进行模式匹配，支持多模式同时匹配。
 *       平均O(n+m)，可扩展到多模式O(n+km)。
 *
 * 协作: KmpMatcher(单模式线性) / BoyerMooreMatcher(启发式)
 */
#ifndef RABINKARPMULTI_H
#define RABINKARPMULTI_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief Rabin-Karp滚动哈希匹配
 */
class RabinKarpMulti : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;       ///< 累计搜索次数
        quint64 totalMatches = 0;        ///< 累计匹配次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit RabinKarpMulti(QObject* parent = nullptr);

    /** @brief 搜索所有匹配位置
     *  @param text 文本
     *  @param pattern 模式
     *  @return 匹配起始位置列表 */
    QVector<int> search(const QByteArray& text,
                        const QByteArray& pattern);

    /** @brief 多模式搜索
     *  @param text 文本
     *  @param patterns 模式列表
     *  @return 每个模式的匹配位置列表 */
    QVector<QVector<int>> multiSearch(
        const QByteArray& text,
        const QVector<QByteArray>& patterns);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param matchCount 匹配数 */
    void searchCompleted(int matchCount);

private:
    /** @brief 计算模式哈希 */
    quint64 computeHash(const QByteArray& data, int start, int len) const;

    static constexpr quint64 BASE = 257;   ///< 多项式基
    static constexpr quint64 MOD = 1000000007ULL; ///< 模数

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // RABINKARPMULTI_H
