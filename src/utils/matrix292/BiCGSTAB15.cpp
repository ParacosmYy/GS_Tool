/**
 * @file BiCGSTAB15.cpp
 * @brief BiCGSTAB15 实现
 *
 * 实现BiCGSTAB求解器：可变预条件器切换与残差平滑的双共轭梯度稳定收敛。
 */

#include "utils/matrix292/BiCGSTAB15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BiCGSTAB15::BiCGSTAB15(QObject *parent)
    : QObject(parent) {}

BiCGSTAB15::~BiCGSTAB15() = default;

/* ---- Configuration ---- */

void BiCGSTAB15::setMaxIter(int iters) { m_maxIter = qBound(10, iters, 100000); }
void BiCGSTAB15::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }
void BiCGSTAB15::setPrecond(PrecondType type) { m_precond = type; m_iluReady = false; }

/* ---- Set matrix (dense) ---- */

void BiCGSTAB15::setMatrix(const QVector<QVector<double>>& mat)
{
    m_mat = mat;
    m_n = mat.size();
    m_iluReady = false;
    m_stats.matrixSize = m_n;
    m_stats.nonZeros = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            if (qAbs(m_mat[i][j]) > 1e-15) m_stats.nonZeros++;
}

/* ---- Set matrix (sparse COO) ---- */

void BiCGSTAB15::setMatrixSparse(const QVector<SparseEntry>& entries, int n)
{
    m_n = n;
    m_mat = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (const auto& e : entries) {
        if (e.row >= 0 && e.row < n && e.col >= 0 && e.col < n)
            m_mat[e.row][e.col] = e.value;
    }
    m_iluReady = false;
    m_stats.matrixSize = n;
    m_stats.nonZeros = entries.size();
}

/* ---- Matrix-vector product ---- */

QVector<double> BiCGSTAB15::matVec(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            y[i] += m_mat[i][j] * x[j];
    return y;
}

/* ---- Dot product / norm ---- */

double BiCGSTAB15::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    for (int i = 0; i < a.size(); ++i) s += a[i] * b[i];
    return s;
}

double BiCGSTAB15::norm(const QVector<double>& v) const { return qSqrt(dot(v, v)); }

/* ---- Build ILU(0) factorization ---- */

void BiCGSTAB15::buildILU()
{
    m_iluL = QVector<QVector<double>>(m_n, QVector<double>(m_n, 0.0));
    m_iluU = QVector<QVector<double>>(m_n, QVector<double>(m_n, 0.0));

    // Copy A into U, set L to identity
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) m_iluU[i][j] = m_mat[i][j];
        m_iluL[i][i] = 1.0;
    }

    // IKJ Gaussian elimination
    for (int k = 0; k < m_n; ++k) {
        if (qAbs(m_iluU[k][k]) < 1e-15) m_iluU[k][k] = 1e-10;
        for (int i = k + 1; i < m_n; ++i) {
            m_iluL[i][k] = m_iluU[i][k] / m_iluU[k][k];
            for (int j = k; j < m_n; ++j)
                m_iluU[i][j] -= m_iluL[i][k] * m_iluU[k][j];
        }
    }
    m_iluReady = true;
}

/* ---- Jacobi preconditioner ---- */

QVector<double> BiCGSTAB15::jacobiSolve(const QVector<double>& r) const
{
    QVector<double> z(m_n);
    for (int i = 0; i < m_n; ++i) {
        double diag = m_mat[i][i];
        z[i] = (qAbs(diag) > 1e-15) ? r[i] / diag : r[i];
    }
    return z;
}

/* ---- ILU forward-backward solve ---- */

QVector<double> BiCGSTAB15::iluSolve(const QVector<double>& r) const
{
    // Forward: L*y = r
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double s = r[i];
        for (int j = 0; j < i; ++j) s -= m_iluL[i][j] * y[j];
        y[i] = s / m_iluL[i][i];
    }
    // Backward: U*x = y
    QVector<double> x(m_n, 0.0);
    for (int i = m_n - 1; i >= 0; --i) {
        double s = y[i];
        for (int j = i + 1; j < m_n; ++j) s -= m_iluU[i][j] * x[j];
        x[i] = s / m_iluU[i][i];
    }
    return x;
}

/* ---- Apply preconditioner ---- */

