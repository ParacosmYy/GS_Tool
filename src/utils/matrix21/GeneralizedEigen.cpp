/**
 * @file GeneralizedEigen.cpp
 * @brief 广义特征值求解器实现 — QZ分解/Hessenberg三角化/Givens旋转
 */

#include "utils/matrix21/GeneralizedEigen.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
GeneralizedEigen::GeneralizedEigen(QObject* parent)
    : QObject(parent)
    , m_maxIter(300)
    , m_tol(1e-12)
    , m_timeSum(0.0)
{
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void GeneralizedEigen::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(10, maxIter);
}

/** @brief 设置收敛容差 @param tol 容差 */
void GeneralizedEigen::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/** @brief 求解广义特征值问题 Ax=λBx @param A 矩阵A @param B 矩阵B @return 特征值列表 */
QVector<GeneralizedEigen::EigenValuePair> GeneralizedEigen::solve(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    QElapsedTimer timer;
    timer.start();

    QZResult qz = qzDecomposition(A, B);
    QVector<EigenValuePair> eigenvalues = extractEigenvalues(qz.S, qz.T);

    /* 计算特征向量(右特征向量通过回代) */
    int n = eigenvalues.size();
    for (int i = 0; i < n; ++i) {
        QVector<double> v(n, 0.0);
        v[i] = 1.0;

        /* 简化: 回代求解 (S - λT)v = 0 的近似解 */
        double re = eigenvalues[i].realPart;
        double im = eigenvalues[i].imagPart;
        if (i + 1 < n && qAbs(im) > m_tol) {
            /* 2×2块处理 */
            double a11 = qz.S[i][i] - re * qz.T[i][i];
            double a12 = qz.S[i][i+1] - re * qz.T[i][i+1];
            double a21 = qz.S[i+1][i];
            double a22 = qz.S[i+1][i+1] - re * qz.T[i+1][i+1];

            double det = a11 * a22 - a12 * a21;
            if (qAbs(det) > m_tol) {
                v[i] = 1.0;
                v[i+1] = -a11 / a12;
            }
        } else if (i > 0 && qAbs(eigenvalues[i-1].imagPart) > m_tol) {
            /* 属于上一个2×2块，跳过 */
            continue;
        } else {
            /* 1×1块 */
            v[i] = 1.0;
        }

        /* 通过Z^T变换回原空间 */
        QVector<double> originalV(n, 0.0);
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                originalV[j] += qz.Z[j][k] * v[k];
            }
        }

        /* 归一化 */
        double norm = 0.0;
        for (double x : originalV) norm += x * x;
        norm = qSqrt(norm);
        if (norm > m_tol) {
            for (auto& x : originalV) x /= norm;
        }

        eigenvalues[i].rightVector = originalV;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSolves;
    m_stats.totalEigenvaluesFound += static_cast<quint64>(n);
    m_stats.lastMatrixSize = n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, eigenvalues.size());
    return eigenvalues;
}

/** @brief QZ分解 @param A 矩阵A @param B 矩阵B @return QZResult */
GeneralizedEigen::QZResult GeneralizedEigen::qzDecomposition(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int n = A.size();
    if (n == 0 || B.size() != n) return {};

    QZResult result;
    result.S = A;
    result.T = B;
    result.Q = identityMatrix(n);
    result.Z = identityMatrix(n);

    /* 步骤1: Householder三角化B */
    householderTriangularize(result.T, result.Q);

    /* 将Q^T应用到A: A <- Q^T * A */
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += result.Q[j][i] * A[j][k];
            }
            result.S[i][k] = sum;
        }
    }

    /* 步骤2: 化简为Hessenberg-三角形式 */
    hessenbergReduce(result.S, result.T, result.Q, result.Z);

    /* 步骤3: QZ迭代(隐式双位移) */
    int p = n - 1;
    int iter = 0;
    while (p > 0 && iter < m_maxIter) {
        /* 检查B[p][p]是否近似零 */
        if (qAbs(result.T[p][p]) <= m_tol * (qAbs(result.S[p][p]) + qAbs(result.T[p][p]))) {
            /* 收缩 */
            --p;
            iter = 0;
            continue;
        }

        /* 检查S[p][p-1]是否足够小 */
        double threshold = m_tol * (qAbs(result.S[p-1][p-1]) + qAbs(result.S[p][p]));
        if (qAbs(result.S[p][p-1]) <= qMax(threshold, m_tol)) {
            result.S[p][p-1] = 0.0;
            --p;
            iter = 0;
            continue;
        }

        /* 执行一步QZ迭代 */
        qzStep(result.S, result.T, result.Q, result.Z, 0, p);
        ++iter;
    }

    return result;
}

