/**
 * @file EulerTourTree.h
 * @brief 欧拉游览树 — Splay序列/link-cut/连通性/子树聚合
 *
 * 功能: 实现基于Splay树的欧拉游览树(ETT)，支持动态森林的
 *       link/cut操作，连通性查询，子树聚合(求和/最值/计数)，
 *       用于动态图算法和网络拓扑管理。
 *
 * 协作: DataFlowMeter(网络流) / StateTracker(状态跟踪)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief 欧拉游览树 — Splay序列/动态森林/子树聚合
 */
class EulerTourTree : public QObject {
    Q_OBJECT

public:
    /** @brief 聚合类型 */
    enum class AggregateType {
        Sum,        ///< 求和
        Min,        ///< 最小值
        Max,        ///< 最大值
        Count       ///< 计数
    };
    Q_ENUM(AggregateType)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalLinkOps = 0;           ///< 累计link操作
        quint64 totalCutOps = 0;            ///< 累计cut操作
        quint64 totalConnectivityQueries = 0;///< 累计连通性查询
        quint64 totalAggregateQueries = 0;  ///< 累计聚合查询
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     currentEdgeCount = 0;       ///< 当前边数
        int     currentComponentCount = 0;  ///< 当前连通分量数
    };

    explicit EulerTourTree(QObject* parent = nullptr);

    /** @brief 初始化森林 @param n 顶点数 */
    void init(int n);

    /** @brief 添加边(u,v) @param u 顶点u @param v 顶点v @return 是否成功 */
    bool link(int u, int v);

    /** @brief 删除边(u,v) @param u 顶点u @param v 顶点v @return 是否成功 */
    bool cut(int u, int v);

    /** @brief 查询连通性 @param u 顶点u @param v 顶点v @return 是否连通 */
    bool connected(int u, int v) const;

    /** @brief 查询子树聚合 @param root 子树根 @param type 聚合类型 @return 聚合值 */
    double queryAggregate(int root, AggregateType type) const;

    /** @brief 更新顶点值 @param v 顶点 @param value 新值 */
    void updateValue(int v, double value);

    /** @brief 获取顶点所在连通分量大小 @param v 顶点 @return 分量大小 */
    int componentSize(int v) const;

    /** @brief 获取当前边数 @return 边数 */
    int edgeCount() const { return m_edgeCount; }

    /** @brief 获取连通分量数 @return 分量数 */
    int componentCount() const;

    /** @brief 获取连通分量 @param v 代表顶点 @return 分量中所有顶点 */
    QVector<int> getComponent(int v) const;

    /** @brief 检查边是否存在 @param u 顶点 @param v 顶点 @return 是否存在 */
    bool hasEdge(int u, int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 边添加完成 @param u 顶点u @param v 顶点v */
    void edgeLinked(int u, int v);

    /** @brief 边删除完成 @param u 顶点u @param v 顶点v */
    void edgeCut(int u, int v);

    /** @brief 连通性查询完成 @param u 顶点u @param v 顶点v @param connected 是否连通 */
    void connectivityQueried(int u, int v, bool connected);

private:
    struct SplayNode {
        int parent = -1;         ///< 父节点索引
        int left = -1;           ///< 左子节点索引
        int right = -1;          ///< 右子节点索引
        int vertex = -1;         ///< 关联顶点
        int edgeId = -1;         ///< 关联边ID(-1表示顶点访问)
        double value = 0.0;      ///< 顶点值
        double aggSum = 0.0;     ///< 子树求和
        double aggMin = 0.0;     ///< 子树最小值
        double aggMax = 0.0;     ///< 子树最大值
        int subtreeSize = 1;     ///< 子树大小
        bool flipped = false;    ///< 翻转标记
    };

    void splay(int x);
    void rotate(int x);
    void pushDown(int x);
    void pushUp(int x);
    void detach(int x);
    int findRoot(int x) const;
    int getLeftmost(int x) const;
    int getRightmost(int x) const;

    void reroot(int u);
    void split(int u, int v, int& left, int& right);
    int join(int left, int right);
    int eulerIndexOf(int vertex) const;
    int edgeEulerIndex(int edgeId, bool isForward) const;

    void updateAggregates(int x);

    int m_n;                       ///< 顶点数
    int m_edgeCount;               ///< 边数
    QVector<SplayNode> m_nodes;    ///< Splay节点池
    QMap<QPair<int,int>, int> m_edgeMap; ///< 边→边ID映射

    QVector<int> m_vertexFirst;    ///< 顶点首次出现的欧拉索引
    QVector<int> m_vertexLast;     ///< 顶点最后出现的欧拉索引
    QVector<double> m_values;      ///< 顶点值

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;        ///< 处理时间累加器
};
