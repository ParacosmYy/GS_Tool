/**
 * @file SeriatedPlayfair.cpp
 * @brief SeriatedPlayfair 实现
 *
 * 实现序列化Playfair：渐进关键字网格、扩展二合字母频率分析、多表加密。
 */

#include "utils/code202/SeriatedPlayfair.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair::SeriatedPlayfair(QObject *parent) : QObject(parent) {}
SeriatedPlayfair::~SeriatedPlayfair() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair::setKeyword(const QString& keyword) { m_keyword = keyword.toUpper(); }
void SeriatedPlayfair::setPeriod(int period) { m_period = qMax(1, period); }

/* ---- Build 5x5 grid ---- */

QVector<QVector<QChar>> SeriatedPlayfair::buildGrid(const QString& keyword) const
{
    QVector<QVector<QChar>> grid(5, QVector<QChar>(5));
    QVector<bool> used(26, false);
    used['J' - 'A'] = true; // I/J share cell

    int row = 0, col = 0;

    // Place keyword characters
    QString kw = keyword.toUpper();
    for (QChar ch : kw) {
        if (!ch.isLetter()) continue;
        int idx = ch.toLatin1() - 'A';
        if (idx >= 0 && idx < 26 && !used[idx]) {
            used[idx] = true;
            grid[row][col] = ch;
            col++;
            if (col >= 5) { col = 0; row++; }
        }
    }

    // Fill remaining alphabet
    for (int c = 0; c < 26; ++c) {
        if (used[c]) continue;
        grid[row][col] = QChar('A' + c);
        col++;
        if (col >= 5) { col = 0; row++; }
    }
    return grid;
}

/* ---- Find position in grid ---- */

QPair<int, int> SeriatedPlayfair::findPosition(const QVector<QVector<QChar>>& grid, QChar ch) const
{
    ch = ch.toUpper();
    if (ch == 'J') ch = 'I';
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            if (grid[r][c] == ch)
                return {r, c};
    return {0, 0};
}

/* ---- Transform a pair ---- */

QPair<QChar, QChar> SeriatedPlayfair::transformPair(const QVector<QVector<QChar>>& grid,
                                                      QChar a, QChar b, bool enc) const
{
    auto [r1, c1] = findPosition(grid, a);
    auto [r2, c2] = findPosition(grid, b);
    int dir = enc ? 1 : 4; // +1 for encrypt, -1 mod 5 for decrypt

    if (r1 == r2) {
        // Same row: shift columns
        return {grid[r1][(c1 + dir) % 5], grid[r2][(c2 + dir) % 5]};
    } else if (c1 == c2) {
        // Same column: shift rows
        return {grid[(r1 + dir) % 5][c1], grid[(r2 + dir) % 5][c2]};
    } else {
        // Rectangle: swap columns
        return {grid[r1][c2], grid[r2][c1]};
    }
}

/* ---- Generate seriated grids ---- */

QVector<QVector<QVector<QChar>>> SeriatedPlayfair::generateGrids() const
{
    QVector<QVector<QVector<QChar>>> grids;
    grids.reserve(m_period);

    // Progressive keyword-dependent grid: shift keyword for each period
    for (int p = 0; p < m_period; ++p) {
        QString shifted;
        shifted.reserve(m_keyword.size());
        for (int i = 0; i < m_keyword.size(); ++i) {
            int shift = (m_keyword[i].toLatin1() - 'A' + p * 3) % 26;
            shifted.append(QChar('A' + shift));
        }
        grids.append(buildGrid(shifted));
    }
    return grids;
}

/* ---- Prepare text ---- */

QString SeriatedPlayfair::prepareText(const QString& text) const
{
    QString result;
    result.reserve(text.size());
    for (QChar ch : text.toUpper()) {
        if (ch.isLetter()) {
            if (ch == 'J') result.append('I');
            else result.append(ch);
        }
    }
    // Pad with X if odd length
    if (result.size() % 2 != 0) result.append('X');
    return result;
}

/* ---- Encrypt ---- */

QString SeriatedPlayfair::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(plaintext);
    QVector<QVector<QVector<QChar>>> grids = generateGrids();

    QString result;
    result.reserve(prepared.size());

    for (int i = 0; i < prepared.size(); i += 2) {
        int gridIdx = (i / 2) % m_period;
        auto [a, b] = transformPair(grids[gridIdx], prepared[i], prepared[i + 1], true);
        result.append(a);
        result.append(b);
    }

    const_cast<SeriatedPlayfair*>(this)->m_stats.totalOps++;
    const_cast<SeriatedPlayfair*>(this)->m_stats.keyLength = m_keyword.size();
    const_cast<SeriatedPlayfair*>(this)->m_stats.textSize = plaintext.size();
    const_cast<SeriatedPlayfair*>(this)->m_timeSum += timer.elapsed();
    const_cast<SeriatedPlayfair*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<SeriatedPlayfair*>(this)->operationCompleted(
        "encrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString SeriatedPlayfair::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString ct = ciphertext.toUpper();
    QVector<QVector<QVector<QChar>>> grids = generateGrids();

    QString result;
    result.reserve(ct.size());

    for (int i = 0; i + 1 < ct.size(); i += 2) {
        int gridIdx = (i / 2) % m_period;
        auto [a, b] = transformPair(grids[gridIdx], ct[i], ct[i + 1], false);
        result.append(a);
        result.append(b);
    }

    const_cast<SeriatedPlayfair*>(this)->m_stats.totalOps++;
    const_cast<SeriatedPlayfair*>(this)->m_timeSum += timer.elapsed();
    const_cast<SeriatedPlayfair*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<SeriatedPlayfair*>(this)->operationCompleted(
        "decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Digraph frequency analysis ---- */

QVector<QVector<double>> SeriatedPlayfair::digraphFrequency(const QString& text) const
{
    QVector<QVector<double>> freq(26, QVector<double>(26, 0.0));
    QString prepared;
    for (QChar ch : text.toUpper())
        if (ch.isLetter()) prepared.append(ch == 'J' ? 'I' : ch);

    int count = 0;
    for (int i = 0; i + 1 < prepared.size(); i += 2) {
        int a = prepared[i].toLatin1() - 'A';
        int b = prepared[i + 1].toLatin1() - 'A';
        if (a >= 0 && a < 26 && b >= 0 && b < 26) {
            freq[a][b]++;
            count++;
        }
    }

    // Normalize to probabilities
    if (count > 0) {
        for (int i = 0; i < 26; ++i)
            for (int j = 0; j < 26; ++j)
                freq[i][j] /= count;
    }
    return freq;
}

/* ---- Reset ---- */

void SeriatedPlayfair::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
