/**
 * @file RouteCipher7.cpp
 * @brief RouteCipher7 实现
 *
 * 实现路径密码：可配置矩阵路径的灵活几何换位加密。
 */

#include "utils/code284/RouteCipher7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RouteCipher7::RouteCipher7(QObject *parent)
    : QObject(parent) {}

RouteCipher7::~RouteCipher7() = default;

/* ---- Configuration ---- */

void RouteCipher7::setConfig(const CipherConfig& cfg)
{
    m_config = cfg;
}

/* ---- Suggest matrix dimensions ---- */

QPair<int, int> RouteCipher7::suggestDimensions(int length) const
{
    int bestR = 1, bestC = length;
    int bestDiff = length;

    for (int r = 1; r <= length; ++r) {
        int c = (length + r - 1) / r;
        int total = r * c;
        int diff = total - length;
        if (diff < bestDiff) {
            bestDiff = diff;
            bestR = r;
            bestC = c;
        }
        if (diff == 0) break;
    }
    return qMakePair(bestR, bestC);
}

/* ---- Build 2D matrix from text ---- */

QVector<QVector<QChar>> RouteCipher7::buildMatrix(const QString& text,
                                                    int rows, int cols) const
{
    QVector<QVector<QChar>> mat(rows, QVector<QChar>(cols, m_config.padWith));
    int idx = 0;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            if (idx < text.size())
                mat[r][c] = text[idx++];
        }
    return mat;
}

/* ---- Flatten matrix row-major ---- */

QString RouteCipher7::flattenMatrix(const QVector<QVector<QChar>>& mat) const
{
    QString result;
    for (const auto& row : mat)
        for (const QChar& ch : row)
            result.append(ch);
    return result;
}

/* ---- Inward spiral traversal ---- */

QVector<int> RouteCipher7::inwardSpiral(int rows, int cols) const
{
    QVector<int> order;
    order.reserve(rows * cols);

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    while (top <= bottom && left <= right) {
        // Right across top
        for (int c = left; c <= right; ++c)
            order.append(top * cols + c);
        ++top;
        // Down right side
        for (int r = top; r <= bottom; ++r)
            order.append(r * cols + right);
        --right;
        // Left across bottom
        if (top <= bottom) {
            for (int c = right; c >= left; --c)
                order.append(bottom * cols + c);
            --bottom;
        }
        // Up left side
        if (left <= right) {
            for (int r = bottom; r >= top; --r)
                order.append(r * cols + left);
            ++left;
        }
    }
    return order;
}

/* ---- Outward spiral traversal ---- */

QVector<int> RouteCipher7::outwardSpiral(int rows, int cols) const
{
    QVector<int> inward = inwardSpiral(rows, cols);
    QVector<int> result;
    result.reserve(inward.size());

    // Reverse the inward order
    for (int i = inward.size() - 1; i >= 0; --i)
        result.append(inward[i]);
    return result;
}

/* ---- Split columns (alternating up/down) ---- */

QVector<int> RouteCipher7::splitColumns(int rows, int cols) const
{
    QVector<int> order;
    order.reserve(rows * cols);

    for (int c = 0; c < cols; ++c) {
        if (c % 2 == 0) {
            // Top to bottom
            for (int r = 0; r < rows; ++r)
                order.append(r * cols + c);
        } else {
            // Bottom to top
            for (int r = rows - 1; r >= 0; --r)
                order.append(r * cols + c);
        }
    }
    return order;
}

/* ---- Zigzag rows ---- */

QVector<int> RouteCipher7::zigzagRows(int rows, int cols) const
{
    QVector<int> order;
    order.reserve(rows * cols);

    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c)
                order.append(r * cols + c);
        } else {
            for (int c = cols - 1; c >= 0; --c)
                order.append(r * cols + c);
        }
    }
    return order;
}

/* ---- Generate path indices ---- */

QVector<int> RouteCipher7::generatePathIndices(int rows, int cols, PathType path) const
{
    switch (path) {
    case PathType::InwardSpiral:  return inwardSpiral(rows, cols);
    case PathType::OutwardSpiral: return outwardSpiral(rows, cols);
    case PathType::SplitColumns:  return splitColumns(rows, cols);
    case PathType::ZigzagRows:    return zigzagRows(rows, cols);
    }
    return inwardSpiral(rows, cols);
}

/* ---- Encrypt ---- */

QString RouteCipher7::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    int len = plaintext.size();
    if (len == 0) return QString();

    int rows = m_config.rows;
    int cols = m_config.cols;
    if (rows <= 0 || cols <= 0) {
        auto dims = suggestDimensions(len);
        rows = dims.first;
        cols = dims.second;
    }

    auto mat = buildMatrix(plaintext, rows, cols);
    auto order = generatePathIndices(rows, cols, m_config.path);

    QString cipher;
    cipher.reserve(order.size());
    for (int idx : order)
        cipher.append(mat[idx / cols][idx % cols]);

    double elapsed = timer.elapsed();
    m_stats.lastRows = rows;
    m_stats.lastCols = cols;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptDone(len, rows, cols, elapsed);

    return cipher;
}

/* ---- Decrypt ---- */

QString RouteCipher7::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    int len = ciphertext.size();
    if (len == 0) return QString();

    int rows = m_config.rows;
    int cols = m_config.cols;
    if (rows <= 0 || cols <= 0) {
        auto dims = suggestDimensions(len);
        rows = dims.first;
        cols = dims.second;
    }

    int total = rows * cols;
    auto order = generatePathIndices(rows, cols, m_config.path);

    // Build flat matrix from path order
    QVector<QChar> flat(total, m_config.padWith);
    for (int i = 0; i < qMin(order.size(), len); ++i)
        flat[order[i]] = ciphertext[i];

    // Reconstruct text row-major
    QString result;
    result.reserve(total);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.append(flat[r * cols + c]);

    // Remove padding
    if (!m_config.padChar) {
        while (result.endsWith(m_config.padWith) && result.size() > 0)
            result.chop(1);
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptDone(len, rows, cols, elapsed);

    return result;
}

/* ---- Reset ---- */

void RouteCipher7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
