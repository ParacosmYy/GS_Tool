/**
 * @file DavidsonEigen.cpp
 * @brief Davidson方法实现 — 大型稀疏矩阵特征值求解
 */

#include "utils/davidson/DavidsonEigen.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
DavidsonEigen::DavidsonEigen(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 求解最小特征值 */
QVector<double> DavidsonEigen::solve(const MatVecFunc& matvec,
                                     int n,
                                     int numEigen,
                                     int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || numEigen <= 0) return {};

    numEigen = qMin(numEigen, n);

    /* 初始猜测: 单位向量的叠加 */
    QVector<QVector<double>> basis;
    for (int k = 0; k < numEigen; ++k) {
        QVector<double> v(n, 0.0);
        /* 选择均匀分布的初始向量 */
        int idx = k * n / numEigen;
        idx = qMin(idx, n - 1);
        v[idx] = 1.0;
        basis.append(v);
    }

    /* 子空间投影矩阵 */
    int subSize = basis.size();
    QVector<QVector<double>> H(subSize, QVector<double>(subSize, 0.0));
    for (int i = 0; i < subSize; ++i) {
        QVector<double> Avi = matvec(basis[i]);
        for (int j = 0; j <= i; ++j) {
            QVector<double> Avj = matvec(basis[j]);
            double hij = dot(Avi, basis[j]);
            H[i][j] = hij;
            H[j][i] = hij;
        }
    }

    QVector<double> eigenvalues;
    int iter = 0;

    for (; iter < maxIter; ++iter) {
        /* 求解子空间特征值问题 */
        QVector<QVector<double>> eigvecs;
        eigenvalues = smallEigenSolve(H, numEigen, eigvecs);

        /* 计算Ritz向量: x = V * s */
        QVector<QVector<double>> ritzVectors(numEigen);
        for (int k = 0; k < numEigen; ++k) {
            ritzVectors[k].resize(n, 0.0);
            for (int j = 0; j < subSize; ++j) {
                for (int i = 0; i < n; ++i) {
                    ritzVectors[k][i] += eigvecs[j][k] * basis[j][i];
                }
            }
        }

        /* 计算残差: r = A*x - theta*x */
        double maxResidual = 0.0;
        QVector<QVector<double>> newDirections;
        for (int k = 0; k < numEigen; ++k) {
            QVector<double> Ax = matvec(ritzVectors[k]);
            double theta = eigenvalues[k];
            QVector<double> residual(n);
            for (int i = 0; i < n; ++i) {
                residual[i] = Ax[i] - theta * ritzVectors[k][i];
            }

            double resNorm = vecNorm(residual);
            if (resNorm > maxResidual) maxResidual = resNorm;

            /* Davidson修正: 预条件 t = (diag(A)-theta*I)^{-1} * r */
            /* 近似: 使用1.0/(theta + 1.0)作为标量预条件 */
            if (resNorm > 1e-10) {
                QVector<double> t = residual;
                /* 对角预条件近似 */
                for (int i = 0; i < n; ++i) {
                    double d = theta - 1.0;
                    if (qAbs(d) < 1e-15) d = 1e-15;
                    t[i] /= d;
                }
                orthogonalizeAgainst(t, basis);
                double tNorm = vecNorm(t);
                if (tNorm > 1e-15) {
                    for (auto& val : t) val /= tNorm;
                    newDirections.append(t);
                }
            }
        }

        /* 收敛判断 */
        if (maxResidual < 1e-8) break;

        /* 扩展子空间 */
        for (auto& dir : newDirections) {
            if (subSize + 1 > qMin(n, maxIter)) {
                /* 子空间过大: 重启 */
                basis.clear();
                for (int k = 0; k < numEigen; ++k) {
                    basis.append(ritzVectors[k]);
                }
                subSize = basis.size();
                H = QVector<QVector<double>>(subSize,
                    QVector<double>(subSize, 0.0));
                for (int i = 0; i < subSize; ++i) {
                    QVector<double> Avi = matvec(basis[i]);
                    for (int j = 0; j <= i; ++j) {
                        double hij = dot(Avi, basis[j]);
                        H[i][j] = hij;
                        H[j][i] = hij;
                    }
                }
                break;
            }

            basis.append(dir);
            /* 扩展H */
            int newIdx = subSize;
            for (auto& row : H) row.append(0.0);
            QVector<double> AvNew = matvec(basis[newIdx]);
            H.append(QVector<double>(newIdx + 2, 0.0));
            for (int j = 0; j <= newIdx; ++j) {
                double hij = dot(AvNew, basis[j]);
                H[newIdx][j] = hij;
                H[j][newIdx] = hij;
            }
            ++subSize;
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(eigenvalues.size(), iter);
    return eigenvalues;
}

/** @brief 向量点积 */
double DavidsonEigen::dot(const QVector<double>& a,
                          const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/** @brief 向量范数 */
double DavidsonEigen::vecNorm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/** @brief 正交化向量对已有基 */
void DavidsonEigen::orthogonalizeAgainst(
    QVector<double>& v, const QVector<QVector<double>>& basis)
{
    for (const auto& b : basis) {
        double proj = dot(v, b);
        int n = qMin(v.size(), b.size());
        for (int i = 0; i < n; ++i) {
            v[i] -= proj * b[i];
        }
    }
}

/** @brief 小规模对称特征值问题(Jacobi迭代) */
QVector<double> DavidsonEigen::smallEigenSolve(
    const QVector<QVector<double>>& mat, int numEigen,
    QVector<QVector<double>>& eigvecs)
{
    int n = mat.size();
    if (n == 0) return {};

    /* 复制矩阵 */
    QVector<QVector<double>> A = mat;

    /* 初始化特征向量矩阵 = 单位矩阵 */
    eigvecs = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) eigvecs[i][i] = 1.0;

    /* Jacobi旋转消去非对角元素 */
    for (int sweep = 0; sweep < 100; ++sweep) {
        double offDiag = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                offDiag += A[i][j] * A[i][j];

        if (offDiag < 1e-20) break;

        for (int p = 0; p < n - 1; ++p) {
            for (int q = p + 1; q < n; ++q) {
                if (qAbs(A[p][q]) < 1e-15) continue;

                /* 计算旋转角 */
                double app = A[p][p];
                double aqq = A[q][q];
                double apq = A[p][q];
                double theta = (aqq - app) / (2.0 * apq);
                double t = 1.0;
                if (qAbs(theta) > 1e-15) {
                    t = (theta > 0 ? 1.0 : -1.0)
                        / (qAbs(theta) + qSqrt(1.0 + theta * theta));
                }
                double c = 1.0 / qSqrt(1.0 + t * t);
                double s = t * c;

                /* 更新矩阵 */
                A[p][p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
                A[q][q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
                A[p][q] = 0.0;
                A[q][p] = 0.0;

                for (int r = 0; r < n; ++r) {
                    if (r == p || r == q) continue;
                    double arp = A[r][p];
                    double arq = A[r][q];
                    A[r][p] = c * arp - s * arq;
                    A[p][r] = A[r][p];
                    A[r][q] = s * arp + c * arq;
                    A[q][r] = A[r][q];
                }

                /* 更新特征向量 */
                for (int r = 0; r < n; ++r) {
                    double erp = eigvecs[r][p];
                    double erq = eigvecs[r][q];
                    eigvecs[r][p] = c * erp - s * erq;
                    eigvecs[r][q] = s * erp + c * erq;
                }
            }
        }
    }

    /* 提取特征值并排序 */
    QVector<QPair<double, int>> eigenPairs(n);
    for (int i = 0; i < n; ++i) {
        eigenPairs[i] = {A[i][i], i};
    }
    std::sort(eigenPairs.begin(), eigenPairs.end());

    /* 重排特征向量 */
    QVector<QVector<double>> sortedEigvecs(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        int origIdx = eigenPairs[i].second;
        for (int r = 0; r < n; ++r) {
            sortedEigvecs[r][i] = eigvecs[r][origIdx];
        }
    }
    eigvecs = sortedEigvecs;

    QVector<double> evals(qMin(numEigen, n));
    for (int i = 0; i < evals.size(); ++i) {
        evals[i] = eigenPairs[i].first;
    }
    return evals;
}

/** @brief 重置统计 */
void DavidsonEigen::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
