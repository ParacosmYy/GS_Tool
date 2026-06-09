/**
 * @file RouteCipher5.cpp
 * @brief RouteCipher5 实现
 *
 * 实现路线密码：行列转置与可配置读取模式空字符填充。
 */

#include "utils/code256/RouteCipher5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RouteCipher5::RouteCipher5(QObject *parent)
    : QObject(parent) {}
RouteCipher5::~RouteCipher5() = default;

/* ---- Configuration ---- */

void RouteCipher5::setRows(int rows) { m_rows = qMax(1, rows); }
void RouteCipher5::setCols(int cols) { m_cols = qMax(1, cols); }
void RouteCipher5::setPattern(Pattern pattern) { m_pattern = pattern; }
void RouteCipher5::setDirection(Direction dir) { m_direction = dir; }
void RouteCipher5::setPaddingChar(QChar ch) { m_paddingChar = ch; }

/* ---- Build grid from text ---- */

QVector<QVector<QChar>> RouteCipher5::buildGrid(const QString& text, int& paddedLen) const
{
    int totalCells = m_rows * m_cols;
    paddedLen = qMax(text.length(), totalCells);
    // Round up to fill the grid
    if (paddedLen % totalCells != 0)
        paddedLen = ((paddedLen / totalCells) + 1) * totalCells;

    QVector<QVector<QChar>> grid(m_rows, QVector<QChar>(m_cols, m_paddingChar));

    if (m_direction == Direction::RowFirst) {
        for (int i = 0; i < qMin(text.length(), paddedLen); ++i)
            grid[i / m_cols][i % m_cols] = (i < text.length()) ? text[i] : m_paddingChar;
    } else {
        for (int i = 0; i < qMin(text.length(), paddedLen); ++i)
            grid[i % m_rows][i / m_rows] = (i < text.length()) ? text[i] : m_paddingChar;
    }
    return grid;
}

/* ---- Spiral-in read ---- */

QString RouteCipher5::readSpiralIn(const QVector<QVector<QChar>>& grid) const
{
    QString result;
    int top = 0, bottom = m_rows - 1, left = 0, right = m_cols - 1;

    while (top <= bottom && left <= right) {
        // Right across top
        for (int c = left; c <= right; ++c) result.append(grid[top][c]);
        top++;
        // Down right side
        for (int r = top; r <= bottom; ++r) result.append(grid[r][right]);
        right--;
        // Left across bottom
        if (top <= bottom) {
            for (int c = right; c >= left; --c) result.append(grid[bottom][c]);
            bottom--;
        }
        // Up left side
        if (left <= right) {
            for (int r = bottom; r >= top; --r) result.append(grid[r][left]);
            left++;
        }
    }
    return result;
}

/* ---- Spiral-out read ---- */

QString RouteCipher5::readSpiralOut(const QVector<QVector<QChar>>& grid) const
{
    // Read spiral-in then reverse
    QString inward = readSpiralIn(grid);
    QString reversed;
    for (int i = inward.length() - 1; i >= 0; --i)
        reversed.append(inward[i]);
    return reversed;
}

/* ---- Zigzag row read ---- */

QString RouteCipher5::readZigzagRow(const QVector<QVector<QChar>>& grid) const
{
    QString result;
    for (int r = 0; r < m_rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < m_cols; ++c) result.append(grid[r][c]);
        } else {
            for (int c = m_cols - 1; c >= 0; --c) result.append(grid[r][c]);
        }
    }
    return result;
}

/* ---- Zigzag col read ---- */

QString RouteCipher5::readZigzagCol(const QVector<QVector<QChar>>& grid) const
{
    QString result;
    for (int c = 0; c < m_cols; ++c) {
        if (c % 2 == 0) {
            for (int r = 0; r < m_rows; ++r) result.append(grid[r][c]);
        } else {
            for (int r = m_rows - 1; r >= 0; --r) result.append(grid[r][c]);
        }
    }
    return result;
}

/* ---- Diagonal read ---- */

