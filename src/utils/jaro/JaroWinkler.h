/**
 * @file JaroWinkler.h
 * @brief Jaro-Winkler相似度 — 字符串模糊匹配
 */
#ifndef JAROWINKLER_H
#define JAROWINKLER_H

#include <QObject>
#include <QString>

class JaroWinkler : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalComputations = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit JaroWinkler(QObject* parent = nullptr);

    /** @brief Jaro相似度 @param s1 字符串1 @param s2 字符串2 @return 相似度(0~1) */
    double jaro(const QString& s1, const QString& s2);

    /** @brief Jaro-Winkler相似度 @param s1 字符串1 @param s2 字符串2 @param p 前缀权重(默认0.1) @return 相似度(0~1) */
    double jaroWinkler(const QString& s1, const QString& s2, double p = 0.1);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(double similarity);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // JAROWINKLER_H
