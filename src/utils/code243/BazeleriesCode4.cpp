/**
 * @file BazeleriesCode4.cpp
 * @brief BazeleriesCode4 实现
 *
 * 实现Bazeleries密码：扰乱列置换与关键字不规则列读取顺序。
 */

#include "utils/code243/BazeleriesCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode4::BazeleriesCode4(QObject *parent) : QObject(parent) {}
BazeleriesCode4::~BazeleriesCode4() = default;

/* ---- Configuration ---- */

void BazeleriesCode4::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper().remove(QLatin1Char(' '));
}

void BazeleriesCode4::setDisruptionRows(int rows)
{
    m_disruptionRows = qMax(0, rows);
}

/* ---- Derive column reading order from keyword ---- */

QVector<int> BazeleriesCode4::deriveColumnOrder(const QString& keyword) const
{
    int n = keyword.length();
    if (n == 0) return {};

    // Assign ranks based on alphabetical order of keyword characters
    QVector<QPair<QChar, int>> pairs;
    pairs.reserve(n);
    for (int i = 0; i < n; ++i)
        pairs.append({keyword[i], i});

    // Sort by character value then by position for stability
    std::stable_sort(pairs.begin(), pairs.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    // Build reading order: position of i-th ranked column
    QVector<int> order(n);
    for (int rank = 0; rank < n; ++rank)
        order[rank] = pairs[rank].second;

    return order;
}

/* ---- Inverse column order ---- */

QVector<int> BazeleriesCode4::inverseOrder(const QVector<int>& order) const
{
    int n = order.size();
    QVector<int> inv(n);
    for (int i = 0; i < n; ++i)
        inv[order[i]] = i;
    return inv;
}

/* ---- Fill transposition grid with disruption ---- */

QVector<QVector<QChar>> BazeleriesCode4::fillGrid(const QString& text, int cols, int rows) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar::Space));

    // Disruption pattern: first disruptionRows are partially filled
    int disruptionRows = m_disruptionRows > 0 ? m_disruptionRows : rows / 2;
    disruptionRows = qBound(0, disruptionRows, rows - 1);

    int textIdx = 0;
    int textLen = text.length();

    // Disruption rows: fill only first (row+1) columns in row r
    for (int r = 0; r < disruptionRows && textIdx < textLen; ++r) {
        int fillCols = qMin(r + 1, cols);
        for (int c = 0; c < fillCols && textIdx < textLen; ++c) {
            grid[r][c] = text[textIdx++];
        }
    }

    // Normal rows: fill all columns
    for (int r = disruptionRows; r < rows && textIdx < textLen; ++r) {
        for (int c = 0; c < cols && textIdx < textLen; ++c) {
            grid[r][c] = text[textIdx++];
        }
    }

    // Fill remaining disruption cells (upper-right triangle)
    for (int r = 0; r < disruptionRows && textIdx < textLen; ++r) {
        int startCol = qMin(r + 1, cols);
        for (int c = startCol; c < cols && textIdx < textLen; ++c) {
            if (grid[r][c] == QChar::Space)
                grid[r][c] = text[textIdx++];
        }
    }

    return grid;
}

/* ---- Read columns in given order ---- */

QString BazeleriesCode4::readColumns(const QVector<QVector<QChar>>& grid,
                                       const QVector<int>& order) const
{
    QString result;
    int rows = grid.size();
    if (rows == 0) return result;
    int cols = grid[0].size();

    for (int colIdx : order) {
        if (colIdx >= cols) continue;
        for (int r = 0; r < rows; ++r) {
            QChar ch = grid[r][colIdx];
            if (ch != QChar::Space)
                result += ch;
        }
    }
    return result;
}

/* ---- Write ciphertext into grid columns ---- */

QVector<QVector<QChar>> BazeleriesCode4::writeColumns(const QString& text, int cols, int rows,
                                                        const QVector<int>& order) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar::Space));

    // Determine column lengths considering disruption pattern
    int disruptionRows = m_disruptionRows > 0 ? m_disruptionRows : rows / 2;
    disruptionRows = qBound(0, disruptionRows, rows - 1);

    // Count actual filled cells per column
    QVector<int> colLengths(cols, 0);
    for (int r = disruptionRows; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            colLengths[c]++;
    for (int r = 0; r < disruptionRows; ++r)
        for (int c = 0; c < cols; ++c)
            colLengths[c]++;

    // Write text into columns following the order
    int textIdx = 0;
    for (int colIdx : order) {
        for (int r = 0; r < rows && textIdx < text.length(); ++r) {
            grid[r][colIdx] = text[textIdx++];
        }
    }

    return grid;
}

/* ---- Read grid row-wise ---- */

QString BazeleriesCode4::readRows(const QVector<QVector<QChar>>& grid) const
{
    QString result;
    for (const auto& row : grid)
        for (QChar ch : row)
            if (ch != QChar::Space)
                result += ch;
    return result;
}

/* ---- Encrypt ---- */

QString BazeleriesCode4::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keyword.isEmpty()) return plaintext;

    QString text = plaintext.toUpper().remove(QLatin1Char(' '));
    int cols = m_keyword.length();
    int rows = qMax(1, static_cast<int>(qCeil(static_cast<double>(text.length()) / cols)));

    // Pad text to fill grid
    while (text.length() < rows * cols)
        text += QLatin1Char('X');

    QVector<int> order = deriveColumnOrder(m_keyword);
    QVector<QVector<QChar>> grid = fillGrid(text, cols, rows);
    QString result = readColumns(grid, order);

    m_stats.numEncrypts++;
    m_stats.keywordLength = cols;
    m_stats.numColumns = cols;
    m_stats.inputLength = text.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("encrypt", result.length(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString BazeleriesCode4::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keyword.isEmpty()) return ciphertext;

    QString text = ciphertext.toUpper().remove(QLatin1Char(' '));
    int cols = m_keyword.length();
    int rows = qMax(1, static_cast<int>(qCeil(static_cast<double>(text.length()) / cols)));

    QVector<int> order = deriveColumnOrder(m_keyword);
    QVector<QVector<QChar>> grid = writeColumns(text, cols, rows, order);
    QString result = readRows(grid);

    m_stats.numDecrypts++;
    m_stats.keywordLength = cols;
    m_stats.numColumns = cols;
    m_stats.inputLength = text.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("decrypt", result.length(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void BazeleriesCode4::resetStatistics()
{
    m_keyword.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
