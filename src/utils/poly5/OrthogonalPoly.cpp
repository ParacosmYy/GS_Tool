/**
 * @file OrthogonalPoly.cpp
 * @brief 正交多项式族实现 — Legendre/Hermite/Laguerre
 */

#include "utils/poly5/OrthogonalPoly.h"

#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
OrthogonalPoly::OrthogonalPoly(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算正交多项式在x处的值
 * 使用三项递推关系: P_{n+1}(x) = (a_n * x + b_n) * P_n(x) - c_n * P_{n-1}(x)
 */
double OrthogonalPoly::evaluate(PolyFamily family, int n, double x) const
{
    switch (family) {
    case PolyFamily::Legendre: return legendreRecurse(n, x);
    case PolyFamily::Hermite:  return hermiteRecurse(n, x);
    case PolyFamily::Laguerre: return laguerreRecurse(n, x);
    }
    return 0.0;
}

/**
 * @brief 批量计算0到maxOrder阶的值
 */
QVector<double> OrthogonalPoly::evaluateRange(PolyFamily family, int maxOrder,
                                               double x) const
{
    m_timer.start();
    QVector<double> result;
    if (maxOrder < 0) return result;

    result.reserve(maxOrder + 1);

    switch (family) {
    case PolyFamily::Legendre: {
        /* P_0 = 1, P_1 = x, (n+1)*P_{n+1} = (2n+1)*x*P_n - n*P_{n-1} */
        double prev2 = 1.0;
        double prev1 = x;
        result.append(prev2);
        if (maxOrder >= 1) result.append(prev1);
        for (int n = 1; n < maxOrder; ++n) {
            double curr = ((2.0 * n + 1.0) * x * prev1 - n * prev2)
                / static_cast<double>(n + 1);
            result.append(curr);
            prev2 = prev1;
            prev1 = curr;
        }
        break;
    }
    case PolyFamily::Hermite: {
        /* H_0 = 1, H_1 = 2x, H_{n+1} = 2x*H_n - 2n*H_{n-1} */
        double prev2 = 1.0;
        double prev1 = 2.0 * x;
        result.append(prev2);
        if (maxOrder >= 1) result.append(prev1);
        for (int n = 1; n < maxOrder; ++n) {
            double curr = 2.0 * x * prev1 - 2.0 * n * prev2;
            result.append(curr);
            prev2 = prev1;
            prev1 = curr;
        }
        break;
    }
    case PolyFamily::Laguerre: {
        /* L_0 = 1, L_1 = 1-x, (n+1)*L_{n+1} = (2n+1-x)*L_n - n*L_{n-1} */
        double prev2 = 1.0;
        double prev1 = 1.0 - x;
        result.append(prev2);
        if (maxOrder >= 1) result.append(prev1);
        for (int n = 1; n < maxOrder; ++n) {
            double curr = ((2.0 * n + 1.0 - x) * prev1 - n * prev2)
                / static_cast<double>(n + 1);
            result.append(curr);
            prev2 = prev1;
            prev1 = curr;
        }
        break;
    }
    }

    m_timeSum += m_timer.elapsed();
    m_stats.totalEvaluations += (maxOrder + 1);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalEvaluations));

    return result;
}

/**
 * @brief 计算高斯求积节点和权重
 * 使用Newton法求多项式根，然后由导数计算权重
 */
QPair<QVector<double>, QVector<double>>
OrthogonalPoly::gaussQuadrature(PolyFamily family, int nPoints)
{
    m_timer.start();

    QVector<double> nodes(nPoints), weights(nPoints);
    if (nPoints <= 0) return {nodes, weights};

    /* 确定求积区间和初始猜测 */
    double a = 0.0, b = 1.0;
    switch (family) {
    case PolyFamily::Legendre: a = -1.0; b = 1.0; break;
    case PolyFamily::Hermite:  a = -10.0; b = 10.0; break;
    case PolyFamily::Laguerre: a = 0.0; b = 20.0; break;
    }

    /* 初始猜测: Chebyshev节点 */
    for (int i = 0; i < nPoints; ++i) {
        double guess = 0.5 * ((b - a) * std::cos(M_PI * (i + 0.75)
            / static_cast<double>(nPoints + 0.5)) + (b + a));

        /* Newton迭代求根 */
        nodes[i] = findRoot(family, nPoints, guess, a, b);
    }

    /* 计算权重: w_i = 积分(连乘基函数) / P_n'(x_i) */
    for (int i = 0; i < nPoints; ++i) {
        double xi = nodes[i];

        /* 计算P_n'(x_i) (使用递推关系的导数) */
        double pnVal = evaluate(family, nPoints, xi);
        double pn1Val = evaluate(family, nPoints - 1, xi);

        /* 通用公式: w_i = 1 / (x_i * [P_n'(x_i)]^2) (Legendre特化) */
        double deriv = 0.0;
        switch (family) {
        case PolyFamily::Legendre:
            /* P_n'(x_i) = n * (x*P_n - P_{n-1}) / (x^2 - 1) */
            if (std::abs(xi * xi - 1.0) > 1e-12) {
                deriv = nPoints * (xi * pnVal - pn1Val) / (xi * xi - 1.0);
            }
            if (std::abs(deriv) > 1e-15) {
                weights[i] = 2.0 / ((1.0 - xi * xi) * deriv * deriv);
            }
            break;
        case PolyFamily::Hermite:
            /* w_i = sqrt(pi) * 2^{n-1} * n! / (n * H_{n-1}(x_i))^2 */
            deriv = 2.0 * nPoints * pn1Val;
            if (std::abs(deriv) > 1e-15) {
                weights[i] = std::sqrt(M_PI) * std::pow(2.0, nPoints - 1)
                    / (deriv * deriv);
            }
            break;
        case PolyFamily::Laguerre:
            /* w_i = x_i / ((n+1)^2 * L_{n+1}(x_i))^2 */
            deriv = evaluate(family, nPoints + 1, xi);
            if (std::abs(deriv) > 1e-15) {
                weights[i] = xi / ((nPoints + 1) * deriv * deriv);
            }
            break;
        }
    }

    m_timeSum += m_timer.elapsed();
    ++m_stats.totalQuadratures;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalQuadratures));

    emit quadratureCompleted(nPoints);
    return {nodes, weights};
}

