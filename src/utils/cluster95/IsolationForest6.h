#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 孤立森林异常检测器
 *
 * 基于随机隔离原理检测多维数据中的异常点，
 * 通过构建多棵随机树实现快速异常评分。
 */
class IsolationForest6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalTrees = 0;          ///< 已构建树总数
        int totalSamples = 0;        ///< 已处理样本数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit IsolationForest6(QObject* parent = nullptr);

    /** @brief 设置隔离树数量 */
    void setTreeCount(int count);
    /** @brief 设置子采样大小 */
    void setSampleSize(int size);
    /** @brief 用训练数据拟合模型 */
    void fit(const QVector<QVector<double>>& data);
    /** @brief 预测单个样本的异常分数 */
    double predict(const QVector<double>& sample);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 异常检测完成，返回异常点数量 */
    void detected(int anomalyCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_treeCount = 100;
    int m_sampleSize = 256;
};