/** @brief 化简为Hessenberg三角形式 @param A 矩阵A @param B 矩阵B @return HessenbergResult */
GeneralizedEigen::HessenbergResult GeneralizedEigen::reduceToHessenbergTriangular(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int n = A.size();
    HessenbergResult result;
    result.H = A;
    result.R = B;
    result.Q = identityMatrix(n);
    result.Z = identityMatrix(n);

    householderTriangularize(result.R, result.Q);

    /* Q^T * A */
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += result.Q[j][i] * A[j][k];
            }
            result.H[i][k] = sum;
        }
    }

    hessenbergReduce(result.H, result.R, result.Q, result.Z);
    return result;
}

/** @brief 从QZ分解提取特征值 @param S 上准三角 @param T 上三角 @return 特征值列表 */
QVector<GeneralizedEigen::EigenValuePair> GeneralizedEigen::extractEigenvalues(
    const QVector<QVector<double>>& S,
    const QVector<QVector<double>>& T)
{
    int n = S.size();
    QVector<EigenValuePair> eigenvalues;
    eigenvalues.reserve(n);

    int i = 0;
    while (i < n) {
        EigenValuePair ev;

        if (i == n - 1 || qAbs(S[i+1][i]) < m_tol * (qAbs(S[i][i]) + qAbs(S[i+1][i+1]))) {
            /* 1×1块: 实特征值 */
            if (qAbs(T[i][i]) > m_tol) {
                ev.realPart = S[i][i] / T[i][i];
            } else {
                ev.realPart = (qAbs(S[i][i]) > m_tol) ? 1e30 : 0.0;
            }
            ev.imagPart = 0.0;
            ev.magnitude = qAbs(ev.realPart);
            ev.phase = (ev.realPart >= 0) ? 0.0 : M_PI;
            ++i;
        } else {
            /* 2×2块: 复特征值对 */
            double a = S[i][i], b = S[i][i+1];
            double c = S[i+1][i], d = S[i+1][i+1];
            double e = T[i][i], f = T[i][i+1];
            double g = T[i+1][i], h = T[i+1][i+1];

            /* (a-λe)(d-λh) - (b-λf)(c-λg) = 0 */
            /* 展开: λ^2(eh - fg) - λ(ah + de - bg - cf) + (ad - bc) = 0 */
            double qa = e * h - f * g;
            double qb = -(a * h + d * e - b * g - c * f);
            double qc = a * d - b * c;

            if (qAbs(qa) < m_tol) qa = m_tol;
            double discriminant = qb * qb - 4.0 * qa * qc;

            ev.realPart = -qb / (2.0 * qa);
            if (discriminant < 0) {
                ev.imagPart = qSqrt(-discriminant) / (2.0 * qa);
            } else {
                ev.imagPart = 0.0;
                ev.realPart = (-qb + qSqrt(discriminant)) / (2.0 * qa);
            }

            ev.magnitude = qSqrt(ev.realPart * ev.realPart + ev.imagPart * ev.imagPart);
            ev.phase = qAtan2(ev.imagPart, ev.realPart);

            i += 2;
        }

        eigenvalues.append(ev);
    }

    return eigenvalues;
}

