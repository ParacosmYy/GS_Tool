/**
 * @file NelderMead.cpp
 * @brief Nelder-Mead单纯形优化实现
 */

#include "utils/nelder_mead/NelderMead.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
NelderMead::NelderMead(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_evalCount(0)
{
}

/** @brief 执行Nelder-Mead优化 */
QVector<double> NelderMead::minimize(const ObjFunc& f,
                                     const QVector<double>& x0,
                                     double tol,
                                     int maxIter)
{
    QElapsedTimer timer;
    timer.start();
    m_evalCount = 0;

    int n = x0.size();
    if (n == 0) return x0;

    /* 初始化单纯形 */
    QVector<Vertex> simplex = initSimplex(x0, f);

    /* Nelder-Mead系数 */
    const double alpha = 1.0;   ///< 反射系数
    const double gamma = 2.0;   ///< 扩展系数
    const double beta  = 0.5;   ///< 收缩系数
    const double sigma = 0.5;   ///< 缩小系数

    int iter = 0;
    for (; iter < maxIter; ++iter) {
        /* 排序: 最好→最差 */
        std::sort(simplex.begin(), simplex.end(),
                  [](const Vertex& a, const Vertex& b) {
                      return a.value < b.value;
                  });

        /* 收敛判断: 单纯形足够小 */
        double bestVal = simplex.first().value;
        double worstVal = simplex.last().value;
        double spread = qAbs(worstVal - bestVal);
        if (spread < tol && iter > 0) break;

        /* 计算重心(排除最差点) */
        QVector<double> cent = centroid(simplex, simplex.size() - 1);

        /* 反射 */
        Vertex xr = reflect(cent, simplex.last(), alpha);
        xr.value = f(xr.coords);
        ++m_evalCount;

        if (xr.value >= simplex[0].value &&
            xr.value < simplex[simplex.size() - 2].value) {
            /* 反射点可接受: 替换最差点 */
            simplex.last() = xr;
            continue;
        }

        if (xr.value < simplex[0].value) {
            /* 反射点比最好还好: 尝试扩展 */
            Vertex xe = expand(cent, simplex.last(), gamma, f);
            ++m_evalCount;
            if (xe.value < xr.value) {
                simplex.last() = xe;
            } else {
                simplex.last() = xr;
            }
            continue;
        }

        /* 反射点比次差还差: 尝试收缩 */
        if (xr.value < simplex.last().value) {
            /* 外部收缩 */
            Vertex xc = contractOutside(cent, simplex.last(), beta, f);
            ++m_evalCount;
            if (xc.value <= xr.value) {
                simplex.last() = xc;
                continue;
            }
        } else {
            /* 内部收缩 */
            Vertex xc = contractInside(cent, simplex.last(), beta, f);
            ++m_evalCount;
            if (xc.value < simplex.last().value) {
                simplex.last() = xc;
                continue;
            }
        }

        /* 缩小单纯形 */
        shrink(simplex, sigma, f);
        m_evalCount += static_cast<quint64>(simplex.size() - 1);
    }

    /* 最终排序取最优 */
    std::sort(simplex.begin(), simplex.end(),
              [](const Vertex& a, const Vertex& b) {
                  return a.value < b.value;
              });

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalMinimizations;
    m_stats.totalEvaluations += m_evalCount;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalMinimizations);

    emit minimizationCompleted(simplex.first().value, iter);
    return simplex.first().coords;
}

/** @brief 初始化单纯形 */
QVector<NelderMead::Vertex> NelderMead::initSimplex(
    const QVector<double>& x0, const ObjFunc& f)
{
    int n = x0.size();
    QVector<Vertex> simplex(n + 1);

    /* 第一个顶点 = 初始点 */
    simplex[0].coords = x0;
    simplex[0].value = f(x0);
    ++m_evalCount;

    /* 其余n个顶点: 沿各坐标方向偏移 */
    for (int i = 0; i < n; ++i) {
        simplex[i + 1].coords = x0;
        simplex[i + 1].coords[i] += computeStep(n, i);
        simplex[i + 1].value = f(simplex[i + 1].coords);
        ++m_evalCount;
    }

    return simplex;
}

/** @brief 简单步长估计 */
double NelderMead::computeStep(int dim, int idx)
{
    Q_UNUSED(dim)
    Q_UNUSED(idx)
    return 0.05;  ///< 默认步长5%
}

/** @brief 计算重心 */
QVector<double> NelderMead::centroid(
    const QVector<Vertex>& simplex, int excludeIdx)
{
    int n = simplex[0].coords.size();
    QVector<double> cent(n, 0.0);
    int count = 0;

    for (int i = 0; i < simplex.size(); ++i) {
        if (i == excludeIdx) continue;
        for (int j = 0; j < n; ++j) {
            cent[j] += simplex[i].coords[j];
        }
        ++count;
    }

    for (int j = 0; j < n; ++j) {
        cent[j] /= static_cast<double>(count);
    }
    return cent;
}

/** @brief 反射 */
NelderMead::Vertex NelderMead::reflect(
    const QVector<double>& cent, const Vertex& worst, double alpha)
{
    int n = worst.coords.size();
    Vertex xr;
    xr.coords.resize(n);
    for (int i = 0; i < n; ++i) {
        xr.coords[i] = cent[i] + alpha * (cent[i] - worst.coords[i]);
    }
    return xr;
}

/** @brief 扩展 */
NelderMead::Vertex NelderMead::expand(
    const QVector<double>& cent, const Vertex& worst,
    double gamma, const ObjFunc& f)
{
    Q_UNUSED(f)
    int n = worst.coords.size();
    Vertex xe;
    xe.coords.resize(n);
    for (int i = 0; i < n; ++i) {
        xe.coords[i] = cent[i] + gamma * (cent[i] - worst.coords[i]);
    }
    xe.value = f(xe.coords);
    return xe;
}

/** @brief 外部收缩 */
NelderMead::Vertex NelderMead::contractOutside(
    const QVector<double>& cent, const Vertex& worst,
    double beta, const ObjFunc& f)
{
    Q_UNUSED(f)
    int n = worst.coords.size();
    Vertex xc;
    xc.coords.resize(n);
    for (int i = 0; i < n; ++i) {
        xc.coords[i] = cent[i] + beta * (cent[i] - worst.coords[i]);
    }
    xc.value = f(xc.coords);
    return xc;
}

/** @brief 内部收缩 */
NelderMead::Vertex NelderMead::contractInside(
    const QVector<double>& cent, const Vertex& worst,
    double beta, const ObjFunc& f)
{
    Q_UNUSED(f)
    int n = worst.coords.size();
    Vertex xc;
    xc.coords.resize(n);
    for (int i = 0; i < n; ++i) {
        xc.coords[i] = cent[i] - beta * (cent[i] - worst.coords[i]);
    }
    xc.value = f(xc.coords);
    return xc;
}

/** @brief 缩小 */
void NelderMead::shrink(QVector<Vertex>& simplex, double sigma,
                        const ObjFunc& f)
{
    int n = simplex[0].coords.size();

    /* 向最优点缩小 */
    for (int i = 1; i < simplex.size(); ++i) {
        for (int j = 0; j < n; ++j) {
            simplex[i].coords[j] = simplex[0].coords[j]
                + sigma * (simplex[i].coords[j] - simplex[0].coords[j]);
        }
        simplex[i].value = f(simplex[i].coords);
    }
}

/** @brief 重置统计 */
void NelderMead::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_evalCount = 0;
}
