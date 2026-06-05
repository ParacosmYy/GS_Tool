/**
 * @file SpectralCluster2.h
 * @brief 谱聚类算法,基于归一化切(Normalized Cut)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谱聚类算法
 *
 * 通过拉普拉斯矩阵特征分解实现图上的聚类。
 * 支持k近邻/全连接相似度图、归一化/非归一化拉普拉斯、
 * k-way谱聚类和自动聚类数估计(特征间隙法)。
 */
class SpectralCluster2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 相似度图构建方式 */
    enum GraphMode {
        KNearest = 0,   ///< k近邻图
        Full = 1         ///< 全连接图(RBF核)
    };
    Q_ENUM(GraphMode)

    /** @brief 拉普拉斯归一化类型 */
    enum LaplacianMode {
        Unnormalized = 0,    ///< 非归一化 L = D - W
        Symmetric = 1,       ///< 对称归一化 L_sym = D^{-1/2} L D^{-1/2}
        RandomWalk = 2       ///< 随机游走 L_rw = D^{-1} L
    };
    Q_ENUM(LaplacianMode)

    /** @brief 统计信息 */
    struct Stats {
        int totalClusterings = 0;       ///< 总聚类次数
        int totalPointsProcessed = 0;   ///< 总处理点数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit SpectralCluster2(QObject* parent = nullptr);

    /**
     * @brief 执行谱聚类
     * @param data 输入数据点 (每个点为坐标向量)
     * @param k 聚类数
     * @param sigma RBF核宽度参数
     * @param graphMode 图构建方式
     * @param lapMode 拉普拉斯类型
     * @return 每个点的聚类标签(0~k-1)
     */
    QVector<int> cluster(const QVector<QVector<double>>& data, int k,
                         double sigma = 1.0,
                         GraphMode graphMode = KNearest,
                         LaplacianMode lapMode = Symmetric);

    /**
     * @brief 通过特征间隙自动估计最优聚类数
     * @param data 输入数据
     * @param maxK 最大尝试聚类数
     * @param sigma RBF核参数
     * @return 估计的最优聚类数
     */
    int estimateK(const QVector<QVector<double>>& data,
                  int maxK = 10, double sigma = 1.0);

    /**
     * @brief 计算归一化切值(NCut)
     * @param data 输入数据
     * @param labels 聚类标签
     * @param sigma RBF核参数
     * @return NCut值(越小越好)
     */
    double normalizedCut(const QVector<QVector<double>>& data,
                         const QVector<int>& labels, double sigma = 1.0);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 聚类完成信号 */
    void clusteringCompleted(int k, int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<QVector<double>> buildWeightMatrix(
        const QVector<QVector<double>>& data, double sigma,
        GraphMode mode, int knn = 7) const;
    QVector<QVector<double>> computeLaplacian(
        const QVector<QVector<double>>& W, LaplacianMode mode) const;
    QVector<QVector<double>> eigenDecompose(
        const QVector<QVector<double>>& mat, int numEigenvectors) const;
    QVector<int> kmeansEmbed(const QVector<QVector<double>>& embedded,
                             int k, int maxIter = 100) const;
    double euclideanDist(const QVector<double>& a,
                         const QVector<double>& b) const;
};
