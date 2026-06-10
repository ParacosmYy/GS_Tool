/**
 * @file RouteCipher6.cpp
 * @brief RouteCipher6 实现
 *
 * 实现路线密码：可配置网格路径模式的螺旋/对角/锯齿矩阵几何转置加密。
 */

#include "utils/code270/RouteCipher6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RouteCipher6::RouteCipher6(QObject *parent)
    : QObject(parent) {}

RouteCipher6::~RouteCipher6() = default;

/* ---- Configuration ---- */

void RouteCipher6::setGridSize(int rows, int cols)
{
    m_rows = qBound(0, rows, 1000);
    m_cols = qBound(0, cols, 1000);
}

void RouteCipher6::setPathPattern(PathPattern pattern)
{
    m_pattern = pattern;
}

/* ---- Grid dimension computation ---- */

QPair<int, int> RouteCipher6::computeGrid(int textLen) const
{
    int r = m_rows;
    int c = m_cols;
    if (r > 0 && c > 0) return {r, c};

    // Auto-compute: find closest square-ish grid
    int side = qCeil(qSqrt(static_cast<double>(textLen)));
    if (r <= 0 && c <= 0) return {side, side};
    if (r <= 0) return {qCeil(static_cast<double>(textLen) / c), c};
    return {r, qCeil(static_cast<double>(textLen) / r)};
}

/* ---- Build row-major grid ---- */

QVector<QVector<QChar>> RouteCipher6::buildGrid(const QString& text, int rows, int cols) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
    int idx = 0;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (idx < text.length())
                grid[r][c] = text[idx++];
    return grid;
}

/* ---- Spiral traversal: clockwise from top-left ---- */

QVector<int> RouteCipher6::spiralOrder(int rows, int cols) const
{
    QVector<int> order;
    int total = rows * cols;
    order.reserve(total);

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;

    while (order.size() < total) {
        // Top row, left to right
        for (int c = left; c <= right && order.size() < total; ++c)
            order.append(top * cols + c);
        ++top;

        // Right column, top to bottom
        for (int r = top; r <= bottom && order.size() < total; ++r)
            order.append(r * cols + right);
        --right;

        // Bottom row, right to left
        for (int c = right; c >= left && order.size() < total; --c)
            order.append(bottom * cols + c);
        --bottom;

        // Left column, bottom to top
        for (int r = bottom; r >= top && order.size() < total; --r)
            order.append(r * cols + left);
        ++left;
    }
    return order;
}

/* ---- Diagonal traversal ---- */

QVector<int> RouteCipher6::diagonalOrder(int rows, int cols) const
{
    QVector<int> order;
    int total = rows * cols;
    order.reserve(total);

    // Traverse all diagonals: sum r+c goes from 0 to (rows-1)+(cols-1)
    for (int diag = 0; diag <= rows + cols - 2; ++diag) {
        if (diag % 2 == 0) {
            // Even: go up-right (start from bottom of diagonal)
            for (int r = qMin(diag, rows - 1); r >= 0 && diag - r < cols; --r)
                order.append(r * cols + (diag - r));
        } else {
            // Odd: go down-left (start from top of diagonal)
            for (int c = qMin(diag, cols - 1); c >= 0 && diag - c < rows; --c)
                order.append((diag - c) * cols + c);
        }
    }
    return order;
}

/* ---- Zigzag traversal: alternating row direction ---- */

QVector<int> RouteCipher6::zigzagOrder(int rows, int cols) const
{
    QVector<int> order;
    int total = rows * cols;
    order.reserve(total);

    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            // Even row: left to right
            for (int c = 0; c < cols; ++c)
                order.append(r * cols + c);
        } else {
            // Odd row: right to left
            for (int c = cols - 1; c >= 0; --c)
                order.append(r * cols + c);
        }
    }
    return order;
}

/* ---- Get traversal order ---- */

QVector<int> RouteCipher6::traversalOrder(int rows, int cols) const
{
    switch (m_pattern) {
    case PathPattern::Spiral:   return spiralOrder(rows, cols);
    case PathPattern::Diagonal: return diagonalOrder(rows, cols);
    case PathPattern::Zigzag:   return zigzagOrder(rows, cols);
    }
    return spiralOrder(rows, cols);
}

/* ---- Encrypt ---- */

QString RouteCipher6::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    int len = plaintext.length();
    if (len == 0) return {};

    auto [rows, cols] = computeGrid(len);
    auto grid = buildGrid(plaintext, rows, cols);

    // Read grid via traversal order
    QVector<int> order = traversalOrder(rows, cols);
    QString result;
    result.reserve(order.size());
    for (int idx : order)
        result.append(grid[idx / cols][idx % cols]);

    double elapsed = timer.elapsed();
    m_stats.textSize = len;
    m_stats.gridRows = rows;
    m_stats.gridCols = cols;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherDone(len, rows, cols, elapsed);

    return result;
}

/* ---- Decrypt ---- */

QString RouteCipher6::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    int len = ciphertext.length();
    if (len == 0) return {};

    auto [rows, cols] = computeGrid(len);
    int total = rows * cols;

    // Get traversal order
    QVector<int> order = traversalOrder(rows, cols);

    // Inverse: map traversal position back to grid position
    // order[i] = gridIdx => ciphertext[i] goes to position gridIdx in the grid
    QVector<QChar> flatGrid(total, QChar('X'));
    int ciphLen = qMin(ciphertext.length(), order.size());
    for (int i = 0; i < ciphLen; ++i)
        flatGrid[order[i]] = ciphertext[i];

    // Read grid row-major for plaintext
    QString result;
    result.reserve(len);
    for (int i = 0; i < total && i < len; ++i)
        result.append(flatGrid[i]);

    double elapsed = timer.elapsed();
    m_stats.textSize = len;
    m_stats.gridRows = rows;
    m_stats.gridCols = cols;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherDone(len, rows, cols, elapsed);

    return result;
}

/* ---- Reset ---- */

void RouteCipher6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
