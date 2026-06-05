/**
 * @file KmpMatcher.h
 * @brief KMP字符串匹配算法 — 线性时间复杂度
 *
 * 功能: 使用Knuth-Morris-Pratt算法进行模式匹配，
 *       预处理O(m)，匹配O(n)，总复杂度O(m+n)。
 *       支持QByteArray和QVector<int>模式匹配。
 *
 * 协作: BoyerMooreMatcher / AhoCorasick(多模式)
 */
#ifndef KMPMATCHER_H
#define KMPMATCHER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief KMP字符串匹配
 */
class KmpMatcher : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;       ///< 累计搜索次数
        quint64 totalMatches = 0;        ///< 累计匹配次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit KmpMatcher(QObject* parent = nullptr);

    /** @brief 搜索所有匹配位置
     *  @param text 文本
     *  @param pattern 模式
     *  @return 匹配起始位置列表 */
    QVector<int> search(const QByteArray& text,
                        const QByteArray& pattern);

    /** @brief 搜索第一个匹配位置
     *  @param text 文本
     *  @param pattern 模式
     *  @return 匹配位置，-1表示未找到 */
    int searchFirst(const QByteArray& text,
                    const QByteArray& pattern);

    /** @brief 整数序列匹配(用于协议帧检测)
     *  @param data 数据序列
     *  @param pattern 模式序列
     *  @return 匹配位置列表 */
    QVector<int> searchIntSequence(const QVector<int>& data,
                                   const QVector<int>& pattern);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 搜索完成 @param matchCount 匹配数 */
    void searchCompleted(int matchCount);

private:
    /** @brief 构建KMP失败函数(部分匹配表)
     *  @param pattern 模式
     *  @return 失败函数数组 */
    QVector<int> buildFailureFunction(const QByteArray& pattern);

    /** @brief 整数序列失败函数 */
    QVector<int> buildFailureInt(const QVector<int>& pattern);

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // KMPMATCHER_H