/** @brief 计算残差 @param A 矩阵A @param B 矩阵B @param eigenvalue 特征值 @param eigenvector 特征向量 @return 残差 */
double GeneralizedEigen::residual(const QVector<QVector<double>>& A,
                                   const QVector<QVector<double>>& B,
                                   const EigenValuePair& eigenvalue,
                                   const QVector<double>& eigenvector) const
{
    int n = A.size();
    if (eigenvector.size() != n) return 1e30;

    /* Ax - λBx */
    QVector<double> Ax(n, 0.0), Bx(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            Ax[i] += A[i][j] * eigenvector[j];
            Bx[i] += B[i][j] * eigenvector[j];
        }
    }

    double resNorm = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = Ax[i] - eigenvalue.realPart * Bx[i];
        resNorm += r * r;
    }

    m_stats.lastResidual = qSqrt(resNorm);
    return m_stats.lastResidual;
}

/** @brief 重置统计 */
void GeneralizedEigen::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Householder三角化 @param B 矩阵B @param Q 正交矩阵 */
void GeneralizedEigen::householderTriangularize(
    QVector<QVector<double>>& B,
    QVector<QVector<double>>& Q)
{
    int n = B.size();
    Q = identityMatrix(n);

    for (int k = 0; k < n - 1; ++k) {
        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) norm += B[i][k] * B[i][k];
        norm = qSqrt(norm);

        if (norm < m_tol) continue;

        double alpha = (B[k+1][k] >= 0) ? -norm : norm;
        double r = qSqrt(0.5 * (alpha * alpha - B[k+1][k] * alpha));
        if (r < m_tol) continue;

        QVector<double> v(n - k - 1, 0.0);
        v[0] = (B[k+1][k] - alpha) / (2.0 * r);
        for (int i = 1; i < n - k - 1; ++i) v[i] = B[k+1+i][k] / (2.0 * r);

        /* 应用Householder变换: B <- (I - 2vv^T) B */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n - k - 1; ++i) dot += v[i] * B[k+1+i][j];
            for (int i = 0; i < n - k - 1; ++i) B[k+1+i][j] -= 2.0 * v[i] * dot;
        }

        /* 更新Q: Q <- Q * (I - 2vv^T) */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n - k - 1; ++i) dot += v[i] * Q[j][k+1+i];
            for (int i = 0; i < n - k - 1; ++i) Q[j][k+1+i] -= 2.0 * v[i] * dot;
        }
    }
}

/** @brief Hessenberg化简 @param A 矩阵A @param B 矩阵B @param Q 正交矩阵Q @param Z 正交矩阵Z */
void GeneralizedEigen::hessenbergReduce(
    QVector<QVector<double>>& A,
    QVector<QVector<double>>& B,
    QVector<QVector<double>>& Q,
    QVector<QVector<double>>& Z)
{
    int n = A.size();

    for (int k = n - 2; k >= 0; --k) {
        for (int l = n - 1; l >= k + 2; --l) {
            if (qAbs(A[l][k]) < m_tol) continue;

            /* Givens旋转消除A[l][k] */
            double c, s;
            givensRotation(A[l-1][k], A[l][k], c, s);

            /* 左乘G^T作用于A */
            applyGivensLeft(A, l-1, l, c, -s, k, n);
            applyGivensLeft(B, l-1, l, c, -s, 0, n);

            /* 右乘G作用于A */
            applyGivensRight(A, l-1, l, c, s, 0, n);
            applyGivensRight(B, l-1, l, c, s, 0, qMin(l+1, n));

            /* 累积到Q和Z */
            applyGivensRight(Q, l-1, l, c, s, 0, n);
            applyGivensLeft(Z, l-1, l, c, -s, 0, n);
        }
    }
}

