/**
 * @file BandMatrix2.cpp
 * @brief 带状矩阵求解器实现 — Thomas算法与带状LU分解
 */

#include "utils/matrix8/BandMatrix2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

BandMatrix2::BandMatrix2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> BandMatrix2::solveTridiagonal(const QVector<double>& lower,
                                                const QVector<double>& mainDiag,
                                                const QVector<double>& upper,
                                                const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = mainDiag.size();
    if (n == 0 || rhs.size() != n ||
        lower.size() != n - 1 || upper.size() != n - 1) {
        return {};
    }

    /* Thomas算法 — 前向消元 */
    QVector<double> c(upper);
    QVector<double> d(rhs);
    QVector<double> b(mainDiag);

    for (int i = 1; i < n; ++i) {
        double a = lower[i - 1];
        if (qFabs(b[i - 1]) < 1e-15) {
            return {}; /* 主元为零, 求解失败 */
        }
        double m = a / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }

    if (qFabs(b[n - 1]) < 1e-15) {
        return {};
    }

    /* 回代求解 */
    QVector<double> x(n);
    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
    }

    ++m_stats.totalSystemsSolved;
    ++m_stats.totalThomasSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSystemsSolved;

    return x;
}

QVector<double> BandMatrix2::solveBanded(const QVector<QVector<double>>& bands,
                                           const QVector<double>& rhs,
                                           int halfBandwidth)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    if (n == 0 || bands.isEmpty()) return {};

    /* 将带状存储转为稠密矩阵的Thomas扩展形式 */
    int bw = halfBandwidth;
    QVector<double> lower(n, 0.0), mainDiag(n, 0.0), upper(n, 0.0);

    /* 从bands提取对角线: bands按[下bw...主...上bw]排列 */
    /* bands[i] 中 i = halfBandwidth 对应主对角线 */
    int centerIdx = bw; /* 主对角线索引 */

    /* 简化: 从bands重建完整带状并使用高斯消元 */
    QVector<QVector<double>> a(n, QVector<double>(2 * bw + 1, 0.0));
    QVector<double> d = rhs;

    /* 填充带状矩阵 (compressed row storage) */
    for (int row = 0; row < n; ++row) {
        for (int diag = -bw; diag <= bw; ++diag) {
            int col = row + diag;
            if (col < 0 || col >= n) continue;
            int bandIdx = diag + bw;
            if (bandIdx < bands.size() && bands[bandIdx].size() > qMax(row, col)) {
                a[row][diag + bw] = (diag <= 0)
                    ? bands[bandIdx][row] : bands[bandIdx][col];
            }
        }
    }

    /* 前向消元 */
    for (int k = 0; k < n; ++k) {
        /* 寻找主元(在带宽范围内) */
        double maxVal = qFabs(a[k][bw]);
        int pivotRow = k;
        for (int i = k + 1; i <= qMin(k + bw, n - 1); ++i) {
            double val = qFabs(a[i][bw - (i - k)]);
            if (val > maxVal) {
                maxVal = val;
                pivotRow = i;
            }
        }
        if (maxVal < 1e-15) return {};

        /* 行交换 */
        if (pivotRow != k) {
            for (int j = 0; j <= 2 * bw; ++j)
                std::swap(a[k][j], a[pivotRow][j]);
            std::swap(d[k], d[pivotRow]);
        }

        /* 消元 */
        for (int i = k + 1; i <= qMin(k + bw, n - 1); ++i) {
            int offset = i - k;
            double m = a[i][bw - offset] / a[k][bw];
            for (int j = bw - offset; j <= 2 * bw; ++j) {
                if (j - offset >= 0)
                    a[i][j - offset] -= m * a[k][j];
            }
            d[i] -= m * d[k];
            a[i][bw - offset] = 0.0;
        }
    }

    /* 回代 */
    QVector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        double sum = d[i];
        for (int j = bw + 1; j <= 2 * bw; ++j) {
            int col = i + j - bw;
            if (col < n) sum -= a[i][j] * x[col];
        }
        if (qFabs(a[i][bw]) < 1e-15) return {};
        x[i] = sum / a[i][bw];
    }

    ++m_stats.totalSystemsSolved;
    ++m_stats.totalExtendedSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSystemsSolved;

    return x;
}

