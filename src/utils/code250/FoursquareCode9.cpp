/**
 * @file FoursquareCode9.cpp
 * @brief FoursquareCode9 实现
 *
 * 实现四方密码：关键词驱动扩展网格与三层转置坐标对编码。
 */

#include "utils/code250/FoursquareCode9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode9::FoursquareCode9(QObject *parent) : QObject(parent)
{
    // Initialize all four grids with standard alphabet
    m_gridTL.resize(GRID);
    m_gridTR.resize(GRID);
    m_gridBL.resize(GRID);
    m_gridBR.resize(GRID);
    for (int r = 0; r < GRID; ++r) {
        m_gridTL[r].resize(GRID);
        m_gridTR[r].resize(GRID);
        m_gridBL[r].resize(GRID);
        m_gridBR[r].resize(GRID);
        for (int c = 0; c < GRID; ++c) {
            int ch = r * GRID + c;
            m_gridTL[r][c] = ch;
            m_gridBR[r][c] = ch;
        }
    }
    // TR and BL built from keywords
    buildGrid(QString(), m_gridTR);
    buildGrid(QString(), m_gridBL);
}

FoursquareCode9::~FoursquareCode9() = default;

/* ---- Char mapping ---- */

int FoursquareCode9::charToIndex(QChar c) { return c.toUpper().toLatin1() - 'A'; }
QChar FoursquareCode9::indexToChar(int idx) { return QChar('A' + (idx >= 9 ? idx + 1 : idx)); }

/* ---- Build 5x5 grid from keyword ---- */

void FoursquareCode9::buildGrid(const QString& keyword, QVector<QVector<int>>& grid)
{
    QVector<bool> used(26, false);
    used[9] = true; // Skip J
    QVector<int> chars;

    // Add keyword characters first
    for (QChar c : keyword.toUpper()) {
        int idx = charToIndex(c);
        if (idx >= 0 && idx < 26 && !used[idx]) {
            chars.append(idx);
            used[idx] = true;
        }
    }

    // Fill remaining alphabet
    for (int i = 0; i < 26; ++i) {
        if (!used[i]) chars.append(i);
    }

    // Fill grid
    for (int r = 0; r < GRID; ++r)
        for (int c = 0; c < GRID; ++c)
            grid[r][c] = chars[r * GRID + c];
}

/* ---- Find position in grid ---- */

void FoursquareCode9::findPosition(const QVector<QVector<int>>& grid, int ch,
                                     int& row, int& col) const
{
    for (int r = 0; r < GRID; ++r)
        for (int c = 0; c < GRID; ++c)
            if (grid[r][c] == ch) { row = r; col = c; return; }
    row = 0; col = 0;
}

/* ---- Preprocess text ---- */

QString FoursquareCode9::preprocess(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c >= 'A' && c <= 'Z') {
            result.append(c == 'J' ? 'I' : c);
        }
    }
    if (result.size() % 2 != 0) result.append('X');
    return result;
}

/* ---- Triple-layer transposition ---- */

QVector<int> FoursquareCode9::tripleTranspose(const QVector<int>& coords,
                                                bool encrypt) const
{
    int n = coords.size();
    QVector<int> result(n);

    // Layer 1: Row-column swap
    for (int i = 0; i < n; i += 2) {
        if (encrypt) {
            result[i] = (coords[i] + coords[i + 1]) % GRID;
            result[i + 1] = (coords[i + 1] - coords[i] + GRID) % GRID;
        } else {
            result[i + 1] = (coords[i] + coords[i + 1]) % GRID;
            result[i] = (coords[i + 1] - result[i + 1] + GRID) % GRID;
        }
    }

    // Layer 2: Shift by position
    QVector<int> temp = result;
    for (int i = 0; i < n; ++i) {
        int shift = encrypt ? (i + 1) : -(i + 1);
        result[i] = ((temp[i] + shift) % GRID + GRID) % GRID;
    }

    // Layer 3: Pair swap
    temp = result;
    for (int i = 0; i < n; i += 4) {
        if (i + 3 < n) {
            result[i] = temp[i + 2];
            result[i + 1] = temp[i + 3];
            result[i + 2] = temp[i];
            result[i + 3] = temp[i + 1];
        }
    }

    return result;
}

/* ---- Set keywords ---- */

void FoursquareCode9::setKeywords(const QString& kw1, const QString& kw2)
{
    m_keyword1 = kw1;
    m_keyword2 = kw2;
    buildGrid(kw1, m_gridTR);
    buildGrid(kw2, m_gridBL);
}

/* ---- Encrypt ---- */

QString FoursquareCode9::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString processed = preprocess(plaintext);
    int n = processed.size();
    if (n < 2) return {};

    // Extract coordinate pairs from TL and BR grids
    QVector<int> coords(n);
    for (int i = 0; i < n; i += 2) {
        int r1, c1, r2, c2;
        findPosition(m_gridTL, charToIndex(processed[i]), r1, c1);
        findPosition(m_gridBR, charToIndex(processed[i + 1]), r2, c2);
        coords[i] = r1;
        coords[i + 1] = c2;
    }

    // Triple-layer transposition
    coords = tripleTranspose(coords, true);

    // Map back from TR and BL grids
    QString result;
    for (int i = 0; i < n; i += 2) {
        int row = coords[i];
        int col = coords[i + 1];
        result.append(indexToChar(m_gridTR[row][col]));
        result.append(indexToChar(m_gridBL[row][col]));
    }

    m_stats.inputLength = processed.size();
    m_stats.outputLength = result.size();
    m_stats.numTranspositions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(true, result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode9::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString processed = ciphertext.toUpper();
    int n = processed.size();
    if (n < 2) return {};

    // Extract from TR and BL grids
    QVector<int> coords(n);
    for (int i = 0; i < n; i += 2) {
        int r, c;
        findPosition(m_gridTR, charToIndex(processed[i]), r, c);
        coords[i] = r;
        findPosition(m_gridBL, charToIndex(processed[i + 1]), r, c);
        coords[i + 1] = c;
    }

    // Inverse triple-layer transposition
    coords = tripleTranspose(coords, false);

    // Map back from TL and BR grids
    QString result;
    for (int i = 0; i < n; i += 2) {
        int row = coords[i];
        int col = coords[i + 1];
        result.append(indexToChar(m_gridTL[row][col]));
        result.append(indexToChar(m_gridBR[row][col]));
    }

    m_stats.numTranspositions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(false, result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void FoursquareCode9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
