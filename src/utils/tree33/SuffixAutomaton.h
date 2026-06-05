/**
 * @file SuffixAutomaton.h
 * @brief 后缀自动机 — 最长公共子串/子串计数/出现位置/在线构建
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
class SuffixAutomaton : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit SuffixAutomaton(QObject* parent = nullptr);
    void build(const QString& str);
    bool contains(const QString& substr) const;
    int longestCommonSubstring(const QString& other) const;
    int substringCount(const QString& substr) const;
    QVector<int> occurrences(const QString& substr) const;
    int distinctSubstrings() const;
    int size() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void buildComplete(int states);
    void queryComplete(const QString& query, bool found);
private:
    struct State { int len = 0; int link = -1; int occCount = 0; QMap<int,int> next; };
    QVector<State> m_states;
    int m_last = 0;
    void saExtend(int c);
    Stats m_stats; double m_timeSum = 0.0;
};
