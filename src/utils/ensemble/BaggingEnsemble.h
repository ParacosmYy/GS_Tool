/**
 * @file BaggingEnsemble.h
 * @brief Bagging集成学习 — Bootstrap聚合
 *
 * 功能: 使用Bootstrap采样训练多个基模型，通过投票/平均
 *       组合预测结果，降低模型方差，提高稳定性。
 *
 * 协作: StackedEnsemble(堆叠集成) / CrossValidator(验证)
 */
#ifndef BAGGINGENSEMBLE_H
#define BAGGINGENSEMBLE_H

#include <QObject>
#include <QVector>
#include <QSet>
#include <QMap>
#include <functional>

/**
 * @brief Bootstrap聚合集成学习器
 */
class BaggingEnsemble : public QObject {
    Q_OBJECT

public:
    /** @brief 基模型工厂函数类型 */
    using ModelFactory = std::function<void()>;
    /** @brief 训练函数类型 */
    using TrainFn = std::function<void(const QVector<QVector<double>>&,
                                       const QVector<int>&)>;
    /** @brief 预测函数类型 */
    using PredictFn = std::function<int(const QVector<double>&)>;

    /** @brief 统计 */
    struct Stats {
        quint64 totalTrainings = 0;         ///< 累计训练次数
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit BaggingEnsemble(QObject* parent = nullptr);

    /**
     * @brief 训练Bagging集成
     * @param data 特征数据
     * @param labels 标签列表
     * @param nModels 基模型数量
     * @return 训练准确率(OOB估计)
     */
    double train(const QVector<QVector<double>>& data,
                 const QVector<int>& labels,
                 int nModels = 10);

    /**
     * @brief 多数投票预测
     * @param sample 输入样本
     * @return 预测类别
     */
    int predict(const QVector<double>& sample);

    /**
     * @brief 计算特征重要性(置换法)
     * @return 各特征的重要性分数
     */
    QVector<double> featureImportance() const;

    /** @brief 获取模型数量 */
    int modelCount() const { return m_predictions.size(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 训练完成 @param nModels 模型数量 */
    void trainingCompleted(int nModels);

private:
    /** @brief 生成Bootstrap样本索引 */
    QVector<int> bootstrapSample(int n) const;

    QVector<QVector<int>> m_bootstrapIndices;   ///< 各模型Bootstrap索引
    QVector<QVector<int>> m_oobIndices;         ///< 各模型OOB索引
    QVector<PredictFn> m_predictions;           ///< 各模型预测函数
    QVector<QVector<double>> m_trainData;       ///< 训练数据缓存
    QVector<int> m_trainLabels;                 ///< 训练标签缓存
    int m_nFeatures = 0;                        ///< 特征数
    mutable Stats m_stats;                               ///< 统计信息
    mutable double m_timeSum = 0.0;              ///< 累计耗时
};

#endif // BAGGINGENSEMBLE_H
