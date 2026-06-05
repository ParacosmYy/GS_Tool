/**
 * @file SchurDecomp.cpp
 * @brief Schur分解实现 — Hessenberg化简 + QR迭代 + Wilkinson位移
 */

#include "utils/matrix9/SchurDecomp.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param maxIter 最大迭代 @param tolerance 容差 @param parent 父对象 */
SchurDecomp::SchurDecomp(int maxIter, double tolerance, QObject* parent)
    : QObject(parent)
    , m_maxIter(std::max(10, maxIter))
    , m_tolerance(std::max(1e-16, tolerance))
{
}

/** @brief Householder向量化 @param x 输入向量 @param v 输出向量 @return beta */
double SchurDecomp::householder(const std::vector<double>& x, std::vector<double>& v) const
{
    int n = static_cast<int>(x.size());
    v.assign(n, 0.0);

    double sigma = 0.0;
    for (int i = 1; i < n; ++i) sigma += x[i] * x[i];

    v[0] = 1.0;
    for (int i = 1; i < n; ++i) v[i] = x[i];

    if (sigma < 1e-30) return 0.0;

    double mu = std::sqrt(x[0] * x[0] + sigma);
    if (x[0] <= 0.0) {
        v[0] = x[0] - mu;
    } else {
        v[0] = -sigma / (x[0] + mu);
    }

    double beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    double scale = v[0];
    for (auto& vi : v) vi /= scale;
    return beta;
}

/** @brief Hessenberg化简 @param A 矩阵 @param Q 变换矩阵 */
void SchurDecomp::hessenbergReduce(std::vector<std::vector<double>>& A,
                                    std::vector<std::vector<double>>& Q) const
{
    int n = static_cast<int>(A.size());

    /* Q初始化为单位矩阵 */
    Q.assign(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    for (int k = 0; k < n - 2; ++k) {
        /* 提取列向量 */
        std::vector<double> x(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i) x[i] = A[k + 1 + i][k];

        std::vector<double> v;
        double beta = householder(x, v);
        if (beta == 0.0) continue;

        /* 左乘: A(k+1:n, k:n) = (I - beta*v*v^T) * A(k+1:n, k:n) */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (size_t i = 0; i < v.size(); ++i) dot += v[i] * A[k + 1 + i][j];
            for (size_t i = 0; i < v.size(); ++i) A[k + 1 + i][j] -= beta * v[i] * dot;
        }

        /* 右乘: A(1:n, k+1:n) = A(1:n, k+1:n) * (I - beta*v*v^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (size_t j = 0; j < v.size(); ++j) dot += A[i][k + 1 + j] * v[j];
            for (size_t j = 0; j < v.size(); ++j) A[i][k + 1 + j] -= beta * dot * v[j];
        }

        /* 累积变换到Q */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (size_t j = 0; j < v.size(); ++j) dot += Q[i][k + 1 + j] * v[j];
            for (size_t j = 0; j < v.size(); ++j) Q[i][k + 1 + j] -= beta * dot * v[j];
        }
    }
}

