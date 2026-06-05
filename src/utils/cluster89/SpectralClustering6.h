#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谱聚类工具类
 *
 * 提供基于图拉普拉斯矩阵的谱聚类功能，支持设置簇数和
 * RBF核参数sigma，适用于非凸形状的聚类问题。
 */
class SpectralClustering6 : public QObject {
    Q_OBJECT
public:
    /// 聚类统计信息
    struct Stats {
        int totalClusterings = 0;   ///< 总聚类次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SpectralClustering6(QObject* parent = nullptr);

    /** @brief 设置聚类簇数量 */
    void setClusterCount(int count);

    /** @brief 设置RBF核参数sigma */
    void setSigma(double sigma);

    /** @brief 对输入数据集执行谱聚类 */
    void fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号，返回样本数量 */
    void clusteringCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_clusterCount = 3;
    double m_sigma = 1.0;
};
