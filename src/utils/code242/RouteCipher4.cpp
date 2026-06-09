/**
 * @file RouteCipher4.cpp
 * @brief RouteCipher4 实现
 *
 * 实现路由密码：螺旋/对角/锯齿可配置路径矩形网格转置。
 */

#include "utils/code242/RouteCipher4.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

RouteCipher4::RouteCipher4(QObject *parent) : QObject(parent) {}
RouteCipher4::~RouteCipher4() = default;

/* ---- Configuration ---- */

void RouteCipher4::setGridSize(int rows, int cols)
{
    m_rows = qMax(1, rows);
    m_cols = qMax(1, cols);
}

void RouteCipher4::setPathPattern(PathPattern p) { m_pattern = p; }

/* ---- Spiral order: clockwise inward ---- */

QVector<int> RouteCipher4::spiralOrder(int rows, int cols) const
{
    QVector<int> order;
    int total = rows * cols;
    order.reserve(total);

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    while (order.size() < total) {
        // Top row: left to right
        for (int c = left; c <= right && order.size() < total; ++c)
            order.append(top * cols + c);
        ++top;
        // Right column: top to bottom
        for (int r = top; r <= bottom && order.size() < total; ++r)
            order.append(r * cols + right);
        --right;
        // Bottom row: right to left
        for (int c = right; c >= left && order.size() < total; --c)
            order.append(bottom * cols + c);
        --bottom;
        // Left column: bottom to top
        for (int r = bottom; r >= top && order.size() < total; --r)
            order.append(r * cols + left);
        ++left;
    }
    return order;
}

/* ---- Diagonal order: top-left to bottom-right ---- */

QVector<int> RouteCipher4::diagonalOrder(int rows, int cols) const
{
    QVector<int> order;
    int total = rows * cols;
    order.reserve(total);

    // Sum of (row + col) goes from 0 to (rows-1 + cols-1)
    for (int s = 0; s <= rows - 1 + cols - 1; ++s) {
        // Alternate direction for each diagonal
        if (s % 2 == 0) {
            // Go up-right: start from bottom
            for (int r = qMin(s, rows - 1); r >= 0; --r) {
                int c = s - r;
                if (c >= 0 && c < cols) order.append(r * cols + c);
            }
        } else {
            // Go down-left: start from top
            for (int r = 0; r <= qMin(s, rows - 1); ++r) {
                int c = s - r;
                if (c >= 0 && c < cols) order.append(r * cols + c);
            }
        }
    }
    return order;
}

/* ---- Zigzag order: alternating row direction ---- */

QVector<int> RouteCipher4::zigzagOrder(int rows, int cols) const
{
    QVector<int> order;
    int total = rows * cols;
    order.reserve(total);

    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            // Left to right
            for (int c = 0; c < cols; ++c)
                order.append(r * cols + c);
        } else {
            // Right to left
            for (int c = cols - 1; c >= 0; --c)
                order.append(r * cols + c);
        }
    }
    return order;
}

/* ---- Get pattern indices ---- */

QVector<int> RouteCipher4::patternOrder(int rows, int cols) const
{
    switch (m_pattern) {
    case Spiral:    return spiralOrder(rows, cols);
    case Diagonal:  return diagonalOrder(rows, cols);
    case Zigzag:    return zigzagOrder(rows, cols);
    }
    return spiralOrder(rows, cols);
}

/* ---- Invert a permutation ---- */

QVector<int> RouteCipher4::invertPermutation(const QVector<int>& perm) const
{
    QVector<int> inv(perm.size());
    for (int i = 0; i < perm.size(); ++i)
        inv[perm[i]] = i;
    return inv;
}

/* ---- Read by pattern (encrypt) ---- */

QVector<QChar> RouteCipher4::readByPattern(const QVector<QChar>& rowMajor,
                                            int rows, int cols) const
{
    QVector<int> order = patternOrder(rows, cols);
    QVector<QChar> result;
    result.reserve(order.size());
    for (int idx : order) {
        if (idx >= 0 && idx < rowMajor.size())
            result.append(rowMajor[idx]);
        else
            result.append(QChar('X'));
    }
    return result;
}

/* ---- Reverse pattern (decrypt) ---- */

QVector<QChar> RouteCipher4::reversePattern(const QVector<QChar>& ordered,
                                             int rows, int cols) const
{
    QVector<int> order = patternOrder(rows, cols);
    QVector<QChar> rowMajor(rows * cols, QChar('X'));
    for (int i = 0; i < order.size() && i < ordered.size(); ++i) {
        if (order[i] >= 0 && order[i] < rowMajor.size())
            rowMajor[order[i]] = ordered[i];
    }
    return rowMajor;
}

/* ---- Encrypt ---- */

QString RouteCipher4::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    int gridSize = m_rows * m_cols;
    // Fill grid row-major with padding
    QVector<QChar> grid(gridSize, QChar('X'));
    for (int i = 0; i < qMin(plaintext.length(), gridSize); ++i)
        grid[i] = plaintext[i];

    QVector<QChar> encrypted = readByPattern(grid, m_rows, m_cols);
    QString result;
    for (const QChar& ch : encrypted) result += ch;

    m_stats.inputLength = plaintext.length();
    m_stats.gridRows = m_rows;
    m_stats.gridCols = m_cols;
    m_stats.pattern = m_pattern;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(true, plaintext.length(), m_rows, m_cols, timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString RouteCipher4::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    int gridSize = m_rows * m_cols;
    QVector<QChar> ordered(gridSize, QChar('X'));
    for (int i = 0; i < qMin(ciphertext.length(), gridSize); ++i)
        ordered[i] = ciphertext[i];

    QVector<QChar> decrypted = reversePattern(ordered, m_rows, m_cols);
    QString result;
    for (const QChar& ch : decrypted) result += ch;

    m_stats.inputLength = ciphertext.length();
    m_stats.gridRows = m_rows;
    m_stats.gridCols = m_cols;
    m_stats.pattern = m_pattern;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(false, ciphertext.length(), m_rows, m_cols, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void RouteCipher4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
