/**
 * @file HierarchicalClassifier.h
 * @brief 层次分类器 — 自顶向下/自底向上聚类
 *
 * 功能: 支持凝聚(自底向上)和分裂(自顶向下)两种策略，
 *       提供单链接/全链接/平均链接/Ward链接准则，
 *       统计聚类次数/合并操作数/平均处理耗时。
 */
#ifndef HIERARCHICALCLASSIFIER_H
#define HIERARCHICALCLASSIFIER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class HierarchicalClassifier
 * @brief 层次聚类分类器，支持多种链接准则
 */
class HierarchicalClassifier : public QObject {
    Q_OBJECT
public:
    /** 聚类策略 */
    enum class Strategy {
        Agglomerative,  ///< 凝聚(自底向上)
        Divisive        ///< 分裂(自顶向下)
    };

    /** 链接准则 */
    enum class Linkage {
        Single,     ///< 单链接(最短距离)
        Complete,   ///< 全链接(最远距离)
        Average,    ///< 平均链接
        Ward        ///< Ward方差最小化
    };

    /** 合并步骤记录 */
    struct MergeStep {
        int clusterA;       ///< 合并簇A索引
        int clusterB;       ///< 合并簇B索引
        double distance;    ///< 合并距离
        int newSize;        ///< 合并后簇大小
    };

    /** 聚类结果 */
    struct Cluster {
        QVector<int> memberIndices; ///< 成员索引
        double centroid;            ///< 簇中心
        double variance;            ///< 簇内方差
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalClusterings = 0;       ///< 总聚类次数
        quint64 totalMerges = 0;            ///< 累计合并操作数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit HierarchicalClassifier(QObject* parent = nullptr);

    /** 设置参数 */
    void setTargetClusters(int k);
    void setStrategy(Strategy strategy);
    void setLinkage(Linkage linkage);
    void setDistanceThreshold(double threshold);

    /** 执行层次聚类 */
    QList<Cluster> classify(const QVector<double>& data);

    /** 获取合并历史(树状图数据) */
    QList<MergeStep> dendrogram() const;

    /** 计算轮廓系数评估聚类质量 */
    double silhouetteScore(const QVector<double>& data,
                           const QList<Cluster>& clusters) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 聚类完成信号 */
    void classificationComplete(int clusterCount, double totalTimeMs);
    /** 合并步骤信号 */
    void mergePerformed(int fromSize, int toSize, double distance);

private:
    /** 凝聚聚类主流程 */
    QList<Cluster> agglomerativeCluster(const QVector<double>& data);
    /** 分裂聚类主流程 */
    QList<Cluster> divisiveCluster(const QVector<double>& data);

    /** 计算簇间距离 */
    double clusterDistance(const QVector<double>& data,
                           const QVector<int>& a, const QVector<int>& b) const;
    /** 计算Ward距离 */
    double wardDistance(const QVector<double>& data,
                        const QVector<int>& a, const QVector<int>& b) const;

    int m_targetClusters;
    Strategy m_strategy;
    Linkage m_linkage;
    double m_distanceThreshold;
    QList<MergeStep> m_mergeHistory;
    Stats m_stats;
    double m_timeSum;
};

#endif // HIERARCHICALCLASSIFIER_H
