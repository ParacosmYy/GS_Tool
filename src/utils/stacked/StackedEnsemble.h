/**
 * @file StackedEnsemble.h
 * @brief 堆叠集成学习 — 多模型+元学习器
 *
 * 功能: 将多个基模型的预测结果作为特征输入到元学习器，
 *       实现模型堆叠(Stacking)，提升预测性能。
 *
 * 协作: NaiveBayesClassifier(基模型) / BaggingEnsemble(Bootstrap)
 */
#ifndef STACKEDENSEMBLE_H
#define STACKEDENSEMBLE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <functional>

/**
 * @brief 堆叠集成学习器
 */
class StackedEnsemble : public QObject {
    Q_OBJECT

public:
    /** @brief 基模型预测函数类型 */
    using PredictFn = std::function<double(const QVector<double>&)>;

    /** @brief 统计 */
    struct Stats {
        quint64 totalTrainings = 0;         ///< 累计训练次数
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit StackedEnsemble(QObject* parent = nullptr);

    /**
     * @brief 添加基模型
     * @param name 模型名称
     * @param predictFn 预测函数
     */
    void addModel(const QString& name, PredictFn predictFn);

    /**
     * @brief 训练元学习器
     * @param data 特征数据
     * @param labels 标签列表
     * @return 训练误差
     */
    double train(const QVector<QVector<double>>& data,
                 const QVector<double>& labels);

    /**
     * @brief 预测样本
     * @param sample 输入样本特征
     * @return 预测值
     */
    double predict(const QVector<double>& sample);

    /** @brief 获取基模型名称列表 */
    QStringList modelNames() const { return m_models.keys(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 预测完成 @param label 预测值 */
    void predictionCompleted(double label);

private:
    /** @brief 生成元特征(基模型预测) */
    QVector<double> metaFeatures(const QVector<double>& sample) const;

    /** @brief 简单线性元学习器系数 */
    QMap<QString, PredictFn> m_models;     ///< 基模型映射
    QVector<double> m_metaWeights;         ///< 元学习器权重
    double m_metaBias = 0.0;               ///< 元学习器偏置
    mutable Stats m_stats;                          ///< 统计信息
    mutable double m_timeSum = 0.0;         ///< 累计耗时
};

#endif // STACKEDENSEMBLE_H
