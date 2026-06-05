/**
 * @file CartesianTree2.h
 * @brief 笛卡尔树 — 堆优先/中序键/RMQ via LCA/线性时间构建
 *
 * 功能: 实现笛卡尔树(Cartesian Tree)，按优先级满足堆序、
 *       按键值满足中序遍历。支持线性时间O(n)构建、
 *       基于LCA的RMQ查询、序列到树的转换等操作。
 *
 * 协作: EventTimeline(时间线) / DataSegmentAnalyzer(分段分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QStack>

/**
 * @brief 笛卡尔树 — 堆序笛卡尔树与RMQ查询
 */
class CartesianTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 树节点 */
    struct Node {
        int key = 0;            ///< 中序键(序列索引)
        double priority = 0.0;  ///< 优先级(序列值)
        int left = -1;          ///< 左子节点索引(-1=无)
        int right = -1;         ///< 右子节点索引(-1=无)
        int parent = -1;        ///< 父节点索引(-1=无)
    };

    /** @brief RMQ查询结果 */
    struct RmqResult {
        int minIndex = 0;           ///< 最小值位置
        double minValue = 0.0;     ///< 最小值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalBuilds = 0;                ///< 累计建树次数
        quint64 totalQueries = 0;               ///< 累计查询次数
        quint64 totalNodesProcessed = 0;        ///< 累计处理节点数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
        int     maxTreeDepth = 0;              ///< 最大树深度
    };

    explicit CartesianTree2(QObject* parent = nullptr);

    /** @brief 从序列线性时间构建笛卡尔树(最小堆) @param sequence 输入序列 */
    void buildMin(const QVector<double>& sequence);

    /** @brief 从序列线性时间构建笛卡尔树(最大堆) @param sequence 输入序列 */
    void buildMax(const QVector<double>& sequence);

    /** @brief RMQ查询: 区间最小值 @param left 左边界 @param right 右边界 @return 最小值位置和值 */
    RmqResult rangeMinQuery(int left, int right) const;

    /** @brief RMQ查询: 区间最大值 @param left 左边界 @param right 右边界 @return 最大值位置和值 */
    RmqResult rangeMaxQuery(int left, int right) const;

    /** @brief 计算两节点最近公共祖先 @param u 节点u @param v 节点v @return LCA节点索引 */
    int lca(int u, int v) const;

    /** @brief 获取节点信息 @param idx 节点索引 @return 节点 */
    const Node& node(int idx) const;

    /** @brief 获取根节点索引 @return 根索引(-1=空树) */
    int root() const { return m_root; }

    /** @brief 获取树大小 @return 节点数 */
    int size() const { return m_nodes.size(); }

    /** @brief 计算树深度 @return 最大深度 */
    int computeDepth() const;

    /** @brief 中序遍历 @param start 起始节点 @return 中序序列索引 */
    QVector<int> inorderTraversal(int start) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 建树完成 @param nodeCount 节点数 @param depth 树深度 */
    void buildComplete(int nodeCount, int depth);

    /** @brief 查询完成 @param left 左边界 @param right 右边界 @param resultIndex 结果位置 */
    void queryComplete(int left, int right, int resultIndex);

private:
    void buildImpl(const QVector<double>& sequence, bool isMinHeap);
    int depthHelper(int idx) const;
    void inorderHelper(int idx, QVector<int>& result) const;
    QVector<int> eulerTour() const;
    void eulerHelper(int idx, QVector<int>& euler, int depth,
                     QVector<int>& depthList) const;

    QVector<Node> m_nodes;          ///< 节点存储
    int m_root;                     ///< 根节点索引
    bool m_isMinHeap;               ///< true=最小堆, false=最大堆

    /** @brief LCA加速: 欧拉环游+深度+首次出现 */
    QVector<int> m_euler;           ///< 欧拉环游序列
    QVector<int> m_eulerDepth;      ///< 对应深度
    QVector<int> m_firstOccur;      ///< 节点首次出现位置
    bool m_lcaCacheValid;           ///< LCA缓存是否有效

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
