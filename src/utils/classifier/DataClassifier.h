/**
 * @file DataClassifier.h
 * @brief 数据分类引擎 — 阈值/K近邻/均值漂移/规则分类
 *
 * 功能: 4种分类方法，将数值数据映射到离散类别，
 *       支持自定义规则和训练数据驱动的分类。
 *
 * 协作: PatternRecognizer(模式识别) / AnomalyDetector(异常分类)
 */
#ifndef DATACLASSIFIER_H
#define DATACLASSIFIER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QString>
#include <QPair>

class DataClassifier : public QObject {
    Q_OBJECT

public:
    /** @brief 分类方法 */
    enum class ClassifyMethod {
        Threshold,      ///< 阈值分类
        KNearest,       ///< K近邻分类
        MeanShift,      ///< 均值漂移
        RuleBased       ///< 规则分类
    };
    Q_ENUM(ClassifyMethod)

    /** @brief 分类结果 */
    struct ClassResult {
        int classId = -1;           ///< 类别ID
        QString label;              ///< 类别标签
        double confidence = 0.0;    ///< 置信度[0,1]
        double distance = 0.0;      ///< 到类心距离
    };

    /** @brief 分类规则 */
    struct ClassRule {
        double minValue = 0.0;      ///< 最小值
        double maxValue = 0.0;      ///< 最大值
        int classId = 0;            ///< 类别ID
        QString label;              ///< 标签
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalClassifications = 0;   ///< 累计分类次数
        quint64 totalErrors = 0;            ///< 累计错误数
        double  peakConfidence = 0.0;       ///< 峰值置信度
        QMap<int, quint64> classCounts;     ///< 各类别计数
    };

    explicit DataClassifier(QObject* parent = nullptr);

    void setMethod(ClassifyMethod method);
    void addRule(const ClassRule& rule);
    void clearRules();
    void setTrainingData(const QVector<QPair<double, int>>& data);

    ClassResult classify(double value);
    QList<ClassResult> classifyBatch(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classified(int classId, double confidence);

private:
    ClassResult classifyThreshold(double value);
    ClassResult classifyKNearest(double value);
    ClassResult classifyMeanShift(double value);
    ClassResult classifyRuleBased(double value);

    ClassifyMethod m_method;
    QList<ClassRule> m_rules;
    QVector<QPair<double, int>> m_trainingData;
    double m_bandwidth;

    Stats m_stats;
};

#endif // DATACLASSIFIER_H
