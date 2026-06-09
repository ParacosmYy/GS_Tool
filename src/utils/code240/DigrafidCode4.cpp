/**
 * @file DigrafidCode4.cpp
 * @brief DigrafidCode4 实现
 *
 * 实现Digrafid密码：二连字替换与分数混合群转置。
 */

#include "utils/code240/DigrafidCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode4::DigrafidCode4(QObject *parent) : QObject(parent)
{
    // Default 26-letter alphabet (A-Z)
    m_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    buildGrid();
}
DigrafidCode4::~DigrafidCode4() = default;

/* ---- Configuration ---- */

void DigrafidCode4::setGridSize(int rows, int cols)
{
    m_rows = qMax(2, rows);
    m_cols = qMax(2, cols);
    buildGrid();
}

void DigrafidCode4::setKey(const QString& key)
{
    m_key = key.toUpper();
    buildGrid();
}

void DigrafidCode4::setPeriod(int period) { m_period = qMax(1, period); }

/* ---- Build mixed-alphabet grid ---- */

void DigrafidCode4::buildGrid()
{
    QString used;
    // Place key characters first (deduplicated)
    for (QChar ch : m_key) {
        if (m_alphabet.contains(ch) && !used.contains(ch))
            used.append(ch);
    }
    // Fill remaining from alphabet
    for (QChar ch : m_alphabet) {
        if (!used.contains(ch))
            used.append(ch);
    }

    int gridSize = m_rows * m_cols;
    m_grid.resize(gridSize);
    int len = qMin(used.size(), gridSize);
    for (int i = 0; i < gridSize; ++i)
        m_grid[i] = (i < len) ? used[i] : QChar('?');
}

/* ---- Find position of character ---- */

bool DigrafidCode4::findPosition(QChar ch, int& row, int& col) const
{
    ch = ch.toUpper();
    for (int i = 0; i < m_grid.size(); ++i) {
        if (m_grid[i] == ch) {
            row = i / m_cols;
            col = i % m_cols;
            return true;
        }
    }
    return false;
}

/* ---- Fractional transposition ---- */

QVector<int> DigrafidCode4::transpose(const QVector<int>& coords, int period) const
{
    int numPairs = coords.size() / 2;
    if (numPairs == 0) return coords;

    QVector<int> result;
    // Process in blocks of period pairs
    for (int block = 0; block < numPairs; block += period) {
        int blockLen = qMin(period, numPairs - block);
        QVector<int> rows, cols;
        for (int i = 0; i < blockLen; ++i) {
            rows.append(coords[(block + i) * 2]);
            cols.append(coords[(block + i) * 2 + 1]);
        }
        // Transpose: read rows then columns in column-major order
        // Write back interleaved: col1_row1, col1_row2, ..., col2_row1, ...
        QVector<int> newRow, newCol;
        for (int c = 0; c < m_cols; ++c) {
            for (int r = 0; r < blockLen; ++r) {
                newRow.append(cols[r]);
                newCol.append(rows[r]);
            }
        }
        int outLen = qMin(blockLen, qMin(newRow.size(), newCol.size()));
        for (int i = 0; i < outLen; ++i) {
            result.append(newRow[i]);
            result.append(newCol[i]);
        }
    }
    return result;
}

QVector<int> DigrafidCode4::reverseTranspose(const QVector<int>& coords, int period) const
{
    // Reverse is same operation (double transposition restores)
    return transpose(coords, period);
}

/* ---- Encrypt ---- */

QString DigrafidCode4::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString pt = plaintext.toUpper();
    // Remove non-alphabet characters
    QString clean;
    for (QChar ch : pt) {
        if (m_alphabet.contains(ch))
            clean.append(ch);
    }
    // Pad to even length
    if (clean.size() % 2 != 0)
        clean.append('X');

    // Step 1: Digram substitution -> coordinates
    QVector<int> coords;
    for (int i = 0; i < clean.size(); i += 2) {
        int r1, c1, r2, c2;
        if (!findPosition(clean[i], r1, c1)) { r1 = 0; c1 = 0; }
        if (!findPosition(clean[i + 1], r2, c2)) { r2 = 0; c2 = 0; }
        coords.append(r1);
        coords.append(c1);
        coords.append(r2);
        coords.append(c2);
    }

    // Step 2: Fractional transposition
    QVector<int> transposed = transpose(coords, m_period);

    // Step 3: Convert back to ciphertext via grid lookup
    QString result;
    for (int i = 0; i + 3 < transposed.size(); i += 4) {
        int r1 = transposed[i] % m_rows;
        int c1 = transposed[i + 1] % m_cols;
        int r2 = transposed[i + 2] % m_rows;
        int c2 = transposed[i + 3] % m_cols;
        int idx1 = r1 * m_cols + c1;
        int idx2 = r2 * m_cols + c2;
        if (idx1 >= 0 && idx1 < m_grid.size()) result.append(m_grid[idx1]);
        if (idx2 >= 0 && idx2 < m_grid.size()) result.append(m_grid[idx2]);
    }

    m_stats.numEncryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptionCompleted(plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString DigrafidCode4::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString ct = ciphertext.toUpper();
    QString clean;
    for (QChar ch : ct) {
        if (m_alphabet.contains(ch))
            clean.append(ch);
    }
    if (clean.size() % 2 != 0)
        clean.append('X');

    // Convert to coordinates
    QVector<int> coords;
    for (int i = 0; i < clean.size(); i += 2) {
        int r1, c1, r2, c2;
        if (!findPosition(clean[i], r1, c1)) { r1 = 0; c1 = 0; }
        if (!findPosition(clean[i + 1], r2, c2)) { r2 = 0; c2 = 0; }
        coords.append(r1);
        coords.append(c1);
        coords.append(r2);
        coords.append(c2);
    }

    // Reverse fractional transposition
    QVector<int> reversed = reverseTranspose(coords, m_period);

    // Convert back to plaintext
    QString result;
    for (int i = 0; i + 3 < reversed.size(); i += 4) {
        int r1 = reversed[i] % m_rows;
        int c1 = reversed[i + 1] % m_cols;
        int r2 = reversed[i + 2] % m_rows;
        int c2 = reversed[i + 3] % m_cols;
        int idx1 = r1 * m_cols + c1;
        int idx2 = r2 * m_cols + c2;
        if (idx1 >= 0 && idx1 < m_grid.size()) result.append(m_grid[idx1]);
        if (idx2 >= 0 && idx2 < m_grid.size()) result.append(m_grid[idx2]);
    }

    m_stats.numDecryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DigrafidCode4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
