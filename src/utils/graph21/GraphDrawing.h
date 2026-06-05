/**
 * @file GraphDrawing.h
 * @brief 力导向图布局,基于Fruchterman-Reingold算法
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>

/**
 * @brief 力导向图绘制器
 *
 * 使用Fruchterman-Reingold算法计算图的二维布局。
 * 支持有向/无向图、可调参数(引力/斥力系数)、
 * 温度退火策略和迭代收敛检测。
 */
class GraphDrawing : public QObject
{
    Q_OBJECT

public:
    /** @brief 节点布局结果 */
    struct NodePosition {
        int id = 0;              ///< 节点编号
        double x = 0.0;         ///< x坐标
        double y = 0.0;         ///< y坐标
        double dispX = 0.0;     ///< x位移量
        double dispY = 0.0;     ///< y位移量
    };

    /** @brief 边 */
    struct Edge {
        int source = 0;         ///< 起始节点
        int target = 0;         ///< 目标节点
        double weight = 1.0;    ///< 边权重
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalLayouts = 0;           ///< 总布局计算次数
        int totalIterations = 0;        ///< 总迭代次数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit GraphDrawing(QObject* parent = nullptr);

    /**
     * @brief 执行力导向布局
     * @param edges 边列表
     * @param nodeCount 节点总数
     * @param width 画布宽度
     * @param height 画布高度
     * @param iterations 最大迭代次数
     * @return 各节点布局坐标
     */
    QVector<NodePosition> computeLayout(const QVector<Edge>& edges,
                                        int nodeCount,
                                        double width = 800.0,
                                        double height = 600.0,
                                        int iterations = 100);

    /**
     * @brief 增量式布局更新(在现有布局上微调)
     * @param currentPositions 当前布局
     * @param edges 边列表
     * @param width 画布宽度
     * @param height 画布高度
     * @param iterations 追加迭代次数
     * @return 更新后的布局
     */
    QVector<NodePosition> refineLayout(
        const QVector<NodePosition>& currentPositions,
        const QVector<Edge>& edges,
        double width, double height,
        int iterations = 30);

    /**
     * @brief 计算布局能量(总受力大小)
     * @param positions 节点位置
     * @param edges 边列表
     * @param idealDist 理想边长
     * @return 布局能量值
     */
    double computeEnergy(const QVector<NodePosition>& positions,
                         const QVector<Edge>& edges,
                         double idealDist) const;

    /**
     * @brief 自动确定理想边长
     * @param nodeCount 节点数
     * @param area 画布面积
     * @return 理想边长
     */
    double idealEdgeLength(int nodeCount, double area) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 布局迭代进度信号 */
    void iterationProgress(int iteration, double energy);

    /** @brief 布局计算完成信号 */
    void layoutCompleted(int nodeCount, double finalEnergy);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    void applyRepulsive(QVector<NodePosition>& nodes, double k) const;
    void applyAttractive(QVector<NodePosition>& nodes,
                         const QVector<Edge>& edges, double k) const;
    void applyGravity(QVector<NodePosition>& nodes,
                      double cx, double cy, double strength) const;
    double coolTemperature(double temp, double initialTemp,
                           int iteration, int maxIter) const;
};
