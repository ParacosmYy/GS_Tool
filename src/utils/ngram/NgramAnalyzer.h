/**
 * @file NgramAnalyzer.h
 * @brief N-gram分析器 — 序列模式/频率分析
 *
 * 功能: 对字节/字符串序列进行N-gram频率统计，
 *       支持可配置N值/频率排序/概率计算。
 */
#ifndef NGRAMANALYZER_H
#define NGRAMANALYZER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QByteArray>
#include <QString>

class NgramAnalyzer : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalAnalyses = 0;
        quint64 totalNgramsExtracted = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit NgramAnalyzer(QObject* parent = nullptr);

    /** @brief 字节N-gram分析 @param data 数据 @param n N值 @return N-gram→频率映射 */
    QMap<QByteArray, int> analyzeBytes(const QByteArray& data, int n);

    /** @brief 字符串N-gram分析 @param text 文本 @param n N值 @return N-gram→频率映射 */
    QMap<QString, int> analyzeText(const QString& text, int n);

    /** @brief N-gram概率 @param data 数据 @param n N值 @return N-gram→概率映射 */
    QMap<QByteArray, double> byteProbabilities(const QByteArray& data, int n);

    /** @brief Top-K N-gram @param data 数据 @param n N值 @param k 返回数量 @return (N-gram, 频率)列表 */
    QVector<QPair<QByteArray, int>> topK(const QByteArray& data, int n, int k);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int uniqueNgrams, int totalCount);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // NGRAMANALYZER_H
