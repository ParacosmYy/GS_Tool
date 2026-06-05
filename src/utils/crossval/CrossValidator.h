/**
 * @file CrossValidator.h
 * @brief K折交叉验证框架 — 模型评估
 *
 * 功能: 将数据集分割为K个折叠，执行交叉验证评估，
 *       支持K折和留一法(LOO)两种策略。
 *
 * 协作: NaiveBayesClassifier(分类模型) / GaussianMixture(聚类模型)
 */
#ifndef CROSSVALIDATOR_H
#define CROSSVALIDATOR_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief K折交叉验证框架
 */
class CrossValidator : public QObject {
    Q_OBJECT

public:
    /** @brief 验证结果 */
    struct ValidationResult {
        double meanScore = 0.0;            ///< 平均得分
        double stdScore = 0.0;             ///< 得分标准差
        QVector<double> foldScores;        ///< 各折得分
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalValidations = 0;      ///< 累计验证次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 可验证的模型接口(函数指针)
     * @param trainData 训练数据
     * @param trainLabels 训练标签
     * @param testData 测试数据
     * @param testLabels 测试标签
     * @return 准确率/得分
     */
    using ModelEvaluator = std::function<double(
        const QVector<QVector<double>>& trainData,
        const QVector<int>& trainLabels,
        const QVector<QVector<double>>& testData,
        const QVector<int>& testLabels)>;

    explicit CrossValidator(QObject* parent = nullptr);

    /**
     * @brief K折交叉验证
     * @param evaluator 模型评估函数
     * @param data 特征数据
     * @param labels 标签列表
     * @param k 折叠数
     * @return 验证结果
     */
    ValidationResult validate(ModelEvaluator evaluator,
                              const QVector<QVector<double>>& data,
                              const QVector<int>& labels, int k = 5);

    /**
     * @brief 留一法交叉验证
     * @param evaluator 模型评估函数
     * @param data 特征数据
     * @param labels 标签列表
     * @return 验证结果
     */
    ValidationResult leaveOneOut(ModelEvaluator evaluator,
                                const QVector<QVector<double>>& data,
                                const QVector<int>& labels);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 单折完成 @param fold 当前折索引 @param score 该折得分 */
    void foldCompleted(int fold, double score);

private:
    Stats m_stats;                   ///< 统计信息
    double m_timeSum = 0.0;          ///< 累计耗时
};

#endif // CROSSVALIDATOR_H
