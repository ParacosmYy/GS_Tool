/**
 * @file TridiagonalSolver6.cpp
 * @brief TridiagonalSolver6 实现
 *
 * 实现三对角求解器：抛物线循环约化与向量化扫描GPU友好并行。
 */

#include "utils/matrix261/TridiagonalSolver6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TridiagonalSolver6::TridiagonalSolver6(QObject *parent)
    : QObject(parent) {}
TridiagonalSolver6::~TridiagonalSolver6() = default;

/* ---- Configuration ---- */

void TridiagonalSolver6::setSystem(const System& sys)
{
    m_system = sys;
    m_n = sys.main.size();
}

void TridiagonalSolver6::setSystem(const QVector<double>& lower,
                                    const QVector<double>& main,
                                    const QVector<double>& upper,
                                    const QVector<double>& rhs)
{
    m_system.lower = lower;
    m_system.main = main;
    m_system.upper = upper;
    m_system.rhs = rhs;
    m_n = main.size();
}

/* ---- Validate system ---- */

bool TridiagonalSolver6::validateSystem() const
{
    if (m_n < 2) return false;
    if (m_system.main.size() != m_n) return false;
    if (m_system.rhs.size() != m_n) return false;
    if (m_system.lower.size() != m_n - 1) return false;
    if (m_system.upper.size() != m_n - 1) return false;
    return true;
}

/* ---- Pad to power of 2 ---- */

int TridiagonalSolver6::padToPowerOf2()
{
    int p = 1;
    while (p < m_n) p <<= 1;
    if (p == m_n) return p;

    // Pad with identity rows (no effect on solution)
    int oldN = m_n;
    m_system.lower.resize(p - 1, 0.0);
    m_system.main.resize(p, 1.0);
    m_system.upper.resize(p - 1, 0.0);
    m_system.rhs.resize(p, 0.0);

    // Fill padded rows with identity
    for (int i = oldN; i < p; ++i) {
        m_system.main[i] = 1.0;
        m_system.rhs[i] = 0.0;
        if (i > 0) m_system.lower[i - 1] = 0.0;
        if (i < p - 1) m_system.upper[i] = 0.0;
    }
    m_n = p;
    return p;
}

/* ---- PCR reduction step ---- */

void TridiagonalSolver6::pcrReduceStage(int stage, int stride)
{
    ReductionStage& cur = m_stages[stage];
    const ReductionStage& prev = m_stages[stage - 1];
    int n = prev.a.size();

    cur.a.resize(n);
    cur.b.resize(n);
    cur.c.resize(n);
    cur.d.resize(n);

    // Parallel sweep: each equation eliminates neighbors at distance 'stride'
    for (int i = 0; i < n; ++i) {
        int leftIdx = i - stride;
        int rightIdx = i + stride;

        // Coefficients from current row
        double ai = prev.a[i]; // sub-diagonal
        double bi = prev.b[i]; // main diagonal
        double ci = prev.c[i]; // super-diagonal
        double di = prev.d[i]; // RHS

        double k1 = 0.0, k2 = 0.0;
        double b1 = 1.0, b2 = 1.0;
        double c1 = 0.0, a2 = 0.0;
        double d1 = 0.0, d2 = 0.0;

        // Eliminate left neighbor
        if (leftIdx >= 0) {
            double denom = qMax(qFabs(prev.b[leftIdx]), 1e-300);
            k1 = ai / denom;
            a1 = 0.0;  // NOLINT
            b1 = bi - k1 * prev.c[leftIdx];
            c1 = ci - k1 * 0.0;  // simplified
            d1 = di - k1 * prev.d[leftIdx];
            ai = 0.0;
            bi = b1;
            ci = c1;
            di = d1;
        }

        // Eliminate right neighbor
        if (rightIdx < n) {
            double denom = qMax(qFabs(prev.b[rightIdx]), 1e-300);
            k2 = ci / denom;
            double a2val = -k2 * prev.a[rightIdx];
            double b2val = bi - k2 * prev.c[rightIdx];
            double d2val = di - k2 * prev.d[rightIdx];
            ai = (leftIdx >= 0) ? 0.0 : ai;
            bi = b2val;
            ci = 0.0;
            di = d2val;
        }

        cur.a[i] = ai;
        cur.b[i] = qMax(qFabs(bi), 1e-300) * (bi >= 0 ? 1.0 : -1.0);
        cur.c[i] = ci;
        cur.d[i] = di;
    }
}

