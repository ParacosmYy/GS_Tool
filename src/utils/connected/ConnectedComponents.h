/**
 * @file ConnectedComponents.h
 * @brief 连通分量标注引擎 — 二值图像连通域分析
 *
 * 功能: 对二值图像执行连通分量标注，支持4-连通和8-连通邻域，
 *       统计标注次数、连通分量数及平均处理耗时。
 *
 * 协作: DataPatternDetector(模式检测) / DataSegmentAnalyzer(区域分析)
 */
#ifndef CONNECTEDCOMPONENTS_H
#define CONNECTEDCOMPONENTS_H

#include <QObject>
#include <QVector>

/**
 * @brief 连通分量标注 — 二值图像连通域分析
 */
class ConnectedComponents : public QObject {
    Q_OBJECT

public:
    /** @brief 连通类型 */
    enum class Connectivity {
        Four,       ///< 4-连通(上下左右)
        Eight       ///< 8-连通(含对角)
    };
    Q_ENUM(Connectivity)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalLabeled = 0;           ///< 累计标注次数
        quint64 totalComponents = 0;        ///< 累计连通分量总数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ConnectedComponents(QObject* parent = nullptr);

    void setConnectivity(Connectivity conn);

    /**
     * @brief 标注连通分量
     * @param binaryImage 二值图像(0=背景, 非零=前景)
     * @return 标签图像(每个像素的连通分量标签, 0=背景)
     */
    QVector<QVector<int>> label(const QVector<QVector<int>>& binaryImage);

    /** @brief 最近一次标注的连通分量数 @return 分量数 */
    int componentCount() const { return m_componentCount; }

    /** @brief 获取各连通分量的大小 @return 大小列表 */
    QVector<int> componentSizes() const { return m_componentSizes; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 标注完成信号 @param componentCount 连通分量数 */
    void labelingCompleted(int componentCount);

private:
    /** @brief 并查集查找(路径压缩) @param x 元素 @param parent 父数组 */
    int findRoot(int x, QVector<int>& parent) const;

    /** @brief 并查集合并(按秩合并) @param a 元素a @param b 元素b @param parent 父数组 @param rank 秩数组 */
    void unionSets(int a, int b, QVector<int>& parent, QVector<int>& rank);

    Connectivity    m_connectivity;        ///< 连通类型
    int             m_componentCount;      ///< 最近一次连通分量数
    QVector<int>    m_componentSizes;      ///< 各分量大小
    double          m_timeSum;             ///< 累计耗时(ms)
    mutable Stats   m_stats;               ///< 可变统计
};

#endif // CONNECTEDCOMPONENTS_H
