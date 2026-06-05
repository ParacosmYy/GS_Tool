/**
 * @file ChebyshevApproximation.cpp
 * @brief Chebyshev 多项式逼近实现
 */

#include "utils/chebyshev2/ChebyshevApproximation.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
ChebyshevApproximation::ChebyshevApproximation(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 通过离散余弦变换计算 Chebyshev 系数
 *
 * 利用 Clenshaw-Curtis 求积公式，在 Chebyshev-Gauss-Lobatto 节点
 * x_k = cos(kπ/N), k=0..N 上采样，再由 DCT-II 得到展开系数。
 * 系数计算: c_j = (2/N) Σ_{k=0}^{N} f(x_k) T_j(x_k) / γ_j
 * 其中 γ_0 = γ_N = 2, 其余 γ_j = 1。
 */
QVector<double> ChebyshevApproximation::approximate(
    Func f, double a, double b, int degree)
{
    QElapsedTimer timer;
    timer.start();

    const int N = degree;  /* 采样点数 = degree */
    QVector<double> coeffs(degree + 1, 0.0);

    /* 在 Chebyshev-Gauss-Lobatto 节点上采样 */
    QVector<double> fvals(N + 1);
    for (int k = 0; k <= N; ++k) {
        double theta = M_PI * k / N;
        double cosT  = std::cos(theta);
        /* 从 [-1,1] 映射到 [a,b]: t = (b-a)/2 * cosT + (a+b)/2 */
        double t = (b - a) / 2.0 * cosT + (a + b) / 2.0;
        fvals[k] = f(t);
    }

    /* DCT-II 计算系数 */
    for (int j = 0; j <= degree; ++j) {
        double sum = 0.0;
        for (int k = 0; k <= N; ++k) {
            double theta = M_PI * j * k / N;
            sum += fvals[k] * std::cos(theta);
        }
        coeffs[j] = 2.0 * sum / N;

        /* 首尾项权重因子 */
        if (j == 0)
            coeffs[j] *= 0.5;
        if (j == degree)
            coeffs[j] *= 0.5;
    }

    m_stats.totalApproximations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalApproximations;
    emit approximationCompleted(degree);
    return coeffs;
}

/**
 * @brief Clenshaw 递推求值
 *
 * 将查询点从 [a,b] 映射到 [-1,1]，然后用递推:
 *   b_{n+1} = 0, b_n = c_n
 *   b_k = 2t·b_{k+1} - b_{k+2} + c_k
 * 最终: f(x) ≈ t·b_1 - b_2 + c_0
 */
double ChebyshevApproximation::evaluate(QVector<double> coeffs,
                                        double a, double b, double x)
{
    const int n = coeffs.size() - 1;
    if (n < 0) return 0.0;

    /* 从 [a,b] 映射到 [-1,1] */
    double t = (2.0 * x - a - b) / (b - a);

    if (n == 0) return coeffs[0];
    if (n == 1) return coeffs[0] + coeffs[1] * t;

    /* Clenshaw 递推 */
    double b2 = 0.0;
    double b1 = coeffs[n];
    for (int k = n - 1; k >= 1; --k) {
        double b0 = 2.0 * t * b1 - b2 + coeffs[k];
        b2 = b1;
        b1 = b0;
    }

    return t * b1 - b2 + coeffs[0];
}

/** @brief 重置统计 */
void ChebyshevApproximation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
