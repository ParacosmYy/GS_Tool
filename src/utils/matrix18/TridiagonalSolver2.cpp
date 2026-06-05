/**
 * @file TridiagonalSolver2.cpp
 * @brief 一般三对角方程组求解器实现 — Thomas算法/循环约化/多RHS
 */

#include "utils/matrix18/TridiagonalSolver2.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

TridiagonalSolver2::TridiagonalSolver2(QObject* parent)
    : QObject(parent)
{
}

TridiagonalSolver2::~TridiagonalSolver2() = default;

// ═══════════════════════════════════════════════════════════
// Thomas算法
// ═══════════════════════════════════════════════════════════

TridiagonalSolver2::SolveResult TridiagonalSolver2::solveThomas(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    result.method = QStringLiteral("Thomas");

    int n = mainDiag.size();

    /* 维度检查 */
    if (n < 1) {
        result.success = false;
        m_stats.totalFailures++;
        return result;
    }

    if (n == 1) {
        if (std::abs(mainDiag[0]) < 1e-15) {
            result.success = false;
            m_stats.totalFailures++;
            return result;
        }
        result.solution = {rhs[0] / mainDiag[0]};
        result.determinant = mainDiag[0];
        result.success = true;

        m_stats.totalSolves++;
        m_stats.totalDimensions += n;
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalSolves);
        emit solveCompleted(n, result.method);
        return result;
    }

    if (lower.size() != n - 1 || upper.size() != n - 1 || rhs.size() != n) {
        result.success = false;
        m_stats.totalFailures++;
        return result;
    }

    /* ── 前向消元 ── */
    QVector<double> cStar(n - 1);  /* 修改后的上对角线 */
    QVector<double> dStar(n);      /* 修改后的右端 */

    cStar[0] = upper[0] / mainDiag[0];
    dStar[0] = rhs[0] / mainDiag[0];

    for (int i = 1; i < n; ++i) {
        double a_i = (i < n) ? lower[i - 1] : 0.0;
        double denom = mainDiag[i] - a_i * cStar[i - 1];

        if (std::abs(denom) < 1e-15) {
            result.success = false;
            m_stats.totalFailures++;
            return result;
        }

        if (i < n - 1) {
            cStar[i] = upper[i] / denom;
        }
        dStar[i] = (rhs[i] - a_i * dStar[i - 1]) / denom;
    }

    /* ── 回代 ── */
    QVector<double> x(n);
    x[n - 1] = dStar[n - 1];

    for (int i = n - 2; i >= 0; --i) {
        x[i] = dStar[i] - cStar[i] * x[i + 1];
    }

    result.solution = x;
    result.success = true;

    /* 行列式 = prod of pivots */
    result.determinant = mainDiag[0];
    double prevPivot = mainDiag[0];
    for (int i = 1; i < n; ++i) {
        double pivot = mainDiag[i] - lower[i - 1] * (upper[i - 1] / prevPivot);
        result.determinant *= pivot;
        prevPivot = pivot;
    }

    m_stats.totalSolves++;
    m_stats.totalDimensions += n;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, result.method);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 循环约化
// ═══════════════════════════════════════════════════════════

TridiagonalSolver2::SolveResult TridiagonalSolver2::solveCyclicReduction(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    result.method = QStringLiteral("CyclicReduction");

    int n = mainDiag.size();

    if (n < 1 || lower.size() != qMax(0, n - 1) ||
        upper.size() != qMax(0, n - 1) || rhs.size() != n) {
        result.success = false;
        m_stats.totalFailures++;
        return result;
    }

    if (n == 1) {
        if (std::abs(mainDiag[0]) < 1e-15) {
            m_stats.totalFailures++;
            return result;
        }
        result.solution = {rhs[0] / mainDiag[0]};
        result.success = true;

        m_stats.totalSolves++;
        m_stats.totalDimensions += n;
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs =
            m_timeSum / static_cast<double>(m_stats.totalSolves);
        emit solveCompleted(n, result.method);
        return result;
    }

    /* 循环约化: 前向阶段 — 消去奇数行 */
    int numLevels = 0;
    int temp = n;
    while (temp > 1) { temp /= 2; ++numLevels; }

    /* 工作副本 */
    QVector<double> a = lower;
    QVector<double> b = mainDiag;
    QVector<double> c = upper;
    QVector<double> d = rhs;

    /* 为方便索引, 扩展lower/upper使索引对齐 */
    QVector<double> aFull(n, 0.0); /* aFull[i] = 下对角线, aFull[0] = 0 */
    QVector<double> cFull(n, 0.0); /* cFull[i] = 上对角线, cFull[n-1] = 0 */

    for (int i = 1; i < n; ++i) aFull[i] = lower[i - 1];
    for (int i = 0; i < n - 1; ++i) cFull[i] = upper[i];

    QVector<double> aWork = aFull;
    QVector<double> bWork = b;
    QVector<double> cWork = cFull;
    QVector<double> dWork = d;

    /* 前向消元: 每级消除一半的奇数索引方程 */
    int stride = 1;
    QVector<QVector<double>> savedA, savedB, savedC, savedD;
    savedA.reserve(numLevels);
    savedB.reserve(numLevels);
    savedC.reserve(numLevels);
    savedD.reserve(numLevels);

    while (stride < n) {
        savedA.append(aWork);
        savedB.append(bWork);
        savedC.append(cWork);
        savedD.append(dWork);

        QVector<double> newA(n, 0.0);
        QVector<double> newB(n, 0.0);
        QVector<double> newC(n, 0.0);
        QVector<double> newD(n, 0.0);

        for (int i = 0; i < n; ++i) {
            if (i % (2 * stride) == 0) {
                /* 偶数行: 消去两侧奇数行后合并 */
                int left = i - stride;
                int right = i + stride;
                double alpha = 0.0, gamma = 0.0;

                if (left >= 0) {
                    double denom = bWork[left];
                    if (std::abs(denom) < 1e-15) denom = 1e-15;
                    alpha = aWork[i] / denom;
                }
                if (right < n) {
                    double denom = bWork[right];
                    if (std::abs(denom) < 1e-15) denom = 1e-15;
                    gamma = cWork[i] / denom;
                }

                newA[i] = -alpha * aWork[left];
                newB[i] = bWork[i] - alpha * cWork[left] - gamma * aWork[right];
                newC[i] = -gamma * cWork[right];
                newD[i] = dWork[i] - alpha * dWork[left] - gamma * dWork[right];
            }
        }

        aWork = newA;
        bWork = newB;
        cWork = newC;
        dWork = newD;

        stride *= 2;
    }

    /* 回代阶段 */
    QVector<double> x(n, 0.0);

    /* 最终约化后, 只有x[0]有有效方程 */
    if (std::abs(bWork[0]) > 1e-15) {
        x[0] = dWork[0] / bWork[0];
    }

    /* 反向展开 */
    stride = 1;
    int level = 0;
    while (stride < n) {
        /* 使用前一级的系数来恢复被消去的奇数行 */
        const auto& prevA = savedA[level];
        const auto& prevB = savedB[level];
        const auto& prevC = savedC[level];
        const auto& prevD = savedD[level];

        QVector<double> newX = x;

        for (int i = stride; i < n; i += 2 * stride) {
            int left = i - stride;
            int right = i + stride;
            if (right >= n) right = i; /* 边界处理 */

            double rhsVal = prevD[i];
            if (left >= 0) rhsVal -= prevA[i] * x[left];
            if (right < n && right != i) rhsVal -= prevC[i] * x[right];

            if (std::abs(prevB[i]) > 1e-15) {
                newX[i] = rhsVal / prevB[i];
            }
        }

        x = newX;
        stride *= 2;
        ++level;
    }

    result.solution = x;
    result.success = true;

    m_stats.totalSolves++;
    m_stats.totalDimensions += n;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, result.method);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 多右端求解
