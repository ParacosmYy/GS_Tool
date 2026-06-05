/**
 * @file RationalInterp.cpp
 * @brief 有理插值引擎实现 — 重心Floater-Hormann无极点插值
 */

#include "utils/interp10/RationalInterp.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RationalInterp::RationalInterp(QObject* parent)
    : QObject(parent)
    , m_d(0)
    , m_timeSum(0.0)
{
}

/** @brief 设置插值节点 @param xNodes x坐标 @param yNodes y坐标 */
void RationalInterp::setNodes(const QVector<double>& xNodes,
                               const QVector<double>& yNodes)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(xNodes.size(), yNodes.size());
    m_xNodes = xNodes.mid(0, n);
    m_yNodes = yNodes.mid(0, n);

    /* 自动调整d参数: 不超过n-1 */
    if (m_d >= n) {
        m_d = qMax(0, n - 1);
    }

    recomputeWeights();

    m_stats.totalNodesUpdated++;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalNodesUpdated));

    emit nodesChanged(n);
}

/** @brief 设置Floater-Hormann d参数 @param d 阶数 */
void RationalInterp::setFloaterHormannD(int d)
{
    int n = m_xNodes.size();
    m_d = qBound(0, d, qMax(0, n - 1));
    recomputeWeights();
}

/** @brief 计算插值点函数值 @param x 目标x @return 插值结果 */
double RationalInterp::interpolate(double x) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_xNodes.size();
    if (n == 0) return 0.0;

    /* 检查是否恰好落在节点上 */
    for (int i = 0; i < n; ++i) {
        if (qFuzzyCompare(x, m_xNodes[i]) || qAbs(x - m_xNodes[i]) < 1e-15) {
            return m_yNodes[i];
        }
    }

    /* 重心插值公式:
     * r(x) = (sum_i w_i * y_i / (x - x_i)) / (sum_i w_i / (x - x_i))
     */
    double numerator = 0.0;
    double denominator = 0.0;

    for (int i = 0; i < n; ++i) {
        double diff = x - m_xNodes[i];
        if (qAbs(diff) < 1e-15) continue;

        double term = m_weights[i] / diff;
        numerator += term * m_yNodes[i];
        denominator += term;
    }

    double result = (qAbs(denominator) > 1e-300) ? numerator / denominator : 0.0;

    m_stats.totalInterpolations++;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    return result;
}

/** @brief 批量插值 @param xPoints 目标x数组 @return 插值结果数组 */
QVector<double> RationalInterp::interpolateBatch(
    const QVector<double>& xPoints) const
{
    QVector<double> results;
    results.reserve(xPoints.size());
    for (const auto& x : xPoints) {
        results.append(interpolate(x));
    }
    return results;
}

/** @brief 计算插值点一阶导数近似 @param x 目标x @return 导数值 */
double RationalInterp::derivative(double x) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_xNodes.size();
    if (n < 2) return 0.0;

    /* 检查是否在节点上 */
    for (int i = 0; i < n; ++i) {
        if (qFuzzyCompare(x, m_xNodes[i])) {
            /* 使用相邻差分近似 */
            if (i > 0 && i < n - 1) {
                return (m_yNodes[i + 1] - m_yNodes[i - 1])
                       / (m_xNodes[i + 1] - m_xNodes[i - 1]);
            } else if (i == 0) {
                return (m_yNodes[1] - m_yNodes[0])
                       / (m_xNodes[1] - m_xNodes[0]);
            } else {
                return (m_yNodes[n - 1] - m_yNodes[n - 2])
                       / (m_xNodes[n - 1] - m_xNodes[n - 2]);
            }
        }
    }

    /* 数值导数: (f(x+h) - f(x-h)) / (2h) */
    double h = 1e-7;
    /* 自适应步长: 取局部节点间距的一小部分 */
    double minDist = 1e10;
    for (int i = 0; i < n - 1; ++i) {
        double dist = qAbs(m_xNodes[i + 1] - m_xNodes[i]);
        if (dist < minDist) minDist = dist;
    }
    h = qMin(h, minDist * 1e-3);

    double fp = interpolate(x + h);
    double fm = interpolate(x - h);

    m_stats.totalInterpolations += 2;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    return (fp - fm) / (2.0 * h);
}

/** @brief 添加单个节点 @param x x坐标 @param y y坐标 */
void RationalInterp::addNode(double x, double y)
{
    /* 按x排序插入(保持有序) */
    int pos = 0;
    while (pos < m_xNodes.size() && m_xNodes[pos] < x) {
        pos++;
    }

    /* 检查重复 */
    if (pos < m_xNodes.size() && qFuzzyCompare(m_xNodes[pos], x)) {
        m_yNodes[pos] = y; /* 更新已有节点 */
        return;
    }

    m_xNodes.insert(pos, x);
    m_yNodes.insert(pos, y);

    /* 自动调整d */
    if (m_d >= m_xNodes.size()) {
        m_d = qMax(0, m_xNodes.size() - 1);
    }

    recomputeWeights();

    m_stats.totalNodesUpdated++;
    emit nodesChanged(m_xNodes.size());
}

/** @brief 清空所有节点 */
void RationalInterp::clear()
{
    m_xNodes.clear();
    m_yNodes.clear();
    m_weights.clear();
    m_d = 0;
    emit nodesChanged(0);
}

/** @brief 获取节点数 @return 节点数量 */
int RationalInterp::nodeCount() const
{
    return m_xNodes.size();
}

/** @brief 获取节点 @return (x数组, y数组) */
QPair<QVector<double>, QVector<double>> RationalInterp::nodes() const
{
    return {m_xNodes, m_yNodes};
}

/** @brief 重新计算Floater-Hormann重心权重 */
void RationalInterp::recomputeWeights()
{
    int n = m_xNodes.size();
    m_weights.resize(n);
    if (n == 0) return;

    for (int i = 0; i < n; ++i) {
        m_weights[i] = computeWeight(i);
    }
}

/** @brief 计算单个Floater-Hormann权重 w_i^(d) @param i 节点索引 @return 权重 */
double RationalInterp::computeWeight(int i) const
{
    int n = m_xNodes.size();
    int d = m_d;
    if (d < 0 || d >= n) d = qMax(0, n - 1);

    /* Floater-Hormann权重公式:
     * w_i = (-1)^{i-d} * sum_{k=max(0,i-d)}^{min(i,n-d-1)} (1/prod_{j!=i, k<=j<=k+d} |x_i - x_j|)
     */
    double weight = 0.0;
    int kMin = qMax(0, i - d);
    int kMax = qMin(i, n - d - 1);

    for (int k = kMin; k <= kMax; ++k) {
        double product = 1.0;
        for (int j = k; j <= k + d; ++j) {
            if (j == i) continue;
            double diff = qAbs(m_xNodes[i] - m_xNodes[j]);
            if (diff < 1e-300) diff = 1e-300;
            product *= diff;
        }
        if (product > 1e-300) {
            weight += 1.0 / product;
        }
    }

    /* 乘以 (-1)^{i-d} */
    weight *= signAlternation(i - d);

    return weight;
}

/** @brief 符号交替 (-1)^k @param k 整数 @return 1.0或-1.0 */
double RationalInterp::signAlternation(int k)
{
    return ((k % 2) == 0) ? 1.0 : -1.0;
}

/** @brief 重置统计信息 */
void RationalInterp::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
