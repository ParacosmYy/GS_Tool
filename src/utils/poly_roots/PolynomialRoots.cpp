/**
 * @file PolynomialRoots.cpp
 * @brief 多项式求根实现
 */

#include "utils/poly_roots/PolynomialRoots.h"

#include <QtMath>
#include <algorithm>
#include <numeric>

/** @brief 构造函数 @param parent 父对象 */
PolynomialSolver::PolynomialSolver(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 求多项式所有根(含复数)
 * @param coeffs 系数向量(高次到低次)
 * @return 根的实部列表
 */
QVector<double> PolynomialSolver::findAllRoots(const QVector<double>& coeffs)
{
    if (coeffs.size() < 2) return {};

    m_timer.start();

    /* 去除前导零系数 */
    QVector<double> c;
    for (int i = 0; i < coeffs.size(); ++i) {
        if (!qFuzzyIsNull(coeffs[i]) || !c.isEmpty()) {
            c.append(coeffs[i]);
        }
    }
    if (c.size() < 2) return {};

    /* 缩放为首一多项式 */
    double lead = c[0];
    if (qFuzzyIsNull(lead)) return {};
    for (double& v : c) v /= lead;

    int n = c.size() - 1;

    /* 用同伴矩阵QR迭代求解 */
    QVector<Complex> roots = solveCompanionQR(c);

    /* Aberth迭代精化 */
    aberthRefine(c, roots);

    /* 提取实部 */
    QVector<double> realParts;
    realParts.reserve(roots.size());
    for (const Complex& r : roots) {
        realParts.append(r.re);
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalComputed;
    m_stats.totalRoots += static_cast<quint64>(realParts.size());
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalComputed);

    emit computed(realParts.size());
    return realParts;
}

/**
 * @brief 仅求实数根
 * @param coeffs 系数向量
 * @return 实数根列表
 */
QVector<double> PolynomialSolver::findRealRoots(const QVector<double>& coeffs)
{
    QVector<double> allRoots = findAllRoots(coeffs);

    /* 重新计算同伴矩阵结果获取虚部信息 */
    if (coeffs.size() < 2) return {};

    QVector<double> c;
    for (int i = 0; i < coeffs.size(); ++i) {
        if (!qFuzzyIsNull(coeffs[i]) || !c.isEmpty()) {
            c.append(coeffs[i]);
        }
    }
    if (c.size() < 2) return {};

    double lead = c[0];
    if (qFuzzyIsNull(lead)) return {};
    for (double& v : c) v /= lead;

    QVector<Complex> roots = solveCompanionQR(c);
    aberthRefine(c, roots);

    /* 过滤: 虚部绝对值 < 阈值 */
    QVector<double> realRoots;
    for (const Complex& r : roots) {
        if (qAbs(r.im) < IMAG_THRESHOLD) {
            realRoots.append(r.re);
        }
    }

    std::sort(realRoots.begin(), realRoots.end());
    return realRoots;
}

/**
 * @brief 求多项式在某点的值(Horner法则)
 * @param coeffs 系数向量
 * @param x 自变量
 * @return 多项式值
 */
double PolynomialSolver::evaluate(const QVector<double>& coeffs, double x) const
{
    if (coeffs.isEmpty()) return 0.0;

    double result = coeffs[0];
    for (int i = 1; i < coeffs.size(); ++i) {
        result = result * x + coeffs[i];
    }
    return result;
}

/** @brief 重置统计 */
void PolynomialSolver::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 同伴矩阵QR分解求根
 * 构造nxn同伴矩阵并对其做QR迭代，对角元即为特征值(根)
 */
QVector<PolynomialSolver::Complex> PolynomialSolver::solveCompanionQR(
    const QVector<double>& coeffs) const
{
    int n = coeffs.size() - 1;
    if (n <= 0) return {};

    /* 特殊情况: 一次方程 */
    if (n == 1) {
        return {Complex{-coeffs[1], 0.0}};
    }

    /* 二次方程直接公式 */
    if (n == 2) {
        double disc = coeffs[1] * coeffs[1] - 4.0 * coeffs[2];
        if (disc >= 0) {
            double sq = qSqrt(disc);
            return {Complex{(-coeffs[1] + sq) / 2.0, 0.0},
                    Complex{(-coeffs[1] - sq) / 2.0, 0.0}};
        } else {
            double sq = qSqrt(-disc);
            return {Complex{-coeffs[1] / 2.0, sq / 2.0},
                    Complex{-coeffs[1] / 2.0, -sq / 2.0}};
        }
    }

    /* 构造同伴矩阵(上Hessenberg形式) */
    QVector<QVector<Complex>> mat(n, QVector<Complex>(n, Complex{0, 0}));
    for (int i = 0; i < n - 1; ++i) {
        mat[i + 1][i] = Complex{1.0, 0.0};
    }
    for (int i = 0; i < n; ++i) {
        mat[i][n - 1] = Complex{-coeffs[n - i], 0.0};
    }

    /* QR迭代 */
    qrIteration(mat);

    /* 提取对角线作为根 */
    QVector<Complex> roots;
    roots.reserve(n);
    for (int i = 0; i < n; ++i) {
        roots.append(mat[i][i]);
    }
    return roots;
}

/**
 * @brief QR迭代 — 将矩阵对角化
 * 使用移位QR加速收敛
 */
void PolynomialSolver::qrIteration(
    QVector<QVector<Complex>>& mat) const
{
    int n = mat.size();

    for (int iter = 0; iter < MAX_ITER * n; ++iter) {
        /* 检查是否已基本对角化 */
        bool converged = true;
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(mat[i + 1][i].re) > 1e-10 || qAbs(mat[i + 1][i].im) > 1e-10) {
                converged = false;
                break;
            }
        }
        if (converged) break;

        /* Wilkinson移位: 取右下2x2块的特征值 */
        Complex shift = mat[n - 1][n - 1];

        /* 应用移位 */
        for (int i = 0; i < n; ++i) {
            mat[i][i] = cadd(mat[i][i], Complex{-shift.re, -shift.im});
        }

        /* Givens旋转QR分解(原地) */
        for (int i = 0; i < n - 1; ++i) {
            Complex a = mat[i][i];
            Complex b = mat[i + 1][i];
            double r = qSqrt(a.re * a.re + a.im * a.im
                           + b.re * b.re + b.im * b.im);
            if (r < 1e-15) continue;

            Complex c{a.re / r, a.im / r};
            Complex s{b.re / r, b.im / r};

            /* 左乘Givens旋转 */
            for (int j = i; j < n; ++j) {
                Complex t1 = cmul(c, mat[i][j]);
                Complex t2 = cmul(Complex{s.re, -s.im}, mat[i + 1][j]);
                mat[i][j] = cadd(t1, t2);

                Complex t3 = cmul(s, mat[i][j]);
                Complex t4 = cmul(Complex{c.re, -c.im}, mat[i + 1][j]);
                /* 保存原始值用于第二行计算 */
                Complex origLower = mat[i + 1][j];
                mat[i + 1][j] = cadd(
                    cmul(Complex{-s.re, -s.im}, mat[i][j]),
                    cmul(Complex{c.re, -c.im}, origLower));
            }
        }

        /* 右乘R恢复 + 反移位 */
        for (int i = 0; i < n; ++i) {
            mat[i][i] = cadd(mat[i][i], shift);
        }
    }
}

