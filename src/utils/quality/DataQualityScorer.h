/**
 * @file DataQualityScorer.h
 * @brief 数据质量评分引擎 — 5维度评估数据流质量
 *
 * 功能: 从完整性/一致性/时效性/准确性/有效性5个维度评估数据质量，
 *       加权综合评分(0-100)，支持等级划分(A/B/C/D/F)。
 *
 * 协作: DataQualityScorer(质量评估) / EventTimeline(质量事件)
 */
#ifndef DATAQUALITYSCORER_H
#define DATAQUALITYSCORER_H

#include <QObject>
#include <QMap>
#include <QVector>

/**
 * @brief 数据质量评分引擎 — 5维度评估数据流质量
 */
class DataQualityScorer : public QObject {
    Q_OBJECT

public:
    /** @brief 质量等级 */
    enum class Grade {
        A, ///< 90-100: 优秀
        B, ///< 80-89: 良好
        C, ///< 60-79: 一般
        D, ///< 40-59: 较差
        F  ///< 0-39: 不合格
    };
    Q_ENUM(Grade)

    /** @brief 质量维度 */
    enum class Dimension {
        Completeness, ///< 完整性: 缺失数据检测
        Consistency,  ///< 一致性: 值域稳定性
        Timeliness,   ///< 时效性: 传输延迟
        Accuracy,     ///< 准确性: 偏差评估
        Validity      ///< 有效性: 格式/范围
    };
    Q_ENUM(Dimension)

    /** @brief 维度分数 */
    struct DimensionScore {
        Dimension dimension;    ///< 维度
        double score = 0.0;     ///< 分数(0-100)
        double weight = 0.0;    ///< 权重
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalEvaluations = 0;       ///< 累计评估次数
        double  averageScore = 0.0;         ///< 平均综合分
        double  peakScore = 0.0;            ///< 峰值综合分
        double  lowestScore = 100.0;        ///< 最低综合分
        QMap<int, quint64> evaluationsByGrade; ///< 各等级计数
    };

    explicit DataQualityScorer(QObject* parent = nullptr);

    /** @brief 设置维度权重 @param dimension 维度 @param weight 权重(0-1) */
    void setWeight(Dimension dimension, double weight);

    /** @brief 评估数据质量 @param data 数据 @param expectedCount 预期点数 @return 综合分(0-100) */
    double evaluate(const QVector<double>& data, int expectedCount);

    /** @brief 获取各维度分数 @return 分数列表 */
    QVector<DimensionScore> dimensionScores() const;

    /** @brief 获取等级 @param score 分数 @return 等级 */
    static Grade toGrade(double score);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 质量分数变化 @param oldScore 旧分 @param newScore 新分 */
    void qualityChanged(double oldScore, double newScore);

private:
    double scoreCompleteness(const QVector<double>& data, int expected);
    double scoreConsistency(const QVector<double>& data);
    double scoreTimeliness(const QVector<double>& data);
    double scoreAccuracy(const QVector<double>& data);
    double scoreValidity(const QVector<double>& data);

    QMap<Dimension, double> m_weights;      ///< 维度权重
    QVector<DimensionScore> m_lastScores;   ///< 最近维度分数
    double m_lastScore;                     ///< 最近综合分

    Stats m_stats;
    double m_scoreSum;                      ///< 分数累加器
};

#endif // DATAQUALITYSCORER_H
