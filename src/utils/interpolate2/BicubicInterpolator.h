/**
 * @file BicubicInterpolator.h
 * @brief 双三次样条插值器 — 二维数据高精度插值
 *
 * 功能: 对二维规则网格数据进行双三次样条插值, 支持
 *       任意浮点坐标的单点插值和批量插值。先对每行做
 *       一维三次样条, 再沿列方向做三次样条, 实现二维重建。
 *
 * 协作: DataInterpolator(一维插值) / DataResampler(重采样)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双三次样条插值器 — 二维数据高精度插值
 */
class BicubicInterpolator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInterpolations = 0;   ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 默认构造函数 @param parent 父对象 */
    explicit BicubicInterpolator(QObject* parent = nullptr);

    /**
     * @brief 在指定坐标处执行双三次插值
     * @param grid 输入二维网格(外层行, 内层列)
     * @param x 列方向坐标(浮点, 0基索引)
     * @param y 行方向坐标(浮点, 0基索引)
     * @return 插值结果
     *
     * grid必须为非空规整矩阵(每行长度相同, >= 4x4)。
     * 超出边界的坐标会被钳位到有效范围。
     */
    double interpolate(const QVector<QVector<double>>& grid,
                       double x, double y);

    /**
     * @brief 构建样条(预计算行/列样条系数)
     * @param controlPoints 控制点网格(外层行, 内层列)
     *
     * 预计算后可加速后续单点插值调用。
     * 若不调用此方法, interpolate()会自动按需计算。
     */
    void buildSpline(const QVector<QVector<double>>& controlPoints);

    /**
     * @brief 批量插值
     * @param points 坐标列表(第一项为x, 第二项为y)
     * @return 插值结果列表, 与points一一对应
     *
     * 依赖已构建的样条或最近一次传入的grid数据。
     */
    QVector<double> batchInterpolate(const QVector<QPair<double, double>>& points);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 插值完成 @param value 插值结果 @param x 列坐标 @param y 行坐标 */
    void interpolationDone(double value, double x, double y);

    /** @brief 批量插值完成 @param count 点数 */
    void batchDone(int count);

    /** @brief 样条构建完成 @param rows 行数 @param cols 列数 */
    void splineBuilt(int rows, int cols);

private:
    /**
     * @brief 一维三次样条插值核心
     * @param yVals 已知Y值数组
     * @param t 目标位置(0基浮点索引)
     * @return 插值结果
     */
    double cubicInterp1D(const QVector<double>& yVals, double t) const;

    /**
     * @brief 计算一维三次样条系数
     * @param yVals 已知Y值数组
     * @return 样条系数 {a, b, c, d} 每段4个
     */
    QVector<double> computeSplineCoeffs(const QVector<double>& yVals) const;

    /**
     * @brief 使用预计算系数进行一维插值
     * @param coeffs 样条系数
     * @param nPoints 原始点数
     * @param t 目标位置
     * @return 插值结果
     */
    double interpWithCoeffs(const QVector<double>& coeffs,
                            int nPoints, double t) const;

    QVector<QVector<double>> m_grid;           ///< 缓存的网格数据
    QVector<QVector<double>> m_rowCoeffs;      ///< 每行的样条系数
    QVector<QVector<double>> m_colSpline;      ///< 列方向样条缓存
    bool m_splineBuilt;                        ///< 样条是否已构建

    Stats  m_stats;                            ///< 统计信息
    double m_timeSum;                          ///< 处理时间累加器
};
