/**
 * @file EigenVectorSolver7.cpp
 * @brief EigenVectorSolver7 实现
 *
 * 实现特征向量求解器：Davidson方法与预处理校正方程内部特征值计算。
 */

#include "utils/matrix265/EigenVectorSolver7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EigenVectorSolver7::EigenVectorSolver7(QObject *parent)
    : QObject(parent) {}
EigenVectorSolver7::~EigenVectorSolver7() = default;

/* ---- Configuration ---- */

void EigenVectorSolver7::setNumEigenvalues(int num) { m_numEigenvalues = qMax(1, num); }
void EigenVectorSolver7::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void EigenVectorSolver7::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void EigenVectorSolver7::setMaxSubspaceSize(int size) { m_maxSubspace = qMax(2, size); }

/* ---- Vector math ---- */

double EigenVectorSolver7::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double EigenVectorSolver7::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

QVector<double> EigenVectorSolver7::matVec(const QVector<QVector<double>>& mat,
                                             const QVector<double>& vec) const
{
    int n = mat.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += mat[i][j] * vec[j];
    return result;
}

/* ---- Preconditioned correction ---- */

QVector<double> EigenVectorSolver7::precondition(const QVector<QVector<double>>& mat,
                                                   const QVector<double>& residual,
                                                   double shift) const
{
    // Diagonal preconditioner: t_i = r_i / (A_ii - sigma)
    int n = residual.size();
    QVector<double> t(n);
    for (int i = 0; i < n; ++i) {
        double diag = mat[i][i] - shift;
        t[i] = (qAbs(diag) > 1e-14) ? residual[i] / diag : residual[i];
    }
    return t;
}

/* ---- Orthonormalize via modified Gram-Schmidt ---- */

void EigenVectorSolver7::orthonormalize(QVector<double>& vec,
                                          const QVector<QVector<double>>& basis) const
{
    for (const auto& b : basis) {
        double d = dot(vec, b);
        for (int i = 0; i < vec.size(); ++i)
            vec[i] -= d * b[i];
    }
    double n = norm(vec);
    if (n > 1e-14)
        for (int i = 0; i < vec.size(); ++i) vec[i] /= n;
}

/* ---- Solve small dense eigenvalue via QR iteration ---- */

EigenVectorSolver7::EigenPair EigenVectorSolver7::solveSmallDense(
    const QVector<QVector<double>>& smallMat) const
{
    int n = smallMat.size();
    if (n == 0) return {};

    // Copy to working matrix
    QVector<QVector<double>> H = smallMat;

    // Simple QR iteration for smallest eigenvalue
    for (int iter = 0; iter < 100; ++iter) {
        // QR decomposition via Householder
        for (int k = 0; k < n - 1; ++k) {
            double norm_val = 0.0;
            for (int i = k; i < n; ++i) norm_val += H[i][k] * H[i][k];
            norm_val = qSqrt(norm_val);
            if (norm_val < 1e-15) continue;

            double sign = (H[k][k] >= 0) ? 1.0 : -1.0;
            double alpha = sign * norm_val;

            QVector<double> v(n, 0.0);
            v[k] = H[k][k] + alpha;
            for (int i = k + 1; i < n; ++i) v[i] = H[i][k];
            double vNorm = norm(v);
            if (vNorm < 1e-15) continue;
            for (int i = 0; i < n; ++i) v[i] /= vNorm;

            // Apply Householder: H = (I - 2*v*v^T) * H
            for (int col = 0; col < n; ++col) {
                double d = 0.0;
                for (int row = 0; row < n; ++row) d += v[row] * H[row][col];
                for (int row = 0; row < n; ++row) H[row][col] -= 2.0 * v[row] * d;
            }
            // Apply to right: H = H * (I - 2*v*v^T)
            for (int row = 0; row < n; ++row) {
                double d = 0.0;
                for (int col = 0; col < n; ++col) d += H[row][col] * v[col];
                for (int col = 0; col < n; ++col) H[row][col] -= 2.0 * d * v[col];
            }
        }
    }

    // Extract smallest diagonal element as eigenvalue
    EigenPair result;
    result.eigenvalue = H[0][0];
    int minIdx = 0;
    for (int i = 1; i < n; ++i) {
        if (qAbs(H[i][i]) < qAbs(result.eigenvalue)) {
            result.eigenvalue = H[i][i];
            minIdx = i;
        }
    }
    result.eigenvector = QVector<double>(n, 0.0);
    result.eigenvector[minIdx] = 1.0;
    return result;
}

