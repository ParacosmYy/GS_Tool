/**
 * @file EntropyCalculator.h
 * @brief 熵计算器 — Shannon/Renyi/条件熵/互信息
 *
 * 功能: 计算数据的多种信息熵度量，用于数据复杂度和
 *       不确定性分析，支持字节级和符号级熵计算。
 *
 * 协作: ByteFrequencyAnalyzer(频率统计) / DataQualityScorer(质量评估)
 */
#ifndef ENTROPYCALCULATOR_H
#define ENTROPYCALCULATOR_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief 熵计算器 — 多种信息熵度量
 */
class EntropyCalculator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalCalculations = 0;      ///< 累计计算次数
        double  averageEntropy = 0.0;       ///< 平均Shannon熵
        double  peakEntropy = 0.0;          ///< 峰值熵
        double  lowestEntropy = 0.0;        ///< 最低熵
        quint64 totalBytesProcessed = 0;    ///< 累计处理字节数
    };

    explicit EntropyCalculator(QObject* parent = nullptr);

    /** @brief Shannon熵(字节数据) @param data 数据 @return 熵值(bits) */
    double shannonEntropy(const QByteArray& data);

    /** @brief Shannon熵(符号序列) @param symbols 符号列表 @return 熵值 */
    double shannonEntropySymbols(const QVector<int>& symbols);

    /** @brief Renyi熵 @param data 数据 @param alpha 阶数(>0,≠1) @return 熵值 */
    double renyiEntropy(const QByteArray& data, double alpha = 2.0);

    /** @brief 条件熵 H(Y|X) @param x 序列X @param y 序列Y @return 条件熵 */
    double conditionalEntropy(const QVector<int>& x,
                              const QVector<int>& y);

    /** @brief 互信息 I(X;Y) @param x 序列X @param y 序列Y @return 互信息 */
    double mutualInformation(const QVector<int>& x,
                             const QVector<int>& y);

    /** @brief 归一化熵(0-1) @param data 数据 @return 归一化熵 */
    double normalizedEntropy(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 熵计算完成 @param value 熵值 @param type 计算类型 */
    void entropyComputed(double value, const QString& type);

private:
    QMap<int, double> computeDistribution(const QVector<int>& symbols) const;

    Stats m_stats;
    double m_entropySum;
};

#endif // ENTROPYCALCULATOR_H
