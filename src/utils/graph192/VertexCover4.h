/**
 * @file VertexCover4.h
 * @brief 最小顶点覆盖(分支约减+冠分解+核化) — Minimum Vertex Cover via Branch-and-Reduce with Crown Decomposition and Kernelization
 *
 * 功能: 实现最小顶点覆盖算法，支持分支约减、冠分解预处理、
 *       核化技术和LP松弛下界剪枝。
 *
 * 协作: MaxFlow4(最大流) / GraphColoring4(图着色) / Matching4(匹配)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 最小顶点覆盖求解器(分支约减+冠分解)
 */
class VertexCover4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int numVertices = 0;
        int numEdges = 0;
        int coverSize = 0;
        int kernelSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VertexCover4(QObject *parent = nullptr);
    ~VertexCover4() override;

    /** @brief 设置邻接表(无向图) */
    void setGraph(const QVector<QVector<int>>& adj);

    /** @brief 求解最小顶点覆盖 */
    QVector<int> solve();

    /** @brief 获取近似下界(LP松弛) */
    int lowerBound() const;

    /** @brief 验证覆盖有效性 */
    bool verifyCover(const QVector<int>& cover) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int coverSize, int kernelSize);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;
    QVector<int> m_bestCover;
    int m_bestSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 核化: 移除度0/1顶点, 应用LP下界 */
    int kernelize(QVector<int>& cover, QVector<bool>& removed);

    /** @brief 冠分解: 找到冠结构并强制覆盖 */
    int crownDecompose(QVector<bool>& inCover, QVector<bool>& removed);

    /** @brief 分支约减主递归 */
    void branchReduce(QVector<bool> inCover, QVector<bool> removed, int currentSize);

    /** @brief 找最大度顶点 */
    int maxDegreeVertex(const QVector<bool>& removed) const;

    /** @brief 计算剩余边数 */
    int countEdges(const QVector<bool>& removed) const;
};