/** @brief QR单步(带Wilkinson位移) @param H Hessenberg矩阵 @param Q 变换矩阵 @param lo 下界 @param hi 上界 */
void SchurDecomp::qrStep(std::vector<std::vector<double>>& H,
                          std::vector<std::vector<double>>& Q, int lo, int hi)
{
    int n = static_cast<int>(H.size());

    /* Wilkinson位移: 使用右下2x2块的特征值 */
    double a = H[hi - 1][hi - 1], b = H[hi - 1][hi];
    double c = H[hi][hi - 1], d = H[hi][hi];

    /* 计算2x2块的特征值，选择接近d的那个 */
    double tr = a + d;
    double det = a * d - b * c;
    double disc = tr * tr - 4.0 * det;
    double s1, s2;
    if (disc >= 0.0) {
        double sq = std::sqrt(disc);
        s1 = (tr + sq) / 2.0;
        s2 = (tr - sq) / 2.0;
    } else {
        s1 = tr / 2.0;
        s2 = tr / 2.0;
    }
    double shift = (std::abs(s1 - d) < std::abs(s2 - d)) ? s1 : s2;

    /* 带位移的QR步: H - sI 的QR分解 */
    double x = H[lo][lo] - shift;
    double y = H[lo + 1][lo];

    for (int k = lo; k < hi; ++k) {
        /* Givens旋转消除y */
        double r = std::hypot(x, y);
        double c_g = (r > 1e-30) ? x / r : 1.0;
        double s_g = (r > 1e-30) ? -y / r : 0.0;

        /* 左乘Givens旋转 (行k和k+1) */
        for (int j = 0; j < n; ++j) {
            double t1 = H[k][j], t2 = H[k + 1][j];
            H[k][j]     = c_g * t1 - s_g * t2;
            H[k + 1][j] = s_g * t1 + c_g * t2;
        }

        /* 右乘Givens旋转 (列k和k+1) */
        for (int i = 0; i < n; ++i) {
            double t1 = H[i][k], t2 = H[i][k + 1];
            H[i][k]     = c_g * t1 - s_g * t2;
            H[i][k + 1] = s_g * t1 + c_g * t2;
        }

        /* 累积到Q */
        for (int i = 0; i < n; ++i) {
            double t1 = Q[i][k], t2 = Q[i][k + 1];
            Q[i][k]     = c_g * t1 - s_g * t2;
            Q[i][k + 1] = s_g * t1 + c_g * t2;
        }

        /* 准备下一列的旋转 */
        if (k < hi - 1) {
            x = H[k + 1][k];
            y = H[k + 2][k];
        }
    }
}

/** @brief 2x2块特征值 @param a11 元素 @param a12 元素 @param a21 元素 @param a22 元素 @return 特征值对 */
std::pair<std::complex<double>, std::complex<double>> SchurDecomp::eigenvalues2x2(
    double a11, double a12, double a21, double a22) const
{
    double tr = a11 + a22;
    double det = a11 * a22 - a12 * a21;
    double disc = tr * tr - 4.0 * det;

    if (disc >= 0.0) {
        double sq = std::sqrt(disc);
        return {{(tr + sq) / 2.0, 0.0}, {(tr - sq) / 2.0, 0.0}};
    } else {
        double sq = std::sqrt(-disc);
        return {{tr / 2.0, sq / 2.0}, {tr / 2.0, -sq / 2.0}};
    }
}

/** @brief 提取特征值 @param T Schur矩阵 @return 特征值列表 */
std::vector<std::complex<double>> SchurDecomp::extractEigenvalues(
    const std::vector<std::vector<double>>& T) const
{
    std::vector<std::complex<double>> eigs;
    int n = static_cast<int>(T.size());
    int i = 0;
    while (i < n) {
        if (i == n - 1 || std::abs(T[i + 1][i]) < m_tolerance) {
            /* 1x1块: 实特征值 */
            eigs.emplace_back(T[i][i], 0.0);
            ++i;
        } else {
            /* 2x2块: 可能有复特征值 */
            auto [e1, e2] = eigenvalues2x2(T[i][i], T[i][i + 1], T[i + 1][i], T[i + 1][i + 1]);
            eigs.push_back(e1);
            eigs.push_back(e2);
            i += 2;
        }
    }
    return eigs;
}

/** @brief 执行Schur分解 @param matrix 输入矩阵 @return 分解结果 */
SchurDecomp::Result SchurDecomp::decompose(const std::vector<std::vector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = static_cast<int>(matrix.size());
    if (n == 0) return result;

    /* 检查矩阵是否方阵 */
    for (const auto& row : matrix) {
        if (static_cast<int>(row.size()) != n) return result;
    }

    /* 复制矩阵 */
    auto H = matrix;

    /* Step 1: Hessenberg化简 */
    std::vector<std::vector<double>> Q;
    hessenbergReduce(H, Q);

    /* Step 2: QR迭代 */
    int hi = n - 1;
    int iter = 0;

    while (hi > 0 && iter < m_maxIter) {
        /* 寻找可缩减的子矩阵 */
        int lo = hi;
        while (lo > 0 && std::abs(H[lo][lo - 1]) > m_tolerance * (
            std::abs(H[lo - 1][lo - 1]) + std::abs(H[lo][lo]))) {
            --lo;
        }

        if (lo == hi) {
            /* 1x1块已收敛 */
            --hi;
        } else if (lo == hi - 1) {
            /* 2x2块检查 */
            if (std::abs(H[hi][hi - 1]) < m_tolerance * (
                std::abs(H[hi - 1][hi - 1]) + std::abs(H[hi][hi]))) {
                H[hi][hi - 1] = 0.0;
                hi -= 2;
            } else {
                qrStep(H, Q, lo, hi);
                ++iter;
            }
        } else {
            /* 对子矩阵执行QR步 */
            qrStep(H, Q, lo, hi);
            ++iter;
        }
    }

    /* 清理小量 */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i > j + 1) H[i][j] = 0.0;
        }
    }

    result.Q = Q;
    result.T = H;
    result.eigenvalues = extractEigenvalues(H);
    result.iterations = iter;
    result.converged = (hi <= 0);

    m_stats.totalDecompositions++;
    m_stats.totalIterations += iter;
    if (result.converged) m_stats.totalConverged++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    return result;
}

