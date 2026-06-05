/**
 * @file CubicSpline2.h
 * @brief 自然三次样条插值 — 三对角方程求解 + 导数评估
 *
 * 功能: 构造自然三次样条插值函数，通过Thomas算法求解三对角方程组
 *       计算样条系数。支持一阶/二阶导数评估和积分计算。
 *
 * 协作: DataInterpolator(通用插值) / WaveformEngine(波形显示)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 自然三次样条插值引擎
 */
class CubicSpline2 : public QObject {
    Q_OBJECT

public:
    /** @brief 样条段系数: S_i(x) = a + b*(x-x_i) + c*(x-x_i)^2 + d*(x-x_i)^3 */
    struct SplineCoeffs {
        double a = 0.0;     ///< 常数项(= y_i)
        double b = 0.0;     ///< 一次项系数
        double c = 0.0;     ///< 二次项系数
        double d = 0.0;     ///< 三次项系数
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalSplinesBuilt = 0;          ///< 累计构建样条次数
        int totalPointsEvaluated = 0;       ///< 累计评估点数
        int totalKnotPoints = 0;            ///< 累计节点数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit CubicSpline2(QObject* parent = nullptr);

    /** @brief 从节点数据构建自然三次样条 @param xKnots x坐标节点(必须递增) @param yKnots y坐标节点值 @return 是否构建成功 */
    bool build(const QVector<double>& xKnots, const QVector<double>& yKnots);

    /** @brief 评估样条值 @param x 评估点 @return 插值结果(未构建返回NaN) */
    double evaluate(double x) const;

    /** @brief 批量评估样条值 @param xPoints 评估点数组 @return 插值结果数组 */
    QVector<double> evaluateBatch(const QVector<double>& xPoints) const;

    /** @brief 评估一阶导数 @param x 评估点 @return 导数值 */
    double evaluateDerivative(double x) const;

    /** @brief 评估二阶导数 @param x 评估点 @return 二阶导数值 */
    double evaluateSecondDerivative(double x) const;

    /** @brief 计算样条在[a,b]上的定积分 @param a 积分下限 @param b 积分上限 @return 积分值 */
    double integrate(double a, double b) const;

    /** @brief 获取指定段的系数 @param index 段索引 @return 系数 */
    SplineCoeffs segmentCoeffs(int index) const;

    /** @brief 获取所有段系数 @return 系数数组 */
    QVector<SplineCoeffs> allCoeffs() const;

    /** @brief 获取节点数 @return 节点数 */
    int knotCount() const;

    /** @brief 样条是否已构建 @return 是否可用 */
    bool isBuilt() const;

    /** @brief 清除样条数据 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 样条构建完成 @param knotCount 节点数 */
    void splineBuilt(int knotCount);

    /** @brief 批量评估完成 @param pointCount 评估点数 */
    void evaluationComplete(int pointCount);

private:
    /** @brief Thomas算法求解三对角方程组 @param lower 下对角线 @param main 主对角线 @param upper 上对角线 @param rhs 右端项 @return 解向量 */
    QVector<double> thomasSolve(QVector<double> lower,
                                QVector<double> main,
                                QVector<double> upper,
                                const QVector<double>& rhs) const;

    /** @brief 二分查找x所在的段索引 @param x 查询点 @return 段索引 */
    int findSegment(double x) const;

    QVector<double> m_xKnots;               ///< x节点坐标
    QVector<double> m_yKnots;               ///< y节点值
    QVector<SplineCoeffs> m_coeffs;         ///< 每段的系数
    bool m_built = false;                   ///< 是否已构建

    Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
