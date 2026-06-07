/**
 * @file BazeleriesCode.cpp
 * @brief BazeleriesCode 实现
 *
 * 实现Bazeleries密码：不规则Polybius矩形映射、列置换混合加解密。
 */

#include "utils/code189/BazeleriesCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode::BazeleriesCode(QObject *parent) : QObject(parent)
{
    buildGrid();
}

BazeleriesCode::~BazeleriesCode() = default;

/* ---- Configuration ---- */

void BazeleriesCode::setKeyword(const QString& key)
{
    m_keyword = key.toUpper();
    buildGrid();
}

void BazeleriesCode::setTranspositionKey(const QString& key)
{
    m_transKey = key.toUpper();
}

void BazeleriesCode::setGridRows(int rows) { m_gridRows = qMax(2, rows); buildGrid(); }
void BazeleriesCode::setGridCols(int cols) { m_gridCols = qMax(2, cols); buildGrid(); }

/* ---- Build irregular Polybius rectangle ---- */

void BazeleriesCode::buildGrid()
{
    m_grid.resize(m_gridRows);
    for (auto& row : m_grid)
        row.resize(m_gridCols, QChar(' '));

    // Build character pool: keyword (deduplicated) + remaining A-Z + digits 0-9
    QString pool;
    QSet<QChar> used;
    for (QChar ch : m_keyword) {
        if (ch.isLetterOrNumber() && !used.contains(ch)) {
            pool.append(ch);
            used.insert(ch);
        }
    }
    for (char c = 'A'; c <= 'Z'; ++c) {
        QChar ch(c);
        if (!used.contains(ch)) { pool.append(ch); used.insert(ch); }
    }
    for (char c = '0'; c <= '9'; ++c) {
        QChar ch(c);
        if (!used.contains(ch)) { pool.append(ch); used.insert(ch); }
    }

    // Fill grid row by row with irregular wrapping
    int pi = 0;
    for (int r = 0; r < m_gridRows; ++r) {
        for (int c = 0; c < m_gridCols; ++c) {
            if (pi < pool.size())
                m_grid[r][c] = pool[pi++];
            else
                m_grid[r][c] = QChar(' ');
        }
    }
}

/* ---- Find character in grid ---- */

QPair<int, int> BazeleriesCode::findInGrid(QChar ch) const
{
    ch = ch.toUpper();
    for (int r = 0; r < m_grid.size(); ++r) {
        for (int c = 0; c < m_grid[r].size(); ++c) {
            if (m_grid[r][c] == ch)
                return {r, c};
        }
    }
    return {-1, -1};
}

/* ---- Read grid cell ---- */

QChar BazeleriesCode::gridAt(int row, int col) const
{
    if (row < 0 || row >= m_grid.size()) return QChar();
    if (col < 0 || col >= m_grid[row].size()) return QChar();
    return m_grid[row][col];
}

/* ---- Columnar transposition encrypt ---- */

QString BazeleriesCode::columnarEncrypt(const QString& text)
{
    if (m_transKey.isEmpty() || text.isEmpty()) return text;

    int cols = m_transKey.size();
    int rows = qCeil(static_cast<double>(text.size()) / cols);

    // Build column order from key
    QVector<QPair<QChar, int>> keyOrder;
    for (int i = 0; i < cols; ++i)
        keyOrder.append({m_transKey[i], i});
    std::sort(keyOrder.begin(), keyOrder.end());

    QVector<int> order(cols);
    for (int i = 0; i < cols; ++i)
        order[i] = keyOrder[i].second;

    // Write into grid row-wise
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
    for (int i = 0; i < text.size(); ++i)
        grid[i / cols][i % cols] = text[i];

    // Read out column-wise in key order
    QString result;
    for (int c : order) {
        for (int r = 0; r < rows; ++r) {
            if (grid[r][c] != QChar(' '))
                result.append(grid[r][c]);
        }
    }
    return result;
}

/* ---- Columnar transposition decrypt ---- */

QString BazeleriesCode::columnarDecrypt(const QString& text)
{
    if (m_transKey.isEmpty() || text.isEmpty()) return text;

    int cols = m_transKey.size();
    int rows = qCeil(static_cast<double>(text.size()) / cols);

    QVector<QPair<QChar, int>> keyOrder;
    for (int i = 0; i < cols; ++i)
        keyOrder.append({m_transKey[i], i});
    std::sort(keyOrder.begin(), keyOrder.end());

    QVector<int> order(cols);
    for (int i = 0; i < cols; ++i)
        order[i] = keyOrder[i].second;

    // Determine length of each column
    QVector<int> colLens(cols, rows);
    int extra = rows * cols - text.size();
    for (int i = extra - 1; i >= 0; --i)
        colLens[order[cols - 1 - i]]--;

    // Read column-wise
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar(' ')));
    int pos = 0;
    for (int c : order) {
        for (int r = 0; r < colLens[c]; ++r) {
            if (pos < text.size())
                grid[r][c] = text[pos++];
        }
    }

    // Read row-wise
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (grid[r][c] != QChar(' '))
                result.append(grid[r][c]);
    return result;
}

/* ---- Encrypt ---- */

QString BazeleriesCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Polybius substitution (row, col pairs)
    QString substituted;
    for (QChar ch : plaintext.toUpper()) {
        auto pos = findInGrid(ch);
        if (pos.first >= 0) {
            substituted.append(QChar('0' + pos.first));
            substituted.append(QChar('0' + pos.second));
        }
    }

    // Step 2: Columnar transposition
    QString cipher = columnarEncrypt(substituted);

    m_stats.totalOperations++;
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = cipher.size();
    m_stats.gridSize = m_gridRows * m_gridCols;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", plaintext.size(), cipher.size(), timer.elapsed());
    return cipher;
}

/* ---- Decrypt ---- */

QString BazeleriesCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Reverse columnar transposition
    QString substituted = columnarDecrypt(ciphertext);

    // Step 2: Reverse Polybius substitution
    QString result;
    for (int i = 0; i + 1 < substituted.size(); i += 2) {
        int row = substituted[i].digitValue();
        int col = substituted[i + 1].digitValue();
        if (row >= 0 && col >= 0) {
            QChar ch = gridAt(row, col);
            if (ch != QChar(' '))
                result.append(ch);
        }
    }

    m_stats.totalOperations++;
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.gridSize = m_gridRows * m_gridCols;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decrypt", ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void BazeleriesCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