/* ---- Back-substitute from PCR stages ---- */

QVector<double> TridiagonalSolver6::pcrBackSubstitute() const
{
    int n = m_n;
    QVector<double> x(n, 0.0);

    // After full PCR, each equation is decoupled: b[i]*x[i] = d[i]
    const ReductionStage& last = m_stages[m_stages.size() - 1];
    for (int i = 0; i < n; ++i) {
        if (qFabs(last.b[i]) > 1e-300)
            x[i] = last.d[i] / last.b[i];
    }
    return x;
}

/* ---- Solve using PCR ---- */

QVector<double> TridiagonalSolver6::solvePCR()
{
    QElapsedTimer timer;
    timer.start();

    if (!validateSystem()) return {};

    int paddedN = padToPowerOf2();
    int numStages = 0;
    int s = paddedN;
    while (s > 1) { s >>= 1; numStages++; }

    // Stage 0: copy original system
    m_stages.resize(numStages + 1);
    m_stages[0].a = m_system.lower;
    m_stages[0].a.resize(paddedN, 0.0);       // Pad a to n
    m_stages[0].b = m_system.main;
    m_stages[0].c = m_system.upper;
    m_stages[0].c.resize(paddedN, 0.0);        // Pad c to n
    m_stages[0].d = m_system.rhs;

    // Insert dummy a[0] and c[n-1]
    m_stages[0].a.insert(0, 0.0);
    m_stages[0].c.append(0.0);

    // Run reduction stages
    int stride = 1;
    for (int stage = 1; stage <= numStages; ++stage) {
        pcrReduceStage(stage, stride);
        stride <<= 1;
    }

    QVector<double> x = pcrBackSubstitute();

    // Trim to original size
    int origN = m_system.main.size();
    if (x.size() > origN)
        x.resize(origN);

    double res = residual(x);
    double elapsed = timer.elapsed();

    m_stats.systemSize = origN;
    m_stats.numReductionStages = numStages;
    m_stats.residualNorm = res;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(origN, res, elapsed);
    return x;
}

/* ---- Solve using Thomas algorithm (sequential) ---- */

QVector<double> TridiagonalSolver6::solveThomas()
{
    QElapsedTimer timer;
    timer.start();

    if (!validateSystem()) return {};

    int n = m_n;
    QVector<double> a = m_system.lower;
    QVector<double> b = m_system.main;
    QVector<double> c = m_system.upper;
    QVector<double> d = m_system.rhs;

    // Forward sweep
    for (int i = 1; i < n; ++i) {
        double m = a[i - 1] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }

    // Back substitution
    QVector<double> x(n);
    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];

    double res = residual(x);
    double elapsed = timer.elapsed();

    m_stats.systemSize = n;
    m_stats.residualNorm = res;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(n, res, elapsed);
    return x;
}

/* ---- Compute residual ---- */

double TridiagonalSolver6::residual(const QVector<double>& x) const
{
    int n = qMin(m_n, x.size());
    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = m_system.rhs[i] - m_system.main[i] * x[i];
        if (i > 0) r += m_system.lower[i - 1] * x[i - 1];
        if (i < n - 1) r += m_system.upper[i] * x[i + 1];
        norm += r * r;
    }
    return qSqrt(norm);
}

/* ---- Reset ---- */

void TridiagonalSolver6::resetStatistics()
{
    m_system = System{};
    m_stages.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