// ═══════════════════════════════════════════════════════════

TridiagonalSolver2::MultiRhsResult TridiagonalSolver2::solveMultiRhs(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper,
    const QVector<QVector<double>>& rhsList)
{
    QElapsedTimer timer;
    timer.start();

    MultiRhsResult result;
    result.method = QStringLiteral("Thomas-MultiRHS");

    int n = mainDiag.size();
    if (n < 1 || rhsList.isEmpty()) {
        result.success = false;
        m_stats.totalFailures++;
        return result;
    }

    /* 一次性LU分解(前向消元) */
    QVector<double> cStar(n - 1);
    QVector<double> pivot(n);

    if (std::abs(mainDiag[0]) < 1e-15) {
        result.success = false;
        m_stats.totalFailures++;
        return result;
    }

    pivot[0] = mainDiag[0];
    cStar[0] = upper[0] / pivot[0];

    for (int i = 1; i < n; ++i) {
        double a_i = lower[i - 1];
        pivot[i] = mainDiag[i] - a_i * cStar[i - 1];

        if (std::abs(pivot[i]) < 1e-15) {
            result.success = false;
            m_stats.totalFailures++;
            return result;
        }

        if (i < n - 1) {
            cStar[i] = upper[i] / pivot[i];
        }
    }

    /* 对每个右端向量分别前代+回代 */
    for (const auto& rhs : rhsList) {
        if (rhs.size() != n) {
            result.solutions.append(QVector<double>());
            continue;
        }

        /* 前代 */
        QVector<double> dStar(n);
        dStar[0] = rhs[0] / pivot[0];

        for (int i = 1; i < n; ++i) {
            dStar[i] = (rhs[i] - lower[i - 1] * dStar[i - 1]) / pivot[i];
        }

        /* 回代 */
        QVector<double> x(n);
        x[n - 1] = dStar[n - 1];

        for (int i = n - 2; i >= 0; --i) {
            x[i] = dStar[i] - cStar[i] * x[i + 1];
        }

        result.solutions.append(x);
    }

    result.success = true;

    m_stats.totalSolves++;
    m_stats.totalMultiRhsSolves++;
    m_stats.totalDimensions += n * rhsList.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n * rhsList.size(), result.method);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 行列式
// ═══════════════════════════════════════════════════════════

double TridiagonalSolver2::determinant(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper) const
{
    int n = mainDiag.size();
    if (n == 0) return 0.0;
    if (n == 1) return mainDiag[0];

    /* 递推: det_n = a[n-1] * det_{n-1} - b[n-1]*c[n-2] * det_{n-2} */
    double detPrev2 = mainDiag[0];
    double detPrev1 = mainDiag[0] * mainDiag[1] -
                      (lower.isEmpty() ? 0.0 : lower[0]) *
                      (upper.isEmpty() ? 0.0 : upper[0]);

    if (n == 2) return detPrev1;

    double det = detPrev1;
    for (int i = 2; i < n; ++i) {
        det = mainDiag[i] * detPrev1 -
              lower[i - 1] * upper[i - 1] * detPrev2;
        detPrev2 = detPrev1;
        detPrev1 = det;
    }

    return det;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

TridiagonalSolver2::Stats TridiagonalSolver2::stats() const
{
    return m_stats;
}

void TridiagonalSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