/**
 * @brief 使用正交多项式进行最小二乘拟合
 * 展开系数 c_k = <y, P_k> / <P_k, P_k>
 */
QVector<double> OrthogonalPoly::polyFit(PolyFamily family,
                                         const QVector<double>& xData,
                                         const QVector<double>& yData,
                                         int degree)
{
    m_timer.start();

    int n = qMin(xData.size(), yData.size());
    QVector<double> coeffs;
    if (n == 0 || degree < 0) return coeffs;

    coeffs.resize(degree + 1);

    /* 均匀权重(可扩展为非均匀) */
    QVector<double> w(n, 1.0);

    /* 计算各阶系数 */
    for (int k = 0; k <= degree; ++k) {
        double num = innerProduct(family, k, xData, yData, w);
        double den = innerProduct(family, k, xData,
            /* 以多项式自身作为yData来计算 <P_k, P_k> */
            [&]() -> QVector<double> {
                QVector<double> pk(n);
                for (int i = 0; i < n; ++i) pk[i] = evaluate(family, k, xData[i]);
                return pk;
            }(), w);

        coeffs[k] = (std::abs(den) > 1e-15) ? num / den : 0.0;
    }

    m_timeSum += m_timer.elapsed();
    ++m_stats.totalFits;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalFits));

    emit fitCompleted(degree);
    return coeffs;
}

/**
 * @brief 使用展开系数计算拟合值
 */
double OrthogonalPoly::evaluateFit(PolyFamily family,
                                    const QVector<double>& coeffs,
                                    double x) const
{
    double result = 0.0;
    for (int k = 0; k < coeffs.size(); ++k) {
        result += coeffs[k] * evaluate(family, k, x);
    }
    return result;
}

/** @brief 重置统计 */
void OrthogonalPoly::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Legendre三项递推
 * P_0=1, P_1=x, (n+1)*P_{n+1}=(2n+1)*x*P_n - n*P_{n-1}
 */
double OrthogonalPoly::legendreRecurse(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return x;

    double prev2 = 1.0;
    double prev1 = x;
    double curr = 0.0;

    for (int k = 1; k < n; ++k) {
        curr = ((2.0 * k + 1.0) * x * prev1 - k * prev2)
            / static_cast<double>(k + 1);
        prev2 = prev1;
        prev1 = curr;
    }

    return curr;
}

/**
 * @brief Hermite三项递推
 * H_0=1, H_1=2x, H_{n+1}=2x*H_n - 2n*H_{n-1}
 */
double OrthogonalPoly::hermiteRecurse(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return 2.0 * x;

    double prev2 = 1.0;
    double prev1 = 2.0 * x;
    double curr = 0.0;

    for (int k = 1; k < n; ++k) {
        curr = 2.0 * x * prev1 - 2.0 * k * prev2;
        prev2 = prev1;
        prev1 = curr;
    }

    return curr;
}

/**
 * @brief Laguerre三项递推
 * L_0=1, L_1=1-x, (n+1)*L_{n+1}=(2n+1-x)*L_n - n*L_{n-1}
 */
double OrthogonalPoly::laguerreRecurse(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return 1.0 - x;

    double prev2 = 1.0;
    double prev1 = 1.0 - x;
    double curr = 0.0;

    for (int k = 1; k < n; ++k) {
        curr = ((2.0 * k + 1.0 - x) * prev1 - k * prev2)
            / static_cast<double>(k + 1);
        prev2 = prev1;
        prev1 = curr;
    }

    return curr;
}

/**
 * @brief Newton法求根
 * 在区间[a,b]上求解P_n(x)=0
 */
double OrthogonalPoly::findRoot(PolyFamily family, int n, double guess,
                                 double a, double b) const
{
    double x = qBound(a, guess, b);
    static constexpr int kMaxIter = 100;
    static constexpr double kTol = 1e-14;

    for (int i = 0; i < kMaxIter; ++i) {
        double fx = evaluate(family, n, x);
        if (std::abs(fx) < kTol) break;

        /* 数值导数 */
        double h = qMax(1e-8, std::abs(x) * 1e-8);
        double fxp = evaluate(family, n, x + h);
        double fxm = evaluate(family, n, x - h);
        double dfx = (fxp - fxm) / (2.0 * h);

        if (std::abs(dfx) < 1e-30) break;
        double xNew = x - fx / dfx;
        xNew = qBound(a, xNew, b);

        if (std::abs(xNew - x) < kTol) break;
        x = xNew;
    }

    return x;
}

/**
 * @brief 计算内积 <y, P_k> = sum w_i * y_i * P_k(x_i)
 */
double OrthogonalPoly::innerProduct(PolyFamily family, int k,
                                     const QVector<double>& xData,
                                     const QVector<double>& yData,
                                     const QVector<double>& weights) const
{
    int n = qMin(qMin(xData.size(), yData.size()), weights.size());
    double result = 0.0;

    for (int i = 0; i < n; ++i) {
        result += weights[i] * yData[i] * evaluate(family, k, xData[i]);
    }

    return result;
}
