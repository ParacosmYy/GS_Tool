/**
 * @file RouteCipher.cpp
 * @brief RouteCipher 实现
 *
 * 实现路径密码：螺旋/对角/锯齿网格遍历、加密解密、路径恢复密码分析。
 */

#include "utils/code198/RouteCipher.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RouteCipher::RouteCipher(QObject *parent) : QObject(parent) {}
RouteCipher::~RouteCipher() = default;

/* ---- Configuration ---- */

void RouteCipher::setGridSize(int rows, int cols) { m_rows = qMax(2, rows); m_cols = qMax(2, cols); }
void RouteCipher::setPathMode(PathMode mode) { m_mode = mode; }

/* ---- Spiral path ---- */

QVector<QPair<int,int>> RouteCipher::spiralPath(int rows, int cols) const
{
    QVector<QPair<int,int>> path;
    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;

    while (top <= bottom && left <= right) {
        for (int c = left; c <= right; ++c) path.append({top, c});
        top++;
        for (int r = top; r <= bottom; ++r) path.append({r, right});
        right--;
        if (top <= bottom) {
            for (int c = right; c >= left; --c) path.append({bottom, c});
            bottom--;
        }
        if (left <= right) {
            for (int r = bottom; r >= top; --r) path.append({r, left});
            left++;
        }
    }
    return path;
}

/* ---- Diagonal path ---- */

QVector<QPair<int,int>> RouteCipher::diagonalPath(int rows, int cols) const
{
    QVector<QPair<int,int>> path;
    for (int s = 0; s < rows + cols - 1; ++s) {
        if (s % 2 == 0) {
            for (int r = qMin(s, rows - 1); r >= 0; --r) {
                int c = s - r;
                if (c >= 0 && c < cols) path.append({r, c});
            }
        } else {
            for (int c = qMin(s, cols - 1); c >= 0; --c) {
                int r = s - c;
                if (r >= 0 && r < rows) path.append({r, c});
            }
        }
    }
    return path;
}

/* ---- Zigzag path ---- */

QVector<QPair<int,int>> RouteCipher::zigzagPath(int rows, int cols) const
{
    QVector<QPair<int,int>> path;
    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c) path.append({r, c});
        } else {
            for (int c = cols - 1; c >= 0; --c) path.append({r, c});
        }
    }
    return path;
}

/* ---- Build grid ---- */

QVector<QVector<QChar>> RouteCipher::buildGrid(const QString& text, int rows, int cols) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
    int idx = 0;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (idx < text.length()) grid[r][c] = text[idx++];
    return grid;
}

/* ---- Read path ---- */

QString RouteCipher::readPath(const QVector<QVector<QChar>>& grid,
                               const QVector<QPair<int,int>>& path) const
{
    QString result;
    int rows = grid.size();
    if (rows == 0) return result;
    for (const auto& [r, c] : path)
        if (r >= 0 && r < rows && c >= 0 && c < grid[0].size())
            result.append(grid[r][c]);
    return result;
}

/* ---- Write path ---- */

void RouteCipher::writePath(QVector<QVector<QChar>>& grid,
                             const QVector<QPair<int,int>>& path, const QString& text) const
{
    int rows = grid.size();
    if (rows == 0) return;
    int cols = grid[0].size();
    for (int i = 0; i < path.size() && i < text.length(); ++i) {
        int r = path[i].first, c = path[i].second;
        if (r >= 0 && r < rows && c >= 0 && c < cols)
            grid[r][c] = text[i];
    }
}

/* ---- Auto grid size ---- */

QPair<int,int> RouteCipher::autoGridSize(int textLen) const
{
    int best = qCeil(qSqrt(static_cast<double>(textLen)));
    for (int r = 2; r <= best + 1; ++r) {
        int c = qCeil(static_cast<double>(textLen) / r);
        if (r * c >= textLen) return {r, c};
    }
    return {best, best};
}

/* ---- Current path ---- */

QVector<QPair<int,int>> RouteCipher::currentPath(int rows, int cols) const
{
    switch (m_mode) {
    case PathMode::Spiral:   return spiralPath(rows, cols);
    case PathMode::Diagonal: return diagonalPath(rows, cols);
    case PathMode::Zigzag:   return zigzagPath(rows, cols);
    }
    return spiralPath(rows, cols);
}

/* ---- Encrypt ---- */

QString RouteCipher::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    int len = plaintext.length();
    auto [rows, cols] = (len > 0) ? autoGridSize(len) : qMakePair(m_rows, m_cols);
    if (rows * cols < len) cols = qCeil(static_cast<double>(len) / rows);

    auto grid = buildGrid(plaintext, rows, cols);
    auto path = currentPath(rows, cols);
    QString result = readPath(grid, path);

    m_stats.totalOps++;
    m_stats.lastRows = rows;
    m_stats.lastCols = cols;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit const_cast<RouteCipher*>(this)->operationCompleted("encrypt", rows, cols, timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString RouteCipher::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    int len = ciphertext.length();
    auto [rows, cols] = (len > 0) ? autoGridSize(len) : qMakePair(m_rows, m_cols);
    if (rows * cols < len) cols = qCeil(static_cast<double>(len) / rows);

    auto path = currentPath(rows, cols);
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
    writePath(grid, path, ciphertext);

    // Read row-major
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.append(grid[r][c]);

    m_stats.totalOps++;
    m_stats.lastRows = rows;
    m_stats.lastCols = cols;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit const_cast<RouteCipher*>(this)->operationCompleted("decrypt", rows, cols, timer.elapsed());
    return result;
}

/* ---- English score ---- */

double RouteCipher::englishScore(const QString& text)
{
    if (text.isEmpty()) return 0.0;
    static const QString common = "ETAOINSHRDLCUMWFGYPBVKJXQZetaoinshrdlcumwfgypbvkjqzx ";
    int score = 0;
    for (const QChar& ch : text)
        if (common.contains(ch)) score++;
    return static_cast<double>(score) / text.length();
}

/* ---- Cryptanalyze ---- */

QVector<QPair<int,int>> RouteCipher::cryptanalyze(const QString& ciphertext, int maxDim) const
{
    QVector<QPair<double, QPair<int,int>>> results;
    PathMode modes[] = {PathMode::Spiral, PathMode::Diagonal, PathMode::Zigzag};

    for (int r = 2; r <= maxDim; ++r) {
        for (int c = 2; c <= maxDim; ++c) {
            if (r * c < ciphertext.length()) continue;
            for (PathMode m : modes) {
                m_mode = m;
                QString decrypted = decrypt(ciphertext);
                double score = englishScore(decrypted);
                results.append({score, {r, c}});
            }
        }
    }

    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    QVector<QPair<int,int>> top;
    for (int i = 0; i < qMin(10, results.size()); ++i)
        top.append(results[i].second);
    return top;
}

/* ---- Reset ---- */

void RouteCipher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