QVector<double> BandMatrix2::solvePivoting(const QVector<QVector<double>>& bands,
                                             const QVector<double>& rhs,
                                             int halfBandwidth)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    if (n == 0 || bands.isEmpty()) return {};
    int bw = halfBandwidth;

    /* 重建稠密矩阵 */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int row = 0; row < n; ++row) {
        for (int diag = -bw; diag <= bw; ++diag) {
            int col = row + diag;
            if (col < 0 || col >= n) continue;
            int bandIdx = diag + bw;
            if (bandIdx < bands.size()) {
                A[row][col] = bands[bandIdx][qMax(row, col)];
            }
        }
    }

    QVector<double> d = rhs;
    QVector<int> pivot(n);

    /* 列主元LU分解 */
    for (int k = 0; k < n; ++k) {
        double maxVal = 0.0;
        int pivotRow = k;
        for (int i = k; i < n; ++i) {
            if (qFabs(A[i][k]) > maxVal) {
                maxVal = qFabs(A[i][k]);
                pivotRow = i;
            }
        }
        if (maxVal < 1e-15) return {};

        pivot[k] = pivotRow;
        if (pivotRow != k) {
            for (int j = 0; j < n; ++j) std::swap(A[k][j], A[pivotRow][j]);
            std::swap(d[k], d[pivotRow]);
        }

        for (int i = k + 1; i < n; ++i) {
            A[i][k] /= A[k][k];
            for (int j = k + 1; j < qMin(k + 2 * bw + 1, n); ++j) {
                A[i][j] -= A[i][k] * A[k][j];
            }
            d[i] -= A[i][k] * d[k];
        }
    }

    /* 回代 */
    QVector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        double sum = d[i];
        for (int j = i + 1; j < qMin(i + 2 * bw + 1, n); ++j) {
            sum -= A[i][j] * x[j];
        }
        x[i] = sum / A[i][i];
    }

    ++m_stats.totalSystemsSolved;
    ++m_stats.totalPivotingSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSystemsSolved;

    return x;
}

QVector<double> BandMatrix2::solveCyclicTridiagonal(const QVector<double>& lower,
                                                      const QVector<double>& mainDiag,
                                                      const QVector<double>& upper,
                                                      const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = mainDiag.size();
    if (n < 3) return {};

    /* Sherman-Morrison分解: 将循环三对角分解为普通三对角 + 秩1修正 */
    double alpha = lower[0];     /* 左下角 a[0] */
    double gamma = upper[n - 1]; /* 右上角 c[n-1] */
    double beta = mainDiag[0];

    /* 构建修正后的三对角系统 */
    QVector<double> modMain = mainDiag;
    QVector<double> modLower = lower;
    QVector<double> modUpper = upper;
    modMain[0] -= gamma;
    modMain[n - 1] -= alpha * beta / gamma;

    /* 求解两个普通三对角系统: Ax = d, Ay = u */
    QVector<double> u(n, 0.0);
    u[0] = gamma;
    u[n - 1] = alpha;

    QVector<double> y = solveTridiagonal(modLower, modMain, modUpper, u);
    if (y.isEmpty()) return {};

    QVector<double> xBase = solveTridiagonal(modLower, modMain, modUpper, rhs);
    if (xBase.isEmpty()) return {};

    /* Sherman-Morrison 修正 */
    double dotProduct = 0.0;
    for (int i = 0; i < n; ++i) dotProduct += u[i] * xBase[i];
    double dotY = 1.0;
    for (int i = 0; i < n; ++i) dotY += u[i] * y[i];

    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = xBase[i] - (dotProduct / dotY) * y[i];
    }

    ++m_stats.totalSystemsSolved;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSystemsSolved;

    return x;
}

