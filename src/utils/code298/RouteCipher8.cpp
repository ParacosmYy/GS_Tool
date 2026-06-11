/**
 * @file RouteCipher8.cpp
 * @brief RouteCipher8 实现
 *
 * 实现路线密码：螺旋路径枚举与对角遍历实现多模式几何置换加密。
 */

#include "utils/code298/RouteCipher8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RouteCipher8::RouteCipher8(QObject *parent)
    : QObject(parent) {}

RouteCipher8::~RouteCipher8() = default;

/* ---- Configuration ---- */

void RouteCipher8::setPathMode(PathMode mode) { m_mode = mode; }
void RouteCipher8::setGridSize(int rows, int cols) {
    m_rows = qBound(1, rows, 256);
    m_cols = qBound(1, cols, 256);
}
void RouteCipher8::setPaddingChar(QChar ch) { m_padding = ch; }

/* ---- Compute grid dimensions ---- */

void RouteCipher8::computeGridSize(int textLen)
{
    if (m_rows > 0 && m_cols > 0) return; // user-specified
    int n = qMax(1, textLen);
    int side = qMax(1, static_cast<int>(qCeil(qSqrt(static_cast<double>(n)))));
    m_rows = side;
    m_cols = side;
}

/* ---- Fill grid row-major ---- */

QVector<QVector<QChar>> RouteCipher8::fillGrid(const QString& text, int rows, int cols) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, m_padding));
    int idx = 0;
    for (int r = 0; r < rows && idx < text.size(); ++r)
        for (int c = 0; c < cols && idx < text.size(); ++c)
            grid[r][c] = text[idx++];
    return grid;
}

/* ---- Spiral CW indices ---- */

QVector<int> RouteCipher8::spiralCWIndices(int rows, int cols) const
{
    int total = rows * cols;
    QVector<int> indices;
    indices.reserve(total);

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    while (top <= bottom && left <= right) {
        for (int c = left; c <= right; ++c) indices.append(top * cols + c);
        ++top;
        for (int r = top; r <= bottom; ++r) indices.append(r * cols + right);
        --right;
        if (top <= bottom) {
            for (int c = right; c >= left; --c) indices.append(bottom * cols + c);
            --bottom;
        }
        if (left <= right) {
            for (int r = bottom; r >= top; --r) indices.append(r * cols + left);
            ++left;
        }
    }
    return indices;
}

/* ---- Spiral CCW indices ---- */

QVector<int> RouteCipher8::spiralCCWIndices(int rows, int cols) const
{
    int total = rows * cols;
    QVector<int> indices;
    indices.reserve(total);

    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    while (top <= bottom && left <= right) {
        for (int r = top; r <= bottom; ++r) indices.append(r * cols + left);
        ++left;
        for (int c = left; c <= right; ++c) indices.append(bottom * cols + c);
        --bottom;
        if (left <= right) {
            for (int r = bottom; r >= top; --r) indices.append(r * cols + right);
            --right;
        }
        if (top <= bottom) {
            for (int c = right; c >= left; --c) indices.append(top * cols + c);
            ++top;
        }
    }
    return indices;
}

/* ---- Diagonal down indices ---- */

QVector<int> RouteCipher8::diagonalDownIndices(int rows, int cols) const
{
    QVector<int> indices;
    indices.reserve(rows * cols);
    for (int sum = 0; sum <= rows + cols - 2; ++sum)
        for (int r = 0; r < rows; ++r) {
            int c = sum - r;
            if (c >= 0 && c < cols) indices.append(r * cols + c);
        }
    return indices;
}

/* ---- Diagonal up indices ---- */

QVector<int> RouteCipher8::diagonalUpIndices(int rows, int cols) const
{
    QVector<int> indices;
    indices.reserve(rows * cols);
    for (int sum = rows + cols - 2; sum >= 0; --sum)
        for (int r = rows - 1; r >= 0; --r) {
            int c = sum - r;
            if (c >= 0 && c < cols) indices.append(r * cols + c);
        }
    return indices;
}

/* ---- Zigzag vertical indices ---- */

