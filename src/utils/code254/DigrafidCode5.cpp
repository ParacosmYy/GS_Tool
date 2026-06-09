/**
 * @file DigrafidCode5.cpp
 * @brief DigrafidCode5 实现
 *
 * 实现Digrafid密码：Bifid坐标拆分与关键字周期矩形转置。
 */

#include "utils/code254/DigrafidCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode5::DigrafidCode5(QObject *parent)
    : QObject(parent) { buildGrid(); }
DigrafidCode5::~DigrafidCode5() = default;

/* ---- Build 6x6 grid from keyword ---- */

void DigrafidCode5::buildGrid()
{
    // 36 symbols: A-Z (26) + 0-9 (10)
    m_grid.clear();
    m_grid.resize(36);

    QVector<bool> used(36, false);
    int pos = 0;

    // Place keyword characters first (deduplicated)
    QString kw = m_keyword.toUpper();
    for (int i = 0; i < kw.size(); ++i) {
        QChar ch = kw[i];
        int idx = -1;
        if (ch >= 'A' && ch <= 'Z') idx = ch.unicode() - 'A';
        else if (ch >= '0' && ch <= '9') idx = 26 + ch.digitValue();
        else continue;
        if (idx >= 0 && !used[idx]) {
            m_grid[pos++] = ch;
            used[idx] = true;
        }
    }

    // Fill remaining in order: A-Z, then 0-9
    for (int c = 0; c < 26; ++c) {
        if (!used[c]) m_grid[pos++] = QChar('A' + c);
    }
    for (int d = 0; d < 10; ++d) {
        if (!used[26 + d]) m_grid[pos++] = QChar('0' + d);
    }
}

/* ---- Set keyword ---- */

void DigrafidCode5::setKeyword(const QString& keyword)
{
    m_keyword = keyword;
    buildGrid();
    m_stats.keyLength = keyword.length();
}

/* ---- Set period ---- */

void DigrafidCode5::setPeriod(int period)
{
    m_period = qMax(0, period);
    m_stats.period = m_period;
}

/* ---- Find coordinates ---- */

bool DigrafidCode5::findCoord(QChar ch, int& row, int& col) const
{
    ch = ch.toUpper();
    for (int i = 0; i < 36; ++i) {
        if (m_grid[i] == ch) {
            row = i / 6;
            col = i % 6;
            return true;
        }
    }
    return false;
}

/* ---- Grid character at position ---- */

QChar DigrafidCode5::gridChar(int row, int col) const
{
    if (row < 0 || row >= 6 || col < 0 || col >= 6) return ' ';
    return m_grid[row * 6 + col];
}

/* ---- Normalize text ---- */

QString DigrafidCode5::normalize(const QString& text) const
{
    QString result;
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();
        if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
            result.append(ch);
    }
    return result;
}

/* ---- Core Bifid-style coordinate split ---- */

QString DigrafidCode5::processPairs(const QString& text, bool encrypt) const
{
    int n = text.size();
    if (n < 2) return text;

    // Extract row and column coordinates for each character
    QVector<int> rows, cols;
    rows.reserve(n);
    cols.reserve(n);
    for (int i = 0; i < n; ++i) {
        int r, c;
        if (findCoord(text[i], r, c)) {
            rows.append(r);
            cols.append(c);
        } else {
            rows.append(0);
            cols.append(0);
        }
    }

    // Bifid split: interleave row coords, then col coords
    int period = (m_period > 0) ? qMin(m_period, n) : n;
    QString result;

    for (int start = 0; start < n; start += period) {
        int end = qMin(start + period, n);
        int len = end - start;

        // Collect row coords then col coords for this period block
        QVector<int> combined;
        combined.reserve(2 * len);
        for (int i = start; i < end; ++i) combined.append(rows[i]);
        for (int i = start; i < end; ++i) combined.append(cols[i]);

        // Apply rectangular transposition
        int width = (len > 0) ? len : 1;
        combined = applyTransposition(combined, width, encrypt);

        // Recombine pairs into characters
        for (int i = 0; i + 1 < combined.size(); i += 2) {
            result.append(gridChar(combined[i], combined[i + 1]));
        }
    }
    return result;
}

/* ---- Rectangular transposition ---- */

QVector<int> DigrafidCode5::applyTransposition(const QVector<int>& indices,
                                                int width, bool encrypt) const
{
    int total = indices.size();
    if (total <= 0) return indices;

    // Keyword-derived column order
    int cols = width;
    int rows = (total + cols - 1) / cols;

    // Derive column permutation from keyword character positions
    QVector<int> colOrder;
    colOrder.reserve(cols);
    QString kw = m_keyword.toUpper();
    for (int i = 0; i < cols; ++i) {
        colOrder.append(i);
    }
    // Sort by keyword character (stable), default to position
    for (int i = 0; i < qMin(kw.size(), cols); ++i) {
        for (int j = i + 1; j < qMin(kw.size(), cols); ++j) {
            if (kw[i] > kw[j]) {
                std::swap(colOrder[i], colOrder[j]);
            }
        }
    }

    QVector<int> result;
    result.reserve(total);

    if (encrypt) {
        // Read columns in permuted order
        for (int c = 0; c < cols; ++c) {
            int srcCol = colOrder[c];
            for (int r = 0; r < rows; ++r) {
                int idx = r * cols + srcCol;
                if (idx < total) result.append(indices[idx]);
            }
        }
    } else {
        // Inverse: scatter back to original column positions
        QVector<int> restored(total, 0);
        int src = 0;
        for (int c = 0; c < cols; ++c) {
            int destCol = colOrder[c];
            for (int r = 0; r < rows; ++r) {
                int idx = r * cols + destCol;
                if (idx < total && src < total)
                    restored[idx] = indices[src++];
            }
        }
        result = restored;
    }
    return result;
}

/* ---- Encrypt ---- */

QString DigrafidCode5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString norm = normalize(plaintext);
    QString result = processPairs(norm, true);

    m_stats.numEncryptions++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptionCompleted(norm.size(), result.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString DigrafidCode5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString norm = normalize(ciphertext);
    QString result = processPairs(norm, false);

    m_stats.numDecryptions++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(norm.size(), result.size(), elapsed);
    return result;
}

/* ---- Reset ---- */

void DigrafidCode5::resetStatistics()
{
    m_keyword.clear();
    m_period = 0;
    buildGrid();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