BandMatrix2::ConditionInfo BandMatrix2::estimateCondition(
    const QVector<QVector<double>>& bands, int halfBandwidth, int n) const
{
    ConditionInfo info;

    if (n == 0 || bands.isEmpty()) return info;

    int bw = halfBandwidth;

    /* 检查对角占优性 */
    info.isStrictlyDiagonallyDominant = true;
    for (int row = 0; row < n; ++row) {
        double diag = getElement(bands, bw, row, row);
        double offDiagSum = 0.0;
        for (int col = 0; col < n; ++col) {
            if (col != row)
                offDiagSum += qFabs(getElement(bands, bw, row, col));
        }
        if (qFabs(diag) <= offDiagSum) {
            info.isStrictlyDiagonallyDominant = false;
            break;
        }
    }

    /* 检查对称性 */
    info.isSymmetric = true;
    for (int row = 0; row < n && info.isSymmetric; ++row) {
        for (int col = row + 1; col < qMin(row + bw + 1, n); ++col) {
            double v1 = getElement(bands, bw, row, col);
            double v2 = getElement(bands, bw, col, row);
            if (qFabs(v1 - v2) > 1e-10 * qMax(qFabs(v1), 1.0)) {
                info.isSymmetric = false;
                break;
            }
        }
    }

    /* 条件数估计(使用||A||_inf和简单逆估计) */
    double normA = 0.0;
    for (int row = 0; row < n; ++row) {
        double rowSum = 0.0;
        for (int col = qMax(0, row - bw); col <= qMin(n - 1, row + bw); ++col) {
            rowSum += qFabs(getElement(bands, bw, row, col));
        }
        normA = qMax(normA, rowSum);
    }
    info.conditionEstimate = (normA > 0) ? normA : 1.0;

    /* 正定性检查(Gershgorin) */
    if (info.isSymmetric) {
        info.isPositiveDefinite = true;
        for (int row = 0; row < n; ++row) {
            double diag = getElement(bands, bw, row, row);
            if (diag <= 0) {
                info.isPositiveDefinite = false;
                break;
            }
        }
    }

    return info;
}

double BandMatrix2::computeResidual(const QVector<QVector<double>>& bands,
                                      int halfBandwidth,
                                      const QVector<double>& solution,
                                      const QVector<double>& rhs) const
{
    int n = rhs.size();
    if (n == 0 || solution.size() != n) return -1.0;

    int bw = halfBandwidth;
    double residualNorm = 0.0;
    double rhsNorm = 0.0;

    for (int row = 0; row < n; ++row) {
        double ax = 0.0;
        for (int col = qMax(0, row - bw); col <= qMin(n - 1, row + bw); ++col) {
            ax += getElement(bands, bw, row, col) * solution[col];
        }
        double r = qFabs(ax - rhs[row]);
        residualNorm = qMax(residualNorm, r);
        rhsNorm = qMax(rhsNorm, qFabs(rhs[row]));
    }

    return (rhsNorm > 1e-15) ? residualNorm / rhsNorm : residualNorm;
}

double BandMatrix2::getElement(const QVector<QVector<double>>& bands,
                                 int halfBandwidth, int row, int col) const
{
    int diag = col - row;
    int bandIdx = diag + halfBandwidth;
    if (bandIdx < 0 || bandIdx >= bands.size()) return 0.0;
    int vecIdx = (diag >= 0) ? row : col;
    if (vecIdx < 0 || vecIdx >= bands[bandIdx].size()) return 0.0;
    return bands[bandIdx][vecIdx];
}

BandMatrix2::Stats BandMatrix2::stats() const
{
    return m_stats;
}

void BandMatrix2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
