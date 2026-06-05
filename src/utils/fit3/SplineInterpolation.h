/**
 * @file SplineInterpolation.h
 * @brief 三次样条插值 — 自然/夹持边界条件
 *
 * 功能: 构建三次样条插值函数，支持自然边界(二阶导数为0)和夹持边界
 *       (指定端点一阶导数)，提供单点/批量求值和导数计算。
 *
 * 协作: DataInterpolator(通用插值) / WaveformGenerator(波形生成)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三次样条插值器
 */
class SplineInterpolation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInterpolations = 0;    ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SplineInterpolation(QObject* parent = nullptr);

    /**
     * @brief 构建样条
     * @param x 自变量数组(必须严格递增)
     * @param y 因变量数组
     * @param bc 边界条件: "natural"(自然) 或 "clamped"(夹持)
     * @return 是否构建成功
     */
    bool build(const QVector<double>& x, const QVector<double>& y,
               const QString& bc = "natural");

    /**
     * @brief 单点求值
     * @param x 求值点
     * @return 插值结果(未构建时返回0)
     */
    double evaluate(double x) const;

    /**
     * @brief 批量求值
     * @param xs 求值点数组
     * @return 插值结果数组
     */
    QVector<double> evaluateBatch(const QVector<double>& xs) const;

    /**
     * @brief 一阶导数
     * @param x 求值点
     * @return 导数值
     */
    double derivative(double x) const;

    /** @brief 是否已构建 @return 构建状态 */
    bool isBuilt() const { return m_built; }

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 样条构建完成 @param n 数据点数 @param bc 边界条件类型 */
    void splineBuilt(int n, const QString& bc);

private:
    /**
     * @brief 样条区间结构
     */
    struct SplineSegment {
        double a;  ///< 常数项 (y值本身)
        double b;  ///< 一次项系数
        double c;  ///< 二次项系数
        double d;  ///< 三次项系数
    };

    /**
     * @brief 二分查找x所在区间
     * @param x 查找值
     * @return 区间索引
     */
    int findSegment(double x) const;

    QVector<double> m_x;               ///< 自变量节点
    QVector<SplineSegment> m_segments;  ///< 样条段系数
    bool m_built;                       ///< 构建标志
    Stats m_stats;                      ///< 统计信息
    double m_timeSum;                   ///< 累计耗时
};
