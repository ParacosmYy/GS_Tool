/**
 * @file BicubicInterpolator.cpp
 * @brief 双三次样条插值器实现 — 二维数据高精度插值
 */

#include "utils/interpolate2/BicubicInterpolator.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
BicubicInterpolator::BicubicInterpolator(QObject* parent)
    : QObject(parent)
    , m_splineBuilt(false)
    , m_timeSum(0.0)
{
}

/** @brief 一维三次样条插值核心(Catmull-Rom)
 *  @param yVals 已知Y值数组
 *  @param t 目标位置(0基浮点索引)
 *  @return 插值结果
 */
double BicubicInterpolator::cubicInterp1D(const QVector<double>& yVals,
                                          double t) const
{
    int n = yVals.size();
    if (n == 0) return 0.0;
    if (n == 1) return yVals[0];

    int idx = qBound(0, static_cast<int>(qFloor(t)), n - 2);
    double frac = t - static_cast<double>(idx);

    /* 取4个邻近点(边界处复制端点) */
    double p0 = (idx > 0)     ? yVals[idx - 1] : yVals[idx];
    double p1 = yVals[idx];
    double p2 = (idx + 1 < n) ? yVals[idx + 1] : yVals[idx];
    double p3 = (idx + 2 < n) ? yVals[idx + 2] : ((idx + 1 < n) ? yVals[idx + 1] : yVals[idx]);

    /* Catmull-Rom基函数 */
    double t2 = frac * frac;
    double t3 = t2 * frac;
    double a = -0.5 * t3 + t2 - 0.5 * frac;
    double b =  1.5 * t3 - 2.5 * t2 + 1.0;
    double c = -1.5 * t3 + 2.0 * t2 + 0.5 * frac;
    double d =  0.5 * t3 - 0.5 * t2;

    return a * p0 + b * p1 + c * p2 + d * p3;
}

/** @brief 计算一维三次样条系数(自然边界)
 *  @param yVals 已知Y值数组
 *  @return 样条系数 {a, b, c, d} 每段4个
 */
QVector<double> BicubicInterpolator::computeSplineCoeffs(
    const QVector<double>& yVals) const
{
    int n = yVals.size();
    if (n < 2) return QVector<double>();

    int segs = n - 1;
    QVector<double> coeffs(4 * segs, 0.0);

    if (segs == 1) {
        coeffs[0] = yVals[0];
        coeffs[1] = yVals[1] - yVals[0];
        return coeffs;
    }

    /* 三对角方程求解自然样条 */
    QVector<double> h(segs, 1.0);  /* 等距网格, h=1 */
    QVector<double> alpha(segs, 0.0);
    for (int i = 1; i < segs; ++i) {
        alpha[i] = 3.0 * (yVals[i + 1] - yVals[i])
                 - 3.0 * (yVals[i] - yVals[i - 1]);
    }

    QVector<double> l(n, 0.0), mu(n, 0.0), z(n, 0.0);
    l[0] = 1.0;
    for (int i = 1; i < segs; ++i) {
        l[i] = 2.0 * (1.0 + 1.0) - 1.0 * mu[i - 1];
        if (qAbs(l[i]) < 1e-15) l[i] = 1e-15;
        mu[i] = 1.0 / l[i];
        z[i] = (alpha[i] - 1.0 * z[i - 1]) / l[i];
    }
    l[segs] = 1.0;

    QVector<double> c(n, 0.0), b(segs, 0.0), d(segs, 0.0);
    for (int j = segs - 1; j >= 0; --j) {
        c[j] = z[j] - mu[j] * c[j + 1];
        b[j] = (yVals[j + 1] - yVals[j]) / 1.0
             - 1.0 * (c[j + 1] + 2.0 * c[j]) / 3.0;
        d[j] = (c[j + 1] - c[j]) / 3.0;
    }

    for (int i = 0; i < segs; ++i) {
        coeffs[4 * i + 0] = yVals[i];
        coeffs[4 * i + 1] = b[i];
        coeffs[4 * i + 2] = c[i];
        coeffs[4 * i + 3] = d[i];
    }
    return coeffs;
}

/** @brief 使用预计算系数进行一维插值
 *  @param coeffs 样条系数
 *  @param nPoints 原始点数
 *  @param t 目标位置
 *  @return 插值结果
 */