QString RouteCipher5::readDiagonal(const QVector<QVector<QChar>>& grid) const
{
    QString result;
    // Traverse anti-diagonals
    for (int sum = 0; sum < m_rows + m_cols - 1; ++sum) {
        if (sum % 2 == 0) {
            for (int r = qMin(sum, m_rows - 1); r >= 0; --r) {
                int c = sum - r;
                if (c >= 0 && c < m_cols) result.append(grid[r][c]);
            }
        } else {
            for (int r = 0; r <= qMin(sum, m_rows - 1); ++r) {
                int c = sum - r;
                if (c >= 0 && c < m_cols) result.append(grid[r][c]);
            }
        }
    }
    return result;
}

/* ---- Read grid using current pattern ---- */

QString RouteCipher5::readGrid(const QVector<QVector<QChar>>& grid) const
{
    switch (m_pattern) {
    case Pattern::SpiralIn:  return readSpiralIn(grid);
    case Pattern::SpiralOut: return readSpiralOut(grid);
    case Pattern::ZigzagRow: return readZigzagRow(grid);
    case Pattern::ZigzagCol: return readZigzagCol(grid);
    case Pattern::Diagonal:  return readDiagonal(grid);
    }
    return readSpiralIn(grid);
}

/* ---- Fill grid from spiral-in sequence ---- */

void RouteCipher5::fillSpiralIn(QVector<QVector<QChar>>& grid, const QString& text)
{
    int idx = 0;
    int top = 0, bottom = m_rows - 1, left = 0, right = m_cols - 1;

    while (top <= bottom && left <= right && idx < text.length()) {
        for (int c = left; c <= right && idx < text.length(); ++c)
            grid[top][c] = text[idx++];
        top++;
        for (int r = top; r <= bottom && idx < text.length(); ++r)
            grid[r][right] = text[idx++];
        right--;
        if (top <= bottom) {
            for (int c = right; c >= left && idx < text.length(); --c)
                grid[bottom][c] = text[idx++];
            bottom--;
        }
        if (left <= right) {
            for (int r = bottom; r >= top && idx < text.length(); --r)
                grid[r][left] = text[idx++];
            left++;
        }
    }
}

/* ---- Fill grid from zigzag row sequence ---- */

void RouteCipher5::fillZigzagRow(QVector<QVector<QChar>>& grid, const QString& text)
{
    int idx = 0;
    for (int r = 0; r < m_rows && idx < text.length(); ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < m_cols && idx < text.length(); ++c)
                grid[r][c] = text[idx++];
        } else {
            for (int c = m_cols - 1; c >= 0 && idx < text.length(); --c)
                grid[r][c] = text[idx++];
        }
    }
}

/* ---- Fill grid using current pattern ---- */

void RouteCipher5::fillGrid(QVector<QVector<QChar>>& grid, const QString& text)
{
    switch (m_pattern) {
    case Pattern::SpiralIn:
    case Pattern::SpiralOut:
        fillSpiralIn(grid, text);
        break;
    case Pattern::ZigzagRow:
    case Pattern::ZigzagCol:
        fillZigzagRow(grid, text);
        break;
    case Pattern::Diagonal:
        // For diagonal, fill row-by-row as baseline
        fillZigzagRow(grid, text);
        break;
    }
}

/* ---- Encrypt ---- */

QString RouteCipher5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    int paddedLen = 0;
    auto grid = buildGrid(plaintext, paddedLen);
    QString result = readGrid(grid);

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.length();
    m_stats.paddedLength = paddedLen;
    m_stats.numRows = m_rows;
    m_stats.numCols = m_cols;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(plaintext.length(), result.length(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString RouteCipher5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Fill grid from cipher pattern then read row-by-row
    QVector<QVector<QChar>> grid(m_rows, QVector<QChar>(m_cols, m_paddingChar));
    fillGrid(grid, ciphertext);

    // Read out in row-major order
    QString result;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (grid[r][c] != m_paddingChar)
                result.append(grid[r][c]);

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(ciphertext.length(), result.length(), elapsed);
    return result;
}

/* ---- Reset ---- */

void RouteCipher5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
