/**
 * @file RationalInterp.h
 * @brief 有理插值引擎 — 重心形式 + Floater-Hormann族
 *
 * 功能: 实现基于重心形式的有理插值，采用Floater-Hormann方法
 *       保证无极点、数值稳定。支持插值计算、导数估计、节点
 *       动态添加。适用于传感器非线性校准曲线、高精度数据插值。
 *
 * 协作: DataInterpolator(插值框架) / DataScalerWidget(缩放)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 有理插值引擎 — 重心Floater-Hormann无极点插值
 */
class RationalInterp : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInterpolations = 0;           ///< 累计插值计算次数
        int totalNodesUpdated = 0;             ///< 累计节点更新次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit RationalInterp(QObject* parent = nullptr);

    /**
     * @brief 设置插值节点
     * @param xNodes x坐标数组
     * @param yNodes y坐标数组(f(x))
     */
    void setNodes(const QVector<double>& xNodes,
                  const QVector<double>& yNodes);

    /**
     * @brief 设置Floater-Hormann的d参数(0~n-1)
     * @param d 阶数(d=0为多项式插值, d=n-1为Schneider-Werner)
     */
    void setFloaterHormannD(int d);

    /**
     * @brief 计算插值点处的函数值
     * @param x 目标x值
     * @return 插值结果f(x)
     */
    double interpolate(double x) const;

    /**
     * @brief 批量插值
     * @param xPoints 目标x数组
     * @return 插值结果数组
     */
    QVector<double> interpolateBatch(const QVector<double>& xPoints) const;

    /**
     * @brief 计算插值点处的一阶导数近似
     * @param x 目标x值
     * @return 导数近似值
     */
    double derivative(double x) const;

    /**
     * @brief 添加单个节点(增量更新权重)
     * @param x x坐标
     * @param y y坐标
     */
    void addNode(double x, double y);

    /**
     * @brief 清空所有节点
     */
    void clear();

    /**
     * @brief 获取当前节点数
     * @return 节点数量
     */
    int nodeCount() const;

    /**
     * @brief 获取当前节点
     * @return (x数组, y数组)对
     */
    QPair<QVector<double>, QVector<double>> nodes() const;

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 节点更新信号
     * @param nodeCount 当前节点数
     */
    void nodesChanged(int nodeCount);

private:
    /**
     * @brief 重新计算Floater-Hormann重心权重
     */
    void recomputeWeights();

    /**
     * @brief 计算单个混合权重w_i^(d)
     * @param i 节点索引
     * @return 权重值
     */
    double computeWeight(int i) const;

    /**
     * @brief 符号函数 (-1)^k
     * @param k 整数
     * @return 1.0 或 -1.0
     */
    static double signAlternation(int k);

    QVector<double> m_xNodes;          ///< x坐标节点
    QVector<double> m_yNodes;          ///< y坐标节点
    QVector<double> m_weights;         ///< 重心权重
    int m_d;                           ///< Floater-Hormann d参数

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;    ///< 处理时间累加器
};
