/**
 * @file CatBoostEstimator.h
 * @brief 梯度提升决策树回归估计器 — CatBoost风格Stub
 *
 * 提供基于梯度提升决策树(GBDT)的回归估计Stub实现,
 * 支持训练与预测接口, 适用于嵌入式调试场景中的
 * 信号趋势预测和异常检测辅助。
 */
#ifndef CATBOOST_ESTIMATOR_H
#define CATBOOST_ESTIMATOR_H

#include <QObject>
#include <QVector>

/**
 * @class CatBoostEstimator
 * @brief 梯度提升决策树回归估计器(CatBoost风格)
 *
 * 典型用法:
 * @code
 *   CatBoostEstimator est;
 *   est.train(features, targets);
 *   double pred = est.predict(sample);
 * @endcode
 */
class CatBoostEstimator : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalTrains = 0;           ///< 训练操作总次数
        quint64 totalPredictions = 0;      ///< 预测操作总次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit CatBoostEstimator(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~CatBoostEstimator() override;

    // ── 模型操作 ──

    /**
     * @brief 训练梯度提升模型
     * @param X 特征矩阵, 每行一个样本
     * @param y 目标值向量
     * @return true训练成功
     */
    bool train(const QVector<QVector<double>>& X,
               const QVector<double>& y);

    /**
     * @brief 使用训练好的模型进行预测
     * @param features 单个样本的特征向量
     * @return 预测值; 未训练时返回0.0
     */
    double predict(const QVector<double>& features);

    // ── 查询 ──

    /** @brief 模型是否已训练 */
    bool isTrained() const;

    /** @brief 获取特征维度 */
    int featureCount() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 训练完成信号 @param success 是否成功 */
    void trainingCompleted(bool success);
    /** @brief 预测完成信号 @param value 预测值 */
    void predictionReady(double value);

private:
    /**
     * @brief 更新平均处理时间
     * @param elapsedMs 本次耗时(ms)
     */
    void updateAvgTime(double elapsedMs) const;

    /** @brief 特征均值(用于Stub预测) */
    QVector<double> m_featureMeans;

    /** @brief 目标均值(用于Stub预测) */
    double m_targetMean = 0.0;

    /** @brief 是否已训练 */
    bool m_trained = false;

    /** @brief 操作统计(mutable支持const方法更新) */
    mutable Stats m_stats;
};

#endif // CATBOOST_ESTIMATOR_H