/** @brief 仅计算特征值 @param matrix 输入矩阵 @return 特征值 */
std::vector<std::complex<double>> SchurDecomp::eigenvaluesOnly(
    const std::vector<std::vector<double>>& matrix)
{
    auto result = decompose(matrix);
    return result.eigenvalues;
}

/** @brief 矩阵指数 @param T Schur矩阵 @param Q 正交矩阵 @return exp(A) */
std::vector<std::vector<double>> SchurDecomp::matrixExponential(
    const std::vector<std::vector<double>>& T,
    const std::vector<std::vector<double>>& Q) const
{
    int n = static_cast<int>(T.size());
    if (n == 0 || static_cast<int>(Q.size()) != n) return {};

    /* 计算 exp(T): 对角块分别处理 */
    auto expT = std::vector<std::vector<double>>(n, std::vector<double>(n, 0.0));
    int i = 0;
    while (i < n) {
        if (i == n - 1 || std::abs(T[i + 1][i]) < 1e-12) {
            /* 1x1对角块: exp(T[i][i]) */
            expT[i][i] = std::exp(T[i][i]);
            ++i;
        } else {
            /* 2x2对角块: 使用解析公式 exp([[a,b],[c,d]]) */
            double a = T[i][i], b = T[i][i + 1];
            double c = T[i + 1][i], d = T[i + 1][i + 1];
            double tr = (a + d) / 2.0;
            double det = a * d - b * c;
            double disc = tr * tr - det;

            if (disc >= 0.0) {
                /* 两个不同实特征值 */
                double sq = std::sqrt(disc);
                double e1 = std::exp(tr + sq), e2 = std::exp(tr - sq);
                if (sq > 1e-15) {
                    expT[i][i] = (e1 * (-tr + sq + a) + e2 * (tr - sq - a)) / (2.0 * sq);
                    expT[i][i + 1] = b * (e1 - e2) / (2.0 * sq);
                    expT[i + 1][i] = c * (e1 - e2) / (2.0 * sq);
                    expT[i + 1][i + 1] = (e1 * (tr - sq - a) + e2 * (-tr + sq + a)) / (2.0 * sq) + e1 + e2 - expT[i][i];
                } else {
                    double e = std::exp(tr);
                    expT[i][i] = e * (1.0 + (a - tr));
                    expT[i][i + 1] = e * b;
                    expT[i + 1][i] = e * c;
                    expT[i + 1][i + 1] = e * (1.0 + (d - tr));
                }
            } else {
                /* 复共轭特征值对 */
                double e = std::exp(tr);
                double w = std::sqrt(-disc);
                expT[i][i] = e * (std::cos(w) + (a - tr) * std::sin(w) / w);
                expT[i][i + 1] = e * b * std::sin(w) / w;
                expT[i + 1][i] = e * c * std::sin(w) / w;
                expT[i + 1][i + 1] = e * (std::cos(w) + (d - tr) * std::sin(w) / w);
            }
            i += 2;
        }
    }

    /* exp(A) = Q * exp(T) * Q^T */
    auto result = std::vector<std::vector<double>>(n, std::vector<double>(n, 0.0));
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                for (int l = 0; l < n; ++l) {
                    sum += Q[r][k] * expT[k][l] * Q[c][l];
                }
            }
            result[r][c] = sum;
        }
    }
    return result;
}

/** @brief 重置统计 */
void SchurDecomp::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
