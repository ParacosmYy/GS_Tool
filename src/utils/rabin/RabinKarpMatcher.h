/**
 * @file RabinKarpMatcher.h
 * @brief Rabin-Karp滚动哈希匹配器 — 快速字符串搜索
 *
 * 功能: 基于多项式滚动哈希的字符串匹配，支持多模式搜索，
 *       统计搜索次数/匹配数/哈希冲突数/耗时。
 */
#ifndef RABINKARPMATCHER_H
#define RABINKARPMATCHER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QVector>

class RabinKarpMatcher : public QObject {
    Q_OBJECT
public:
    struct Match {
        int position = 0;
        QByteArray pattern;
    };

    struct Stats {
        quint64 totalSearches = 0;
        quint64 totalMatchesFound = 0;
        quint64 totalHashCollisions = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit RabinKarpMatcher(QObject* parent = nullptr);

    /** @brief 设置参数 @param base 基数 @param modulus 模数 */
    void setParameters(quint64 base = 257, quint64 modulus = 1000000007);

    /** @brief 单模式搜索 @param text 文本 @param pattern 模式 @return 匹配列表 */
    QList<Match> search(const QByteArray& text, const QByteArray& pattern);

    /** @brief 多模式搜索 @param text 文本 @param patterns 模式列表 @return 匹配列表 */
    QList<Match> searchMulti(const QByteArray& text,
                             const QVector<QByteArray>& patterns);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int matchCount);

private:
    quint64 computeHash(const QByteArray& data, int len) const;
    quint64 rollHash(quint64 oldHash, char outChar, char inChar,
                     quint64 basePow) const;

    quint64 m_base;
    quint64 m_modulus;
    Stats m_stats;
    double m_timeSum;
};

#endif // RABINKARPMATCHER_H