QVector<double> BiCGSTAB15::applyPrecond(const QVector<double>& r) const
{
    switch (m_precond) {
    case PrecondType::Jacobi: return jacobiSolve(r);
    case PrecondType::ILU0:
    case PrecondType::SGSType:
        return const_cast<BiCGSTAB15*>(this)->iluSolve(r);
    default: return r;
    }
}

/* ---- Auto-switch preconditioner ---- */

BiCGSTAB15::PrecondType BiCGSTAB15::autoSwitch(const QVector<double>& resHist) const
{
    if (resHist.size() < 10) return m_precond;
    // Check for stagnation: last 5 residuals barely changed
    double recent = 0.0;
    for (int i = resHist.size() - 5; i < resHist.size(); ++i)
        recent += qAbs(resHist[i] - resHist[i - 1]);
    recent /= 5.0;
    if (recent < m_tol * 0.01) {
        return (m_precond == PrecondType::Jacobi) ? PrecondType::ILU0 : PrecondType::Jacobi;
    }
    return m_precond;
}

/* ---- Solve Ax = b ---- */

BiCGSTAB15::SolveResult BiCGSTAB15::solve(const QVector<double>& b)
{
    QVector<double> x0(m_n, 0.0);
    return solveWithGuess(b, x0);
}

/* ---- Solve with initial guess ---- */

BiCGSTAB15::SolveResult BiCGSTAB15::solveWithGuess(const QVector<double>& b,
                                                      const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    if (m_n == 0 || b.size() != m_n) return result;

    // Build preconditioner if needed
    if (m_precond == PrecondType::ILU0 || m_precond == PrecondType::SGSType) {
        if (!m_iluReady) buildILU();
    }

    QVector<double> x = x0;
    QVector<double> r = b;  // r = b - A*x0
    QVector<double> ax = matVec(x0);
    for (int i = 0; i < m_n; ++i) r[i] = b[i] - ax[i];

    double rNorm = norm(r);
    double bNorm = norm(b);
    if (bNorm < 1e-15) bNorm = 1.0;
    result.initialResidual = rNorm / bNorm;

    // Choose r~ = r (shadow residual)
    QVector<double> rHat = r;

    QVector<double> p = r;
    QVector<double> v(m_n, 0.0);

    QVector<double> resHistory;
    resHistory.append(rNorm);

    // Smoothed solution tracking
    QVector<double> xSmooth = x;
    double rNormSmooth = rNorm;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Precondition: solve M*y = p
        QVector<double> y = applyPrecond(p);
        v = matVec(y);

        double rho = dot(rHat, r);
        if (qAbs(rho) < 1e-300) break;

        double alpha = rho / dot(rHat, v);

        // s = r - alpha*v
        QVector<double> s(m_n);
        for (int i = 0; i < m_n; ++i) s[i] = r[i] - alpha * v[i];

        // Precondition: solve M*z = s
        QVector<double> z = applyPrecond(s);
        QVector<double> t = matVec(z);

        double omega = dot(t, s) / (dot(t, t) + 1e-300);

        // Update x and r
        for (int i = 0; i < m_n; ++i) {
            x[i] += alpha * y[i] + omega * z[i];
            r[i] = s[i] - omega * t[i];
        }

        rNorm = norm(r);
        resHistory.append(rNorm);

        // Residual smoothing
        if (rNorm < rNormSmooth) {
            xSmooth = x;
            rNormSmooth = rNorm;
        }

        if (rNorm / bNorm < m_tol) {
            result.converged = true;
            break;
        }

        // Auto-switch preconditioner if stagnating
        if (iter > 0 && iter % 20 == 0) {
            PrecondType newP = autoSwitch(resHistory);
            if (newP != m_precond) {
                m_precond = newP;
                if (m_precond == PrecondType::ILU0 && !m_iluReady) buildILU();
            }
        }

        // Update p for next iteration
        double rhoNew = dot(rHat, r);
        double beta = (rhoNew / rho) * (alpha / (omega + 1e-300));
        for (int i = 0; i < m_n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
    }

    result.x = xSmooth;
    result.residualNorm = rNormSmooth / bNorm;
    result.iterations = resHistory.size() - 1;
    result.usedPrecond = m_precond;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveDone(result.iterations, result.residualNorm, elapsed);
    return result;
}

/* ---- Reset ---- */

void BiCGSTAB15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_mat.clear();
    m_iluL.clear();
    m_iluU.clear();
    m_iluReady = false;
    m_n = 0;
}