double BicubicInterpolator::interpWithCoeffs(const QVector<double>& coeffs,
                                             int nPoints, double t) const
{
    if (coeffs.isEmpty()) return cubicInterp1D(QVector<double>(), t);

    int segs = nPoints - 1;
    int idx = qBound(0, static_cast<int>(qFloor(t)), segs - 1);
    double dx = t - static_cast<double>(idx);
    int off = 4 * idx;

    double a = coeffs[off];
    double b = coeffs[off + 1];
    double c = coeffs[off + 2];
    double d = coeffs[off + 3];

    return a + b * dx + c * dx * dx + d * dx * dx * dx;
}

/** @brief 构建样条(预计算行样条系数)
 *  @param controlPoints 控制点网格
 */
void BicubicInterpolator::buildSpline(
    const QVector<QVector<double>>& controlPoints)
{
    QElapsedTimer timer;
    timer.start();

    m_grid = controlPoints;
    int nRows = m_grid.size();
    if (nRows == 0) {
        m_splineBuilt = false;
        return;
    }

    int nCols = m_grid[0].size();

    /* 为每行预计算样条系数 */
    m_rowCoeffs.resize(nRows);
    for (int r = 0; r < nRows; ++r) {
        if (m_grid[r].size() == nCols && nCols >= 2) {
            m_rowCoeffs[r] = computeSplineCoeffs(m_grid[r]);
        } else {
            m_rowCoeffs[r].clear();
        }
    }

    m_splineBuilt = true;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;

    emit splineBuilt(nRows, nCols);
}

/** @brief 在指定坐标处执行双三次插值
 *  @param grid 输入二维网格
 *  @param x 列方向坐标
 *  @param y 行方向坐标
 *  @return 插值结果
 */
double BicubicInterpolator::interpolate(
    const QVector<QVector<double>>& grid, double x, double y)
{
    QElapsedTimer timer;
    timer.start();

    int nRows = grid.size();
    if (nRows == 0) { return 0.0; }
    int nCols = grid[0].size();
    if (nCols == 0) { return 0.0; }

    /* 钳位坐标到有效范围 */
    double yc = qBound(0.0, y, static_cast<double>(nRows - 1));
    double xc = qBound(0.0, x, static_cast<double>(nCols - 1));

    /* 确定列方向需要的4个行索引 */
    int yIdx = qBound(0, static_cast<int>(qFloor(yc)), nRows - 2);

    /* 收集4行在列位置xc处的插值结果 */
    QVector<double> colValues(4, 0.0);
    for (int i = 0; i < 4; ++i) {
        int ri = yIdx - 1 + i;
        if (ri < 0) ri = 0;
        if (ri >= nRows) ri = nRows - 1;

        /* 对第ri行在列xc处插值 */
        if (nCols >= 4) {
            colValues[i] = cubicInterp1D(grid[ri], xc);
        } else {
            /* 点数不足时线性插值 */
            int ci = qBound(0, static_cast<int>(qFloor(xc)), nCols - 2);
            double frac = xc - static_cast<double>(ci);
            colValues[i] = grid[ri][ci] * (1.0 - frac)
                         + grid[ri][ci + 1] * frac;
        }
    }

    /* 沿列方向在yc处插值 */
    double result;
    if (nRows >= 4) {
        double localY = yc - static_cast<double>(qMax(0, yIdx - 1));
        localY = qBound(0.0, localY, static_cast<double>(
            qMin(3, nRows - 1)));
        result = cubicInterp1D(colValues, localY);
    } else {
        double frac = yc - static_cast<double>(qBound(0, yIdx, nRows - 2));
        result = colValues[1] * (1.0 - frac) + colValues[2] * frac;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalInterpolations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    emit interpolationDone(result, xc, yc);
    return result;
}

/** @brief 批量插值
 *  @param points 坐标列表
 *  @return 插值结果列表
 */
QVector<double> BicubicInterpolator::batchInterpolate(
    const QVector<QPair<double, double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> results;
    results.reserve(points.size());

    /* 若样条已构建, 使用缓存的grid */
    const auto& grid = m_splineBuilt ? m_grid : m_grid;
    if (grid.isEmpty()) {
        return results;
    }

    for (const auto& pt : points) {
        results.append(interpolate(grid, pt.first, pt.second));
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    emit batchDone(points.size());
    return results;
}

/** @brief 重置统计信息 */
void BicubicInterpolator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
