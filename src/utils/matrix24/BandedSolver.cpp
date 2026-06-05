/**
 * @file BandedSolver.cpp
 * @brief 带状矩阵求解器实现 — LU分解/Thomas三对角/紧致存储
 */
#include "utils/matrix24/BandedSolver.h"
#include <QElapsedTimer>
#include <cmath>

BandedSolver::BandedSolver(QObject* parent)
    : QObject(parent), m_n(0), m_lowerBand(0), m_upperBand(0), m_factored(false)
{
}

QVector<double> BandedSolver::solveTridiagonal(const QVector<double>& lower,
                                               const QVector<double>& main,
                                               const QVector<double>& upper,
                                               const QVector<double>& rhs)
{
    int n = main.size();
    if (n == 0) return {};
    QElapsedTimer timer; timer.start();
    QVector<double> result = thomasAlgorithm(lower, main, upper, rhs);
    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveComplete(n, 1);
    return result;
}

QVector<double> BandedSolver::thomasAlgorithm(const QVector<double>& a,
                                              const QVector<double>& b,
                                              const QVector<double>& c,
                                              const QVector<double>& d)
{
    int n = b.size();
    if (n == 0) return {};
    QVector<double> cp(n, 0.0), dp(n, 0.0), x(n, 0.0);
    cp[0] = (n > 1 && std::abs(b[0]) > 1e-15) ? c[0] / b[0] : 0.0;
    dp[0] = (std::abs(b[0]) > 1e-15) ? d[0] / b[0] : 0.0;
    for (int i = 1; i < n; ++i) {
        double ai = (i - 1 < a.size()) ? a[i - 1] : 0.0;
        double ci = (i < c.size()) ? c[i] : 0.0;
        double denom = b[i] - ai * cp[i - 1];
        if (std::abs(denom) < 1e-15) denom = 1e-15;
        cp[i] = ci / denom;
        dp[i] = (d[i] - ai * dp[i - 1]) / denom;
    }
    x[n - 1] = dp[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dp[i] - cp[i] * x[i + 1];
    return x;
}

void BandedSolver::setBandedMatrix(const QVector<QVector<double>>& bands,
                                   int n, int lowerBand, int upperBand)
{
    m_n = n; m_lowerBand = lowerBand; m_upperBand = upperBand;
    m_bands = bands; m_factored = false;
}

bool BandedSolver::factorize()
{
    if (m_n <= 0 || m_bands.isEmpty()) return false;
    QElapsedTimer timer; timer.start();
    int totalBand = m_lowerBand + m_upperBand + 1;
    for (int k = 0; k < m_n; ++k) {
        int jEnd = qMin(k + m_upperBand, m_n - 1);
        for (int j = k; j <= jEnd; ++j) {
            int bj = j - k + m_lowerBand;
            if (bj < 0 || bj >= totalBand) continue;
            double sum = 0.0;
            int iStart = qMax(0, j - m_upperBand);
            for (int m = iStart; m < k; ++m) {
                int bm1 = m - k + m_lowerBand;
                int bm2 = j - m + m_lowerBand;
                if (bm1 >= 0 && bm1 < totalBand && bm2 >= 0 && bm2 < totalBand)
                    sum += m_bands[k][bm1] * m_bands[m][bm2];
            }
            m_bands[k][bj] -= sum;
        }
        if (std::abs(m_bands[k][m_lowerBand]) < 1e-15) {
            m_factored = false;
            emit factorizationComplete(m_n, false);
            return false;
        }
        int iEnd = qMin(k + m_lowerBand, m_n - 1);
        for (int i = k + 1; i <= iEnd; ++i) {
            int bi = k - i + m_lowerBand;
            if (bi < 0 || bi >= totalBand) continue;
            double sum = 0.0;
            int mStart = qMax(0, i - m_lowerBand);
            for (int m = mStart; m < k; ++m) {
                int bm1 = m - i + m_lowerBand;
                int bm2 = k - m + m_lowerBand;
                if (bm1 >= 0 && bm1 < totalBand && bm2 >= 0 && bm2 < totalBand)
                    sum += m_bands[i][bm1] * m_bands[m][bm2];
            }
            m_bands[i][bi] = (m_bands[i][bi] - sum) / m_bands[k][m_lowerBand];
        }
    }
    m_factored = true;
    m_stats.totalFactorizations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves + m_stats.totalFactorizations > 0)
        ? m_timeSum / (m_stats.totalSolves + m_stats.totalFactorizations) : 0.0;
    emit factorizationComplete(m_n, true);
    return true;
}

QVector<double> BandedSolver::solve(const QVector<double>& rhs)
{
    if (!m_factored || m_n <= 0) return {};
    QElapsedTimer timer; timer.start();
    int totalBand = m_lowerBand + m_upperBand + 1;
    QVector<double> x = rhs;
    for (int i = 1; i < m_n; ++i) {
        int jStart = qMax(0, i - m_lowerBand);
        for (int j = jStart; j < i; ++j) {
            int bj = j - i + m_lowerBand;
            if (bj >= 0 && bj < totalBand)
                x[i] -= m_bands[i][bj] * x[j];
        }
    }
    for (int i = m_n - 1; i >= 0; --i) {
        int jEnd = qMin(m_n - 1, i + m_upperBand);
        for (int j = i + 1; j <= jEnd; ++j) {
            int bj = j - i + m_lowerBand;
            if (bj >= 0 && bj < totalBand)
                x[i] -= m_bands[i][bj] * x[j];
        }
        x[i] /= m_bands[i][m_lowerBand];
    }
    m_stats.totalSolves++;
    m_stats.totalSystemsSize += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalSolves + m_stats.totalFactorizations);
    emit solveComplete(m_n, 1);
    return x;
}

QList<QVector<double>> BandedSolver::solveMultiple(const QList<QVector<double>>& rhsList)
{
    QList<QVector<double>> results;
    for (const auto& rhs : rhsList)
        results.append(m_factored ? solve(rhs) : QVector<double>{});
    return results;
}

QVector<QVector<double>> BandedSolver::factoredMatrix() const
{
    return m_bands;
}

void BandedSolver::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
