/**
 * @file AhoCorasickMatcher.h
 * @brief Aho-Corasick多模式匹配器 — 同时匹配多个关键词
 *
 * 功能: 构建Trie+失败跳转表，一次扫描匹配所有模式，
 *       统计构建/匹配次数/匹配数/耗时。
 */
#ifndef AHOCORASICKMATCHER_H
#define AHOCORASICKMATCHER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QMap>

class AhoCorasickMatcher : public QObject {
    Q_OBJECT
public:
    struct Match {
        int position = 0;
        int patternIndex = 0;
        QByteArray matched;
    };

    struct Stats {
        quint64 totalBuilds = 0;
        quint64 totalSearches = 0;
        quint64 totalMatchesFound = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit AhoCorasickMatcher(QObject* parent = nullptr);

    /** @brief 添加模式 @param pattern 模式 */
    void addPattern(const QByteArray& pattern);
    void clearPatterns();

    /** @brief 构建自动机 */
    void build();

    /** @brief 搜索 @param text 文本 @return 匹配列表 */
    QList<Match> search(const QByteArray& text);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int matchCount);

private:
    struct Node {
        QMap<char, int> children;
        int fail = 0;
        QVector<int> output;
    };

    QVector<Node> m_trie;
    QVector<QByteArray> m_patterns;
    bool m_built;
    Stats m_stats;
    double m_timeSum;
};

#endif // AHOCORASICKMATCHER_H
