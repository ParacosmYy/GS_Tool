/**
 * @file PcaAnalyzer.h
 * @brief PCA主成分分析器 — 降维与特征提取
 *
 * 功能: 支持PCA降维、方差解释比、主成分贡献排序，
 *       统计分析次数/平均主成分数/累计方差解释率。
 */
#ifndef PCAANALYZER_H
#define PCAANALYZER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class PcaAnalyzer
 * @brief 主成分分析，用于高维数据降维
 */
class PcaAnalyzer : public QObject {
    Q_OBJECT
public:
    /** PCA结果 */
    struct PcaResult {
        QVector<QVector<double>> components;   ///< 主成分向量
        QVector<double> eigenvalues;           ///< 特征值
        QVector<double> explainedVarianceRatio;///< 方差解释比
        double cumulativeVariance = 0.0;       ///< 累计方差解释率
        QVector<QVector<double>> projected;    ///< 投影后数据
        int selectedComponents = 0;            ///< 选择的主成分数
    };

    /** 分析统计 */
    struct Stats {
        quint64 totalAnalyses = 0;
        double  avgComponents = 0.0;
        double  avgCumulativeVariance = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit PcaAnalyzer(QObject* parent = nullptr);

    void setVarianceThreshold(double threshold);
    void setMaxComponents(int max);

    /** 执行PCA */
    PcaResult analyze(const QVector<QVector<double>>& data);

    /** 仅计算协方差矩阵 */
    QVector<QVector<double>> covarianceMatrix(const QVector<QVector<double>>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisComplete(int components, double cumulativeVariance);

private:
    double m_varianceThreshold;
    int m_maxComponents;
    Stats m_stats;
    double m_timeSum;
};

#endif // PCAANALYZER_H
