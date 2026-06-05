/**
 * @file BoyerMooreMatcher.h
 * @brief Boyer-Moore字符串匹配 — 坏字符+好后缀启发式
 *
 * 功能: 使用Boyer-Moore算法进行高效模式匹配。
 *       通过坏字符和好后缀启发式跳过不必要的比较，
 *       平均复杂度亚线性O(n/m)~O(n)。
 *
 * 协作: KmpMatcher(最坏线性) / RabinKarp(哈希匹配)
 */
#ifndef BOYERMOOREMATCHER_H
#define BOYERMOOREMATCHER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief Boyer-Moore字符串匹配
 */
class BoyerMooreMatcher : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;       ///< 累计搜索次数
        quint64 totalMatches = 0;        ///< 累计匹配次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit BoyerMooreMatcher(QObject* parent = nullptr);

    /** @brief 搜索所有匹配位置
     *  @param text 文本
     *  @param pattern 模式
     *  @return 匹配起始位置列表 */
    QVector<int> search(const QByteArray& text,
                        const QByteArray& pattern);

    /** @brief 搜索第一个匹配
     *  @param text 文本
     *  @param pattern 模式
     *  @return 匹配位置，-1表示未找到 */
    int searchFirst(const QByteArray& text,
                    const QByteArray& pattern);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param matchCount 匹配数 */
    void searchCompleted(int matchCount);

private:
    /** @brief 构建坏字符表 */
    QVector<int> buildBadCharTable(const QByteArray& pattern);

    /** @brief 构建好后缀表 */
    QVector<int> buildGoodSuffixTable(const QByteArray& pattern);

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // BOYERMOOREMATCHER_H