QVector<int> RouteCipher8::zigzagVerticalIndices(int rows, int cols) const
{
    QVector<int> indices;
    indices.reserve(rows * cols);
    for (int c = 0; c < cols; ++c) {
        if (c % 2 == 0) {
            for (int r = 0; r < rows; ++r) indices.append(r * cols + c);
        } else {
            for (int r = rows - 1; r >= 0; --r) indices.append(r * cols + c);
        }
    }
    return indices;
}

/* ---- Zigzag horizontal indices ---- */

QVector<int> RouteCipher8::zigzagHorizontalIndices(int rows, int cols) const
{
    QVector<int> indices;
    indices.reserve(rows * cols);
    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c) indices.append(r * cols + c);
        } else {
            for (int c = cols - 1; c >= 0; --c) indices.append(r * cols + c);
        }
    }
    return indices;
}

/* ---- Get path indices by mode ---- */

QVector<int> RouteCipher8::getPathIndices(int rows, int cols, PathMode mode) const
{
    switch (mode) {
    case PathMode::SpiralCW:        return spiralCWIndices(rows, cols);
    case PathMode::SpiralCCW:       return spiralCCWIndices(rows, cols);
    case PathMode::DiagonalDown:    return diagonalDownIndices(rows, cols);
    case PathMode::DiagonalUp:      return diagonalUpIndices(rows, cols);
    case PathMode::ZigzagVertical:  return zigzagVerticalIndices(rows, cols);
    case PathMode::ZigzagHorizontal:return zigzagHorizontalIndices(rows, cols);
    }
    return spiralCWIndices(rows, cols);
}

/* ---- Read along traversal path ---- */

QString RouteCipher8::readPath(const QVector<QVector<QChar>>& grid, PathMode mode) const
{
    int rows = grid.size();
    int cols = (rows > 0) ? grid[0].size() : 0;
    auto indices = getPathIndices(rows, cols, mode);

    QString result;
    result.reserve(indices.size());
    for (int idx : indices) {
        int r = idx / cols;
        int c = idx % cols;
        result.append(grid[r][c]);
    }
    return result;
}

/* ---- Write along traversal path (inverse for decryption) ---- */

QVector<QVector<QChar>> RouteCipher8::writePath(const QString& text, int rows, int cols, PathMode mode) const
{
    auto indices = getPathIndices(rows, cols, mode);
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, m_padding));

    for (int i = 0; i < indices.size() && i < text.size(); ++i) {
        int r = indices[i] / cols;
        int c = indices[i] % cols;
        grid[r][c] = text[i];
    }
    return grid;
}

/* ---- Encrypt ---- */

RouteCipher8::CipherResult RouteCipher8::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    m_rows = 0; m_cols = 0;
    computeGridSize(plaintext.size());
    result.rows = m_rows;
    result.cols = m_cols;
    result.mode = m_mode;

    auto grid = fillGrid(plaintext, m_rows, m_cols);
    result.text = readPath(grid, m_mode);
    result.paddingChars = m_rows * m_cols - plaintext.size();

    m_stats.totalEncrypts++;
    m_stats.lastMode = m_mode;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncrypts + m_stats.totalDecrypts);

    emit encryptDone(m_rows, m_cols, static_cast<int>(m_mode), elapsed);
    return result;
}

/* ---- Decrypt ---- */

RouteCipher8::CipherResult RouteCipher8::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    m_rows = 0; m_cols = 0;
    computeGridSize(ciphertext.size());
    result.rows = m_rows;
    result.cols = m_cols;
    result.mode = m_mode;

    // Write ciphertext along route path, then read row-major
    auto grid = writePath(ciphertext, m_rows, m_cols, m_mode);
    QString decrypted;
    decrypted.reserve(m_rows * m_cols);
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            decrypted.append(grid[r][c]);

    // Strip padding
    while (decrypted.endsWith(m_padding) && !decrypted.isEmpty())
        decrypted.chop(1);

    result.text = decrypted;
    result.paddingChars = m_rows * m_cols - ciphertext.size();

    m_stats.totalDecrypts++;
    m_stats.lastMode = m_mode;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncrypts + m_stats.totalDecrypts);

    emit decryptDone(m_rows, m_cols, static_cast<int>(m_mode), elapsed);
    return result;
}

/* ---- Reset ---- */

void RouteCipher8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