/** @brief 单步QZ迭代 @param A 矩阵A @param B 矩阵B @param Q 正交矩阵 @param Z 正交矩阵 @param lo 起始索引 @param hi 结束索引 */
void GeneralizedEigen::qzStep(QVector<QVector<double>>& A,
                               QVector<QVector<double>>& B,
                               QVector<QVector<double>>& Q,
                               QVector<QVector<double>>& Z,
                               int lo, int hi)
{
    int n = A.size();

    /* 计算隐式双位移 */
    double a11 = A[hi-1][hi-1] * B[hi][hi] - A[hi][hi] * B[hi-1][hi-1];
    double a21 = A[hi][hi-1] * B[hi][hi];
    double a31 = (hi >= 2) ? A[hi][hi-1] * B[hi-1][hi-2] : 0.0;

    /* Francis隐式双位移的第一列 */
    double h1 = a11 + a21;
    double h2 = a21 + a31;

    for (int k = lo; k < hi - 1; ++k) {
        /* 构造小Givens旋转 */
        double c, s;
        if (k == lo) {
            givensRotation(h1, h2, c, s);
        } else {
            givensRotation(A[k][k-1], A[k+1][k-1], c, s);
        }

        /* 右乘G到A, B */
        applyGivensRight(A, k, k+1, c, s, lo, qMin(k+3, hi+1));
        applyGivensRight(B, k, k+1, c, s, lo, k+2);
        applyGivensRight(Z, k, k+1, c, s, 0, n);

        /* 左乘G^T到A, B */
        givensRotation(A[k][k], A[k][k+1], c, s);
        applyGivensLeft(A, k, k+1, c, -s, k, qMin(k+3, hi+1));
        applyGivensLeft(B, k, k+1, c, -s, k+1, qMin(k+3, hi+1));
        applyGivensLeft(Q, k, k+1, c, -s, 0, n);
    }
}

/** @brief Givens旋转参数 @param a 第一个元素 @param b 第二个元素 @param c 余弦 @param s 正弦 */
void GeneralizedEigen::givensRotation(double a, double b,
                                       double& c, double& s) const
{
    if (qAbs(b) < m_tol) {
        c = 1.0;
        s = 0.0;
    } else if (qAbs(b) > qAbs(a)) {
        double t = -a / b;
        s = 1.0 / qSqrt(1.0 + t * t);
        c = s * t;
    } else {
        double t = -b / a;
        c = 1.0 / qSqrt(1.0 + t * t);
        s = c * t;
    }
}

/** @brief 左乘Givens旋转 @param M 矩阵 @param i 行1 @param j 行2 @param c 余弦 @param s 正弦 @param colStart 列起始 @param colEnd 列结束 */
void GeneralizedEigen::applyGivensLeft(QVector<QVector<double>>& M,
                                        int i, int j,
                                        double c, double s,
                                        int colStart, int colEnd)
{
    for (int k = colStart; k < colEnd; ++k) {
        double t1 = c * M[i][k] - s * M[j][k];
        double t2 = s * M[i][k] + c * M[j][k];
        M[i][k] = t1;
        M[j][k] = t2;
    }
}

/** @brief 右乘Givens旋转 @param M 矩阵 @param i 列1 @param j 列2 @param c 余弦 @param s 正弦 @param rowStart 行起始 @param rowEnd 行结束 */
void GeneralizedEigen::applyGivensRight(QVector<QVector<double>>& M,
                                         int i, int j,
                                         double c, double s,
                                         int rowStart, int rowEnd)
{
    for (int k = rowStart; k < rowEnd; ++k) {
        double t1 = M[k][i] * c + M[k][j] * s;
        double t2 = -M[k][i] * s + M[k][j] * c;
        M[k][i] = t1;
        M[k][j] = t2;
    }
}

/** @brief 生成单位矩阵 @param n 尺寸 @return 单位矩阵 */
QVector<QVector<double>> GeneralizedEigen::identityMatrix(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}
