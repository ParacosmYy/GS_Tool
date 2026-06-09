/**
 * @file FoursquareCode8.cpp
 * @brief FoursquareCode8 实现
 *
 * 实现四方密码：16x16扩展网格与双关键字行列置换编码。
 */

#include "utils/code245/FoursquareCode8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode8::FoursquareCode8(QObject *parent) : QObject(parent)
{
    m_gridTL = buildGrid(QString(), true);
    m_gridTR = buildGrid(m_keyword1, false);
    m_gridBL = buildGrid(m_keyword2, false);
    m_gridBR = buildGrid(QString(), true);
}

FoursquareCode8::~FoursquareCode8() = default;

/* ---- Keyword permutation ---- */

QVector<int> FoursquareCode8::keywordPermutation(const QString& keyword) const
{
    // Build 16-position row permutation from keyword
    // Use character codes modulo 16 to generate permutation
    QVector<int> perm(256);
    for (int i = 0; i < 256; ++i) perm[i] = i;

    if (keyword.isEmpty()) return perm;

    // Fisher-Yates shuffle seeded by keyword hash
    int seed = 0;
    for (int i = 0; i < keyword.size(); ++i)
        seed = seed * 31 + keyword[i].unicode();
    seed = qAbs(seed);
    for (int i = 255; i > 0; --i) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        int j = seed % (i + 1);
        std::swap(perm[i], perm[j]);
    }
    return perm;
}

/* ---- Build grid ---- */

QVector<QVector<int>> FoursquareCode8::buildGrid(const QString& keyword, bool standard) const
{
    QVector<QVector<int>> grid(16);
    QVector<int> perm = standard ? QVector<int>(256) : keywordPermutation(keyword);

    if (standard) {
        for (int i = 0; i < 256; ++i) perm[i] = i;
    }

    for (int r = 0; r < 16; ++r) {
        grid[r].resize(16);
        for (int c = 0; c < 16; ++c)
            grid[r][c] = perm[r * 16 + c];
    }
    return grid;
}

/* ---- Find character in grid ---- */

QPair<int, int> FoursquareCode8::findInGrid(const QVector<QVector<int>>& grid, int code) const
{
    for (int r = 0; r < 16; ++r)
        for (int c = 0; c < 16; ++c)
            if (grid[r][c] == code) return {r, c};
    return {0, 0};
}

/* ---- Text to pairs ---- */

QVector<QPair<int, int>> FoursquareCode8::textToPairs(const QString& text) const
{
    QVector<QPair<int, int>> pairs;
    int i = 0;
    while (i + 1 < text.size()) {
        pairs.append({text[i].unicode() & 0xFF, text[i + 1].unicode() & 0xFF});
        i += 2;
    }
    // Pad with space if odd length
    if (i < text.size())
        pairs.append({text[i].unicode() & 0xFF, ' '});
    return pairs;
}

/* ---- Set keywords ---- */

void FoursquareCode8::setKeyword1(const QString& keyword)
{
    m_keyword1 = keyword;
    m_gridTR = buildGrid(keyword, false);
}

void FoursquareCode8::setKeyword2(const QString& keyword)
{
    m_keyword2 = keyword;
    m_gridBL = buildGrid(keyword, false);
}

/* ---- Encrypt ---- */

QString FoursquareCode8::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    auto pairs = textToPairs(plaintext);
    QString result;
    for (const auto& [a, b] : pairs) {
        // Find a in TL grid, b in BR grid
        auto [r1, c1] = findInGrid(m_gridTL, a);
        auto [r2, c2] = findInGrid(m_gridBR, b);
        // Read from TR and BL using cross coordinates
        int encA = m_gridTR[r1][c2];
        int encB = m_gridBL[r2][c1];
        result.append(QChar(encA));
        result.append(QChar(encB));
    }

    const_cast<FoursquareCode8*>(this)->m_stats.numEncryptions++;
    const_cast<FoursquareCode8*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode8*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<FoursquareCode8*>(this)->encryptionCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode8::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    auto pairs = textToPairs(ciphertext);
    QString result;
    for (const auto& [a, b] : pairs) {
        // Find encrypted a in TR grid, encrypted b in BL grid
        auto [r1, c1] = findInGrid(m_gridTR, a);
        auto [r2, c2] = findInGrid(m_gridBL, b);
        // Read from TL and BR using cross coordinates
        int decA = m_gridTL[r1][c2];
        int decB = m_gridBR[r2][c1];
        result.append(QChar(decA));
        result.append(QChar(decB));
    }

    const_cast<FoursquareCode8*>(this)->m_stats.numDecryptions++;
    const_cast<FoursquareCode8*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode8*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<FoursquareCode8*>(this)->decryptionCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---- Get grid ---- */

QVector<QVector<int>> FoursquareCode8::grid(int squareIndex) const
{
    switch (squareIndex) {
    case 0: return m_gridTL;
    case 1: return m_gridTR;
    case 2: return m_gridBL;
    case 3: return m_gridBR;
    default: return m_gridTL;
    }
}

/* ---- Reset ---- */

void FoursquareCode8::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
