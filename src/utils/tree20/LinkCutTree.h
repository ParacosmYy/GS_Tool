/**
 * @file LinkCutTree.h
 * @brief Link-Cut树 — Splay路径+动态树连通性+路径聚合
 * 支持link/cut/findRoot/路径聚合查询(sum/min/max)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief Link-Cut树 — Splay维护preferred-path的动态树
 */
class LinkCutTree : public QObject {
    Q_OBJECT

public:
    /** @brief 聚合操作类型 */
    enum AggregateOp {
        Sum = 0,           ///< 路径求和
        Min = 1,           ///< 路径最小值
        Max = 2,           ///< 路径最大值
        Product = 3        ///< 路径乘积
    };
    Q_ENUM(AggregateOp)

    /** @brief 路径查询结果 */
    struct PathResult {
        double aggregate = 0.0;           ///< 聚合值
        int pathLength = 0;               ///< 路径长度(边数)
        QVector<int> pathVertices;        ///< 路径顶点序列
        bool valid = false;              ///< 查询是否有效
    };

    /** @brief 树边信息 */
    struct EdgeInfo {
        int from = -1;                    ///< 起点
        int to = -1;                      ///< 终点
        double weight = 0.0;              ///< 边权
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalLinkOps = 0;         ///< 累计link操作数
        quint64 totalCutOps = 0;          ///< 累计cut操作数
        quint64 totalPathQueries = 0;     ///< 累计路径查询数
        quint64 totalFindRootOps = 0;     ///< 累计findRoot操作数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param numNodes 初始节点数
     * @param parent 父对象
     */
    explicit LinkCutTree(int numNodes = 0, QObject* parent = nullptr);

    ~LinkCutTree() override;

    /** @brief 扩展节点数 */
    void resize(int numNodes);
    /** @brief 设置节点值 */
    void setNodeValue(int node, double value);
    /** @brief 获取节点值 */
    double nodeValue(int node) const;
    /** @brief Link: 将v连为u的子节点 @return 是否成功 */
    bool link(int u, int v);
    /** @brief Cut: 断开v与父节点的连边 @param v 子节点 @return 是否成功 */
    bool cut(int v);
    /** @brief 查找v所在树的根 @param v 节点编号 @return 根节点编号 */
    int findRoot(int v);
    /** @brief 查询v的父节点(表示树中) @param v 节点编号 @return 父节点(-1为根) */
    int parent(int v);

    // ── 路径查询 ──

    /** @brief 路径聚合查询(v到根) @param v 节点编号 @param op 聚合操作 @return 结果 */
    PathResult pathQuery(int v, AggregateOp op = Sum);
    /** @brief 路径聚合查询(u到v) @param u 起点 @param v 终点 @param op 聚合操作 @return 结果 */
    PathResult pathQuery(int u, int v, AggregateOp op);
    /** @brief 路径修改(v到根所有节点加delta) @param v 节点 @param delta 增量 */
    void pathUpdate(int v, double delta);
    /** @brief LCA查询 @param u 节点u @param v 节点v @return LCA编号 */
    int lca(int u, int v);

    // ── 连通性查询 ──

    /** @brief 判断u和v是否在同一棵树中 @return 是否连通 */
    bool connected(int u, int v);
    /** @brief 获取节点所在树的大小 @param v 节点编号 @return 树大小 */
    int treeSize(int v);

    // ── 统计 ──

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief Link操作 @param u 父节点 @param v 子节点 */
    void linked(int u, int v);

    /** @brief Cut操作 @param v 被断开的节点 @param formerParent 原父节点 */
    void cutted(int v, int formerParent);

    /** @brief 路径查询完成 @param from 起点 @param to 终点 @param value 聚合值 */
    void pathQueryCompleted(int from, int to, double value);

private:
    /**
     * @brief Splay树节点
     */
    struct SplayNode {
        int parent = -1;                 ///< Splay中的父节点
        int left = -1;                   ///< Splay中的左子
        int right = -1;                  ///< Splay中的右子
        bool reversed = false;           ///< 翻转标记
        double value = 0.0;              ///< 节点值
        double aggregate = 0.0;          ///< 子树聚合值
        int subtreeSize = 1;             ///< 子树大小
    };

    /** @brief 判断x是否为Splay右子 */
    bool isRightChild(int x) const;
    /** @brief 判断x是否为Splay根 */
    bool isSplayRoot(int x) const;
    /** @brief 下推翻转标记 */
    void pushDown(int x);
    /** @brief 上推聚合信息 */
    void pushUp(int x);
    /** @brief 更新聚合值 */
    void updateAggregate(int x);
    /** @brief Splay旋转 */
    void rotate(int x);
    /** @brief 将x旋转到Splay根 */
    void splay(int x);
    /** @brief Access: 将v到根路径变为preferred-path */
    int access(int v);
    /** @brief MakeRoot: 将v变为所在树的根 */
    void makeRoot(int v);

    int m_n;                             ///< 节点总数
    QVector<SplayNode> m_nodes;          ///< Splay节点数组

    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};
