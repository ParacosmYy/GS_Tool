#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 均值漂移聚类工具类
 *
 * 提供均值漂移(Mean Shift)聚类功能，通过核密度估计
 * 自动发现聚类中心和簇数量，无需预设簇数。
 */
class MeanShift7 : public QObject {
    Q_OBJECT
public:
    /// 聚类统计信息
    struct Stats {
        int totalClusterings = 0;   ///< 总聚类次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MeanShift7(QObject* parent = nullptr);

    /** @brief 设置核函数带宽参数 */
    void setBandwidth(double bandwidth);

    /** @brief 设置最大迭代次数 */
    void setMaxIter(int maxIter);

    /** @brief 对输入数据集执行均值漂移聚类 */
    void fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号，返回样本数量 */
    void clusteringCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_bandwidth = 1.0;
    int m_maxIter = 100;
};