/**
 * @brief Aberth迭代精化根的精度
 * 并行牛顿校正 + 互斥加速
 */
void PolynomialSolver::aberthRefine(const QVector<double>& coeffs,
                                   QVector<Complex>& roots) const
{
    int n = roots.size();
    if (n == 0) return;

    for (int iter = 0; iter < 20; ++iter) {
        bool allConverged = true;

        for (int i = 0; i < n; ++i) {
            Complex p = evalComplex(coeffs, roots[i]);
            Complex pp = evalComplex(
                [&]() {
                    QVector<double> deriv;
                    for (int j = 0; j < static_cast<int>(coeffs.size()) - 1; ++j) {
                        deriv.append(coeffs[j]
                            * static_cast<double>(coeffs.size() - 1 - j));
                    }
                    return deriv;
                }(), roots[i]);

            if (qAbs(pp.re) < 1e-15 && qAbs(pp.im) < 1e-15) continue;

            /* 牛顿校正 */
            Complex correction = cdiv(p, pp);

            /* Aberth互斥项 */
            Complex denom{0.0, 0.0};
            for (int j = 0; j < n; ++j) {
                if (j != i) {
                    Complex diff = cadd(roots[i], Complex{-roots[j].re, -roots[j].im});
                    double mag = qSqrt(diff.re * diff.re + diff.im * diff.im);
                    if (mag > 1e-15) {
                        denom = cadd(denom, cdiv(Complex{1.0, 0.0}, diff));
                    }
                }
            }

            Complex aberth = cdiv(correction, Complex{1.0 - correction.re * denom.re
                + correction.im * denom.im,
                -correction.re * denom.im - correction.im * denom.re});

            roots[i] = cadd(roots[i], Complex{-aberth.re, -aberth.im});

            if (qAbs(aberth.re) > 1e-12 || qAbs(aberth.im) > 1e-12) {
                allConverged = false;
            }
        }

        if (allConverged) break;
    }
}

/**
 * @brief 复数求多项式值(Horner法则)
 */
PolynomialSolver::Complex PolynomialSolver::evalComplex(
    const QVector<double>& coeffs, const Complex& z) const
{
    if (coeffs.isEmpty()) return {0.0, 0.0};

    Complex result{coeffs[0], 0.0};
    for (int i = 1; i < coeffs.size(); ++i) {
        result = cadd(cmul(result, z), Complex{coeffs[i], 0.0});
    }
    return result;
}

/** @brief 复数乘法 */
PolynomialSolver::Complex PolynomialSolver::cmul(
    const Complex& a, const Complex& b)
{
    return {a.re * b.re - a.im * b.im,
            a.re * b.im + a.im * b.re};
}

/** @brief 复数加法 */
PolynomialSolver::Complex PolynomialSolver::cadd(
    const Complex& a, const Complex& b)
{
    return {a.re + b.re, a.im + b.im};
}

/** @brief 复数除法 */
PolynomialSolver::Complex PolynomialSolver::cdiv(
    const Complex& a, const Complex& b)
{
    double denom = b.re * b.re + b.im * b.im;
    if (denom < 1e-30) return {0.0, 0.0};
    return {(a.re * b.re + a.im * b.im) / denom,
            (a.im * b.re - a.re * b.im) / denom};
}
