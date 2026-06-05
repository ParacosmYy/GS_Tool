/**
 * @file EulerPath.h
 * @brief 欧拉路径/回路查找器 — Hierholzer算法
 *
 * 功能: 检测并查找有向/无向图中的欧拉路径和欧拉回路。
 *       基于Hierholzer算法，时间复杂度O(V+E)。
 *       适用于通信路径规划、PCB布线、一笔画问题。
 *
 * 协作: GraphAnalyzer(图分析) / CycleDetector(环检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QMultiMap>
#include <QPair>
#include <QElapsedTimer>

/**
 * @brief 欧拉路径/回路查找器
 */
class EulerPath : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSearches = 0;          ///< 累计搜索次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit EulerPath(QObject* parent = nullptr);

    /**
     * @brief 设置图类型(有向/无向)
     * @param directed true为有向图，false为无向图
     */
    void setDirected(bool directed);

    /**
     * @brief 添加边
     * @param from 起始顶点
     * @param to 终止顶点
     */
    void addEdge(int from, int to);

    /**
     * @brief 检查是否存在欧拉路径
     * @return 是否存在欧拉路径
     */
    bool hasEulerianPath() const;

    /**
     * @brief 检查是否存在欧拉回路
     * @return 是否存在欧拉回路
     */
    bool hasEulerianCircuit() const;

    /**
     * @brief 查找欧拉路径(顶点序列)
     * @return 欧拉路径顶点序列，空表示不存在
     */
    QVector<int> findPath();

    /**
     * @brief 查找欧拉回路(顶点序列)
     * @return 欧拉回路顶点序列，空表示不存在
     */
    QVector<int> findCircuit();

    /** @brief 清空图 */
    void clear();

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 路径搜索完成信号 @param pathLength 路径长度 @param isCircuit 是否为回路 */
    void searchCompleted(int pathLength, bool isCircuit);

private:
    /**
     * @brief Hierholzer算法核心实现
     * @param startVertex 起始顶点
     * @return 顶点序列
     */
    QVector<int> hierholzer(int startVertex);

    /**
     * @brief 获取所有顶点
     * @return 顶点列表
     */
    QVector<int> getVertices() const;

    /**
     * @brief 计算顶点的度(无向图)
     * @param v 顶点
     * @return 度数
     */
    int degree(int v) const;

    /**
     * @brief 计算顶点的入度(有向图)
     * @param v 顶点
     * @return 入度
     */
    int inDegree(int v) const;

    /**
     * @brief 计算顶点的出度(有向图)
     * @param v 顶点
     * @return 出度
     */
    int outDegree(int v) const;

    /**
     * @brief 查找欧拉路径起始顶点
     * @return 起始顶点(-1表示无法确定)
     */
    int findStartVertex() const;

    bool m_directed;                                ///< 是否为有向图
    QMultiMap<int, int> m_adjList;                   ///< 邻接表(出边)
    QMultiMap<int, int> m_inEdges;                   ///< 入边(有向图用)
    QMap<QPair<int, int>, int> m_edgeCount;          ///< 边的剩余计数(用于遍历)

    QElapsedTimer m_timer;    ///< 计时器
    double m_timeSum;          ///< 累计耗时
    Stats m_stats;
};
