/**
 * @file BayesClassifier.h
 * @brief 贝叶斯分类器 — 基于概率的数据分类
 *
 * 功能: 支持高斯朴素贝叶斯/多项式朴素贝叶斯/伯努利朴素贝叶斯，
 *       统计分类次数/准确率/各类别样本数。
 */
#ifndef BAYESCLASSIFIER_H
#define BAYESCLASSIFIER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @class BayesClassifier
 * @brief 基于贝叶斯定理的概率分类器
 */
class BayesClassifier : public QObject {
    Q_OBJECT
public:
    /** 分类器类型 */
    enum class ModelType {
        Gaussian,   ///< 高斯朴素贝叶斯(连续特征)
        Multinomial,///< 多项式朴素贝叶斯(计数特征)
        Bernoulli   ///< 伯努利朴素贝叶斯(二值特征)
    };

    /** 分类统计 */
    struct Stats {
        quint64 totalClassifications = 0;       ///< 总分类次数
        quint64 correctClassifications = 0;     ///< 正确分类数
        double  accuracy = 0.0;                 ///< 准确率
        double  averageProcessingTimeMs = 0.0;
    };

    explicit BayesClassifier(QObject* parent = nullptr);

    void setModelType(ModelType type);
    void setSmoothing(double alpha);

    /** 训练: 特征向量 → 类别标签 */
    void train(const QVector<QVector<double>>& features, const QVector<int>& labels);

    /** 预测 */
    int predict(const QVector<double>& feature);
    /** 预测概率 */
    QMap<int, double> predictProba(const QVector<double>& feature);

    /** 评估准确率 */
    double evaluate(const QVector<QVector<double>>& features, const QVector<int>& labels);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void classificationDone(int predictedClass, double confidence);
    void trainingComplete(int classCount, int sampleCount);

private:
    void trainGaussian(const QVector<QVector<double>>& features, const QVector<int>& labels);
    void trainMultinomial(const QVector<QVector<double>>& features, const QVector<int>& labels);
    void trainBernoulli(const QVector<QVector<double>>& features, const QVector<int>& labels);

    ModelType m_modelType;
    double m_smoothing;

    /* 高斯模型参数 */
    QMap<int, double> m_classPrior;                     ///< P(class)
    QMap<int, QVector<double>> m_classMeans;            ///< μ per class
    QMap<int, QVector<double>> m_classVariances;        ///< σ² per class

    /* 多项式/伯努利参数 */
    QMap<int, QVector<double>> m_classLogProbs;         ///< log P(feature|class)

    int m_featureCount;
    Stats m_stats;
    double m_timeSum;
};

#endif // BAYESCLASSIFIER_H
