/**
 * @file FoursquareCode14.cpp
 * @brief FoursquareCode14 实现
 *
 * 实现四方密码：渐进Polybius网格变异与坐标交织演化表加密。
 */

#include "utils/code287/FoursquareCode14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode14::FoursquareCode14(QObject *parent)
    : QObject(parent)
{
    // Build standard plain alphabet (A-Z without J, merged into I)
    m_plainAlphabet.reserve(25);
    for (int c = 'A'; c <= 'Z'; ++c) {
        if (c == 'J') continue;
        m_plainAlphabet.append(static_cast<QChar>(c));
    }
}

FoursquareCode14::~FoursquareCode14() = default;

/* ---- Configuration ---- */

void FoursquareCode14::setConfig(const Config& cfg)
{
    m_config = cfg;
    m_config.gridMutationRounds = qBound(1, m_config.gridMutationRounds, 10);
}

/* ---- Build 5x5 grid from keyword ---- */

QVector<QChar> FoursquareCode14::buildGrid(const QString& keyword) const
{
    QVector<QChar> grid;
    grid.reserve(25);
    QVector<bool> used(26, false);

    // Add keyword characters first (skip J)
    for (QChar ch : keyword.toUpper()) {
        if (!ch.isLetter()) continue;
        int idx = ch.toLatin1() - 'A';
        if (idx == 9) idx = 8;  // J -> I
        if (!used[idx]) {
            grid.append(static_cast<QChar>('A' + idx));
            used[idx] = true;
        }
    }
    // Fill remaining alphabet
    for (int i = 0; i < 26; ++i) {
        if (i == 9) continue;  // Skip J
        if (!used[i]) grid.append(static_cast<QChar>('A' + i));
    }
    // Pad to 25 if needed
    while (grid.size() < 25) grid.append('Z');
    return grid;
}

/* ---- Progressive grid mutation: swap rows/columns ---- */

void FoursquareCode14::mutateGrid(QVector<QChar>& grid, int round)
{
    // Row swap: swap row (round % 5) with row ((round + 2) % 5)
    int r1 = round % 5;
    int r2 = (round + 2) % 5;
    for (int c = 0; c < 5; ++c)
        std::swap(grid[r1 * 5 + c], grid[r2 * 5 + c]);

    // Column swap: swap col ((round + 1) % 5) with col ((round + 3) % 5)
    int c1 = (round + 1) % 5;
    int c2 = (round + 3) % 5;
    for (int r = 0; r < 5; ++r)
        std::swap(grid[r * 5 + c1], grid[r * 5 + c2]);
}

/* ---- Find char in grid ---- */

QPair<int, int> FoursquareCode14::findInGrid(const QVector<QChar>& grid, QChar ch) const
{
    ch = ch.toUpper();
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] == ch) return {i / 5, i % 5};
    }
    return {0, 0};
}

QChar FoursquareCode14::gridAt(const QVector<QChar>& grid, int row, int col) const
{
    return grid[(row % 5) * 5 + (col % 5)];
}

/* ---- Prepare text ---- */

QString FoursquareCode14::prepareText(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (!ch.isLetter()) continue;
        if (ch == 'J') ch = 'I';
        result.append(ch);
    }
    // Pad with X if odd length
    if (result.size() % 2 != 0) result.append('X');
    return result;
}

/* ---- Coordinate interleaving ---- */

QString FoursquareCode14::interleave(const QString& rowStr, const QString& colStr) const
{
    QString result;
    for (int i = 0; i < rowStr.size(); ++i) {
        result.append(rowStr[i]);
        result.append(colStr[i]);
    }
    return result;
}

QPair<QString, QString> FoursquareCode14::deinterleave(const QString& combined) const
{
    QString rows, cols;
    for (int i = 0; i < combined.size(); i += 2) {
        rows.append(combined[i]);
        if (i + 1 < combined.size()) cols.append(combined[i + 1]);
    }
    return {rows, cols};
}

/* ---- Encrypt ---- */

FoursquareCode14::EncResult FoursquareCode14::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    EncResult result;

    // Build four grids: TL = plain, TR = key1, BL = key2, BR = plain
    m_grids.clear();
    m_grids.append(m_plainAlphabet);            // TL: plain
    m_grids.append(buildGrid(m_config.key1));   // TR: key1
    m_grids.append(buildGrid(m_config.key2));   // BL: key2
    m_grids.append(m_plainAlphabet);            // BR: plain

    // Apply progressive mutations to key grids
    for (int round = 0; round < m_config.gridMutationRounds; ++round) {
        mutateGrid(m_grids[1], round);
        mutateGrid(m_grids[2], round);
    }
    result.gridMutationApplied = m_config.gridMutationRounds;

    QString prepared = prepareText(plaintext);
    result.blockCount = prepared.size() / 2;

    QString rowCoords, colCoords;

    // Foursquare cipher: for each pair (a, b)
    for (int i = 0; i + 1 < prepared.size(); i += 2) {
        QChar a = prepared[i];
        QChar b = prepared[i + 1];

        auto [r1, c1] = findInGrid(m_grids[0], a);  // TL plain
        auto [r2, c2] = findInGrid(m_grids[3], b);  // BR plain

        // Encrypted pair: grid[1](r1, c2) and grid[2](r2, c1)
        QChar enc1 = gridAt(m_grids[1], r1, c2);   // TR
        QChar enc2 = gridAt(m_grids[2], r2, c1);   // BL

        rowCoords.append(enc1);
        colCoords.append(enc2);
    }

    if (m_config.interleaving) {
        result.cipherText = interleave(rowCoords, colCoords);
    } else {
        result.cipherText = rowCoords + colCoords;
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.cipherText.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherDone(plaintext.size(), result.cipherText.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode14::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Rebuild grids identically
    m_grids.clear();
    m_grids.append(m_plainAlphabet);
    m_grids.append(buildGrid(m_config.key1));
    m_grids.append(buildGrid(m_config.key2));
    m_grids.append(m_plainAlphabet);

    for (int round = 0; round < m_config.gridMutationRounds; ++round) {
        mutateGrid(m_grids[1], round);
        mutateGrid(m_grids[2], round);
    }

    QString rowStr, colStr;
    if (m_config.interleaving) {
        auto [rows, cols] = deinterleave(ciphertext);
        rowStr = rows;
        colStr = cols;
    } else {
        int half = ciphertext.size() / 2;
        rowStr = ciphertext.left(half);
        colStr = ciphertext.mid(half);
    }

    QString plain;
    int pairs = qMin(rowStr.size(), colStr.size());
    for (int i = 0; i < pairs; ++i) {
        QChar enc1 = rowStr[i];
        QChar enc2 = colStr[i];

        auto [r1, c2] = findInGrid(m_grids[1], enc1);  // TR
        auto [r2, c1] = findInGrid(m_grids[2], enc2);  // BL

        QChar a = gridAt(m_grids[0], r1, c1);  // TL
        QChar b = gridAt(m_grids[3], r2, c2);  // BR

        plain.append(a);
        plain.append(b);
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return plain;
}

/* ---- Reset ---- */

void FoursquareCode14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_grids.clear();
}
