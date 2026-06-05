/**
 * @file GraphSegmenter.h
 * @brief 图分割器 — 基于图的图像/数据区域分割
 *
 * 功能: 实现基于图的分割算法(Felzenszwalb-Huttenlocher)，
 *       将二维数据视为像素图，通过构建4邻域边、按权重
 *       排序、逐步合并区域实现自动分割。支持高斯预平滑
 *       和最小区域尺寸过滤。
 *
 * 协作: DbScan(密度聚类) / KdTree(近邻查询)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图分割器 — 基于图的区域分割
 */
class GraphSegmenter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSegments = 0;       ///< 累计分割次数
        double  avgRegionCount = 0.0;    ///< 平均区域数量
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param sigma 高斯预平滑标准差
     * @param threshold 合并阈值(控制区域粒度)
     * @param minSize 最小区域像素数
     * @param parent 父对象
     */
    explicit GraphSegmenter(double sigma = 0.5, double threshold = 30.0,
                              int minSize = 20, QObject* parent = nullptr);

    /**
     * @brief 执行图分割
     * @param data 输入数据矩阵(每行一个采样行)
     * @return 标签矩阵，同值表示同区域
     *
     * 算法步骤:
     * 1. 高斯预平滑输入数据
     * 2. 构建4邻域边并按权重排序
     * 3. 按Kruskal方式逐边判断是否合并
     * 4. 后处理合并过小区域
     */
    QVector<QVector<int>> segment(
        const QVector<QVector<double>>& data);

    /** @brief 设置高斯平滑参数 @param s 标准差 */
    void setSigma(double s);

    /** @brief 设置合并阈值 @param t 阈值 */
    void setThreshold(double t);

    /** @brief 设置最小区域尺寸 @param size 像素数 */
    void setMinRegionSize(int size);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分割完成 @param regionCount 区域数量 */
    void segmentCompleted(int regionCount);

private:
    /**
     * @brief 并查集 — 查找根节点(带路径压缩)
     * @param parent 父节点数组
     * @param x 待查找节点
     * @return 根节点索引
     */
    int findRoot(QVector<int>& parent, int x) const;

    /**
     * @brief 并查集 — 合并两个集合
     * @param parent 父节点数组
     * @param rank 按秩合并数组
     * @param a 节点A @param b 节点B
     */
    void unionSet(QVector<int>& parent, QVector<int>& rank,
                   int a, int b) const;

    /**
     * @brief 高斯平滑一维数据
     * @param row 输入行数据
     * @return 平滑后的行数据
     */
    QVector<double> gaussianSmooth(const QVector<double>& row) const;

    double m_sigma;      ///< 高斯平滑标准差
    double m_threshold;  ///< 合并阈值
    int    m_minSize;    ///< 最小区域尺寸
    double m_timeSum;    ///< 处理时间累加器
    double m_regionSum;  ///< 区域数量累加器
    Stats  m_stats;      ///< 统计信息
};
