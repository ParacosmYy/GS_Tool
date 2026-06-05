/**
 * @file StrassenMultiply.cpp
 * @brief Strassen矩阵乘法实现 — 动态交叉点
 */

#include "utils/matrix14/StrassenMultiply.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param crossover 交叉点阈值 @param parent 父对象 */
StrassenMultiply::StrassenMultiply(int crossover, QObject* parent)
    : QObject(parent)
    , m_crossover(crossover)
{
}

/** @brief 朴素O(n^3)矩阵乘法 */
QVector<QVector<double>> StrassenMultiply::naiveMultiply(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    if (A.isEmpty() || B.isEmpty() || A[0].size() != B.size()) return {};

    int m = A.size();
    int k = A[0].size();
    int n = B[0].size();

    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int p = 0; p < k; ++p) {
                sum += A[i][p] * B[p][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}

/** @brief 矩阵加法 */
QVector<QVector<double>> StrassenMultiply::add(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int rows = A.size(), cols = A[0].size();
    QVector<QVector<double>> C(rows, QVector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    return C;
}

/** @brief 矩阵减法 */
QVector<QVector<double>> StrassenMultiply::subtract(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int rows = A.size(), cols = A[0].size();
    QVector<QVector<double>> C(rows, QVector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
}

/** @brief 补零到2的幂次方阵 */
QPair<QVector<QVector<double>>, QVector<QVector<double>>>
StrassenMultiply::padToPowerOfTwo(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    int rowsA = A.size(), colsA = A[0].size();
    int rowsB = B.size(), colsB = B[0].size();

    int maxDim = std::max({rowsA, colsA, rowsB, colsB});
    int n = 1;
    while (n < maxDim) n *= 2;

    m_trimRows = rowsA;
    m_trimCols = colsB;

    auto pad = [&](const QVector<QVector<double>>& M, int r, int c) {
        QVector<QVector<double>> P(n, QVector<double>(n, 0.0));
        for (int i = 0; i < r; ++i) {
            for (int j = 0; j < c; ++j) {
                P[i][j] = M[i][j];
            }
        }
        return P;
    };

    return qMakePair(pad(A, rowsA, colsA), pad(B, rowsB, colsB));
}

/** @brief 裁剪回原始尺寸 */
QVector<QVector<double>> StrassenMultiply::trim(
    const QVector<QVector<double>>& M, int rows, int cols)
{
    QVector<QVector<double>> T(rows, QVector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            T[i][j] = M[i][j];
        }
    }
    return T;
}

/** @brief 从方阵中提取子矩阵 */
static QVector<QVector<double>> extractSub(
    const QVector<QVector<double>>& M,
    int rowStart, int colStart, int half)
{
    QVector<QVector<double>> sub(half, QVector<double>(half));
    for (int i = 0; i < half; ++i) {
        for (int j = 0; j < half; ++j) {
            sub[i][j] = M[rowStart + i][colStart + j];
        }
    }
    return sub;
}

/** @brief 将子矩阵写回 */
static void insertSub(QVector<QVector<double>>& M,
                       const QVector<QVector<double>>& sub,
                       int rowStart, int colStart)
{
    int half = sub.size();
    for (int i = 0; i < half; ++i) {
        for (int j = 0; j < half; ++j) {
            M[rowStart + i][colStart + j] = sub[i][j];
        }
    }
}

/** @brief Strassen递归核心(仅方阵，2的幂次) */
QVector<QVector<double>> StrassenMultiply::strassenRecursive(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    int depth)
{
    int n = A.size();

    /* 基础情况: 交叉点以下用朴素乘法 */
    if (n <= m_crossover) {
        ++m_stats.totalNaiveFallbacks;
        return naiveMultiply(A, B);
    }

    ++m_stats.totalStrassenCalls;
    int half = n / 2;

    /* 分解为4个子矩阵 */
    auto A11 = extractSub(A, 0, 0, half);
    auto A12 = extractSub(A, 0, half, half);
    auto A21 = extractSub(A, half, 0, half);
    auto A22 = extractSub(A, half, half, half);

    auto B11 = extractSub(B, 0, 0, half);
    auto B12 = extractSub(B, 0, half, half);
    auto B21 = extractSub(B, half, 0, half);
    auto B22 = extractSub(B, half, half, half);

    /* 计算7个中间矩阵 */
    auto M1 = strassenRecursive(add(A11, A22), add(B11, B22), depth + 1);
    auto M2 = strassenRecursive(add(A21, A22), B11, depth + 1);
    auto M3 = strassenRecursive(A11, subtract(B12, B22), depth + 1);
    auto M4 = strassenRecursive(A22, subtract(B21, B11), depth + 1);
    auto M5 = strassenRecursive(add(A11, A12), B22, depth + 1);
    auto M6 = strassenRecursive(subtract(A21, A11), add(B11, B12), depth + 1);
    auto M7 = strassenRecursive(subtract(A12, A22), add(B21, B22), depth + 1);

    /* 组合结果 */
    int full = n;
    QVector<QVector<double>> C(full, QVector<double>(full, 0.0));
    insertSub(C, add(subtract(add(M1, M4), M5), M7), 0, 0);
    insertSub(C, add(M3, M5), 0, half);
    insertSub(C, add(M2, M4), half, 0);
    insertSub(C, add(subtract(add(M1, M3), M2), M6), half, half);

    return C;
}

/** @brief 矩阵乘法(通用) @param A 左矩阵 @param B 右矩阵 @return 结果矩阵 */
QVector<QVector<double>> StrassenMultiply::multiply(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    if (A.isEmpty() || B.isEmpty() || A[0].size() != B.size()) return {};

    QElapsedTimer timer;
    timer.start();

    auto padded = padToPowerOfTwo(A, B);
    auto result = strassenRecursive(padded.first, padded.second, 0);
    auto trimmed = trim(result, m_trimRows, m_trimCols);

    ++m_stats.totalMultiplications;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalMultiplications);

    emit multiplicationCompleted(trimmed.size(), trimmed.isEmpty() ? 0 : trimmed[0].size());
    return trimmed;
}

/** @brief 方阵乘法(优化路径) */
QVector<QVector<double>> StrassenMultiply::multiplySquare(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    if (A.isEmpty() || B.isEmpty()) return {};
    int n = A.size();
    if (A[0].size() != n || B.size() != n || B[0].size() != n) return {};

    QElapsedTimer timer;
    timer.start();

    /* 确保n是2的幂次 */
    int padded = 1;
    while (padded < n) padded *= 2;

    QVector<QVector<double>> Ap(padded, QVector<double>(padded, 0.0));
    QVector<QVector<double>> Bp(padded, QVector<double>(padded, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            Ap[i][j] = A[i][j];
            Bp[i][j] = B[i][j];
        }
    }

    m_trimRows = n;
    m_trimCols = n;
    auto result = strassenRecursive(Ap, Bp, 0);
    auto trimmed = trim(result, n, n);

    ++m_stats.totalMultiplications;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalMultiplications);

    emit multiplicationCompleted(n, n);
    return trimmed;
}

/** @brief 自动寻找最优交叉点 @param sizes 测试矩阵尺寸 @return 最优交叉点值 */
int StrassenMultiply::autoTuneCrossover(const QList<int>& sizes)
{
    int bestCrossover = m_crossover;
    double bestTime = std::numeric_limits<double>::max();

    /* 生成随机测试矩阵 */
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (int trial : {16, 32, 48, 64, 96, 128}) {
        m_crossover = trial;

        double totalTime = 0.0;
        for (int sz : sizes) {
            int n = 1;
            while (n < sz) n *= 2;

            QVector<QVector<double>> A(n, QVector<double>(n));
            QVector<QVector<double>> B(n, QVector<double>(n));
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    A[i][j] = dist(rng);
                    B[i][j] = dist(rng);
                }
            }

            QElapsedTimer timer;
            timer.start();
            m_trimRows = n;
            m_trimCols = n;
            strassenRecursive(A, B, 0);
            totalTime += timer.elapsed();
        }

        if (totalTime < bestTime) {
            bestTime = totalTime;
            bestCrossover = trial;
        }
    }

    m_crossover = bestCrossover;
    return bestCrossover;
}

/** @brief 重置统计 */
void StrassenMultiply::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