/* ---- Compute residual ---- */

QVector<double> EigenVectorSolver7::computeResidual(
    const QVector<QVector<double>>& mat, const EigenPair& pair) const
{
    int n = mat.size();
    QVector<double> Av = matVec(mat, pair.eigenvector);
    QVector<double> residual(n);
    for (int i = 0; i < n; ++i)
        residual[i] = Av[i] - pair.eigenvalue * pair.eigenvector[i];
    return residual;
}

/* ---- Solve (Davidson method) ---- */

QVector<EigenVectorSolver7::EigenPair> EigenVectorSolver7::solve(
    const QVector<QVector<double>>& matrix)
{
    return solveShifted(matrix, 0.0);
}

QVector<EigenVectorSolver7::EigenPair> EigenVectorSolver7::solveShifted(
    const QVector<QVector<double>>& matrix, double shift)
{
    QElapsedTimer timer;
    timer.start();

    m_n = matrix.size();
    if (m_n == 0) return {};

    QVector<EigenPair> results;
    QVector<double> deflatedDiag;
    for (int i = 0; i < m_n; ++i) deflatedDiag.append(matrix[i][i]);

    for (int ev = 0; ev < m_numEigenvalues && ev < m_n; ++ev) {
        QElapsedTimer evTimer;
        evTimer.start();

        // Initial vector: unit vector along smallest diagonal element
        QVector<double> v(m_n, 0.0);
        int minDiagIdx = 0;
        for (int i = 1; i < m_n; ++i)
            if (qAbs(deflatedDiag[i] - shift) < qAbs(deflatedDiag[minDiagIdx] - shift))
                minDiagIdx = i;
        v[minDiagIdx] = 1.0;

        // Davidson iteration
        QVector<QVector<double>> basis = {v};
        double bestEigenvalue = 0.0;
        double bestResidual = std::numeric_limits<double>::max();
        QVector<double> bestEigenvector = v;

        for (int iter = 0; iter < m_maxIter; ++iter) {
            // Build and solve subspace eigenvalue problem
            int m = basis.size();
            QVector<QVector<double>> subMat(m, QVector<double>(m, 0.0));
            for (int i = 0; i < m; ++i) {
                QVector<double> Abi = matVec(matrix, basis[i]);
                for (int j = 0; j < m; ++j)
                    subMat[i][j] = dot(Abi, basis[j]);
            }

            EigenPair pair = solveSmallDense(subMat);

            // Reconstruct full eigenvector
            QVector<double> fullVec(m_n, 0.0);
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < m_n; ++j)
                    fullVec[j] += pair.eigenvector[i] * basis[i][j];
            double nrm = norm(fullVec);
            if (nrm > 1e-14)
                for (int j = 0; j < m_n; ++j) fullVec[j] /= nrm;

            pair.eigenvector = fullVec;
            pair.eigenvalue = pair.eigenvalue + shift;

            // Compute residual
            QVector<double> res = computeResidual(matrix, pair);
            double resNorm = norm(res);
            bestEigenvalue = pair.eigenvalue;
            bestResidual = resNorm;
            bestEigenvector = fullVec;

            emit iterationCompleted(iter, resNorm, m);

            if (resNorm < m_tolerance) break;

            // Preconditioned correction
            QVector<double> correction = precondition(matrix, res, shift);
            orthonormalize(correction, basis);

            if (norm(correction) > 1e-14) {
                basis.append(correction);
                // Restart if subspace too large
                if (basis.size() > m_maxSubspace) {
                    basis = {fullVec};
                }
            }
        }

        EigenPair found;
        found.eigenvalue = bestEigenvalue;
        found.eigenvector = bestEigenvector;
        found.residual = bestResidual;
        results.append(found);

        // Deflate: remove found eigenvalue from diagonal estimate
        deflatedDiag[minDiagIdx] = std::numeric_limits<double>::max();

        double evElapsed = evTimer.elapsed();
        emit eigenvalueFound(ev, bestEigenvalue, bestResidual, evElapsed);
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = m_n;
    m_stats.numEigenvaluesFound = results.size();
    m_stats.lastResidual = results.isEmpty() ? 0.0 : results.last().residual;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return results;
}

/* ---- Reset ---- */

void EigenVectorSolver7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
