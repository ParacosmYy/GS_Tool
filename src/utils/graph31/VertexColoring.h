/**
 * @file VertexColoring.h
 * @brief 图顶点着色 — 贪心/Welsh-Powell/DSATUR算法
 *
 * 功能: 实现图顶点着色算法，支持贪心、Welsh-Powell、DSATUR三种算法，
 *       色数上下界估计，冲突检测与修正，适用于调度问题、寄存器分配、
 *       频率分配、任务规划等图论应用。
 *
 * 协作: FlowNetwork2(网络流) / DataClassifier(分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QString>

/**
 * @brief 图顶点着色 — 贪心/Welsh-Powell/DSATUR算法
 */
class VertexColoring : public QObject {
    Q_OBJECT

public:
    /** @brief 着色算法 */
    enum Algorithm {
        Greedy = 0,         ///< 贪心着色(按顶点顺序)
        WelshPowell = 1,    ///< Welsh-Powell(按度数降序)
        DSATUR = 2          ///< DSATUR(饱和度优先)
    };
    Q_ENUM(Algorithm)

    /** @brief 着色结果 */
    struct ColoringResult {
        QVector<int> colors;              ///< 每个顶点的颜色(0-indexed, -1=未着色)
        int numColors = 0;                ///< 使用颜色数(色数)
        bool isValid = false;             ///< 是否为合法着色(无冲突)
        int totalConflicts = 0;           ///< 冲突边数(验证用)
        int iterations = 0;              ///< 算法迭代次数
    };

    /** @brief 图统计 */
    struct GraphInfo {
        int numVertices = 0;              ///< 顶点数
        int numEdges = 0;                 ///< 边数
        int maxDegree = 0;               ///< 最大度数
        double avgDegree = 0.0;           ///< 平均度数
        bool isConnected = false;         ///< 是否连通
    };

    /** @brief 色数界 */
    struct ChromaticBounds {
        int lowerBound = 1;               ///< 下界(团数或最大度+1)
        int upperBound = 1;               ///< 上界(最大度+1或Brooks定理)
        double heuristicEstimate = 0.0;   ///< 启发式估计值
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalColorings = 0;       ///< 累计着色次数
        quint64 totalVerticesProcessed = 0; ///< 累计处理顶点数
        quint64 totalEdgesChecked = 0;    ///< 累计检查边数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit VertexColoring(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~VertexColoring() override;

    // ── 图构建 ──

    /**
     * @brief 创建空图(指定顶点数)
     * @param numVertices 顶点数
     */
    void createGraph(int numVertices);

    /**
     * @brief 添加无向边
     * @param u 顶点u
     * @param v 顶点v
     */
    void addEdge(int u, int v);

    /**
     * @brief 从边列表构建图
     * @param numVertices 顶点数
     * @param edges 边列表(u,v)
     */
    void buildFromEdges(int numVertices,
                        const QVector<QPair<int, int>>& edges);

    /**
     * @brief 从邻接矩阵构建图
     * @param adjacencyMatrix 邻接矩阵(对称)
     */
    void buildFromMatrix(const QVector<QVector<int>>& adjacencyMatrix);

    /** @brief 清空图 */
    void clear();

    // ── 着色算法 ──

    /**
     * @brief 执行顶点着色
     * @param algorithm 着色算法
     * @return 着色结果
     */
    ColoringResult color(Algorithm algorithm = DSATUR);

    /**
     * @brief 比较所有算法结果
     * @return 算法名→着色结果
     */
    QMap<QString, ColoringResult> compareAll();

    // ── 验证与分析 ──

    /**
     * @brief 验证着色合法性
     * @param colors 颜色分配
     * @return 冲突边列表
     */
    QVector<QPair<int, int>> validateColoring(const QVector<int>& colors) const;

    /**
     * @brief 计算色数上下界
     * @return 色数界
     */
    ChromaticBounds chromaticBounds() const;

    /**
     * @brief 获取图信息
     * @return 图统计
     */
    GraphInfo graphInfo() const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 着色完成 @param numColors 使用颜色数 @param algorithm 算法名 */
    void coloringCompleted(int numColors, const QString& algorithm);

    /** @brief 发现冲突 @param u 顶点u @param v 顶点v @param color 冲突颜色 */
    void conflictFound(int u, int v, int color);

private:
    /** @brief 贪心着色 @param order 顶点处理顺序 @return 着色结果 */
    ColoringResult greedyColor(const QVector<int>& order);

    /** @brief Welsh-Powell排序(按度数降序) @return 顶点顺序 */
    QVector<int> welshPowellOrder() const;

    /** @brief DSATUR着色 @return 着色结果 */
    ColoringResult dsaturColor();

    /** @brief 计算顶点度数 @param v 顶点 @return 度数 */
    int degree(int v) const;

    /** @brief 查找最大团(近似贪心) @return 团大小 */
    int maxCliqueApprox() const;

    /** @brief BFS检查连通性 @return 连通分量数 */
    int countComponents() const;

    int m_numVertices;                   ///< 顶点数
    QVector<QVector<int>> m_adj;         ///< 邻接表

    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};
