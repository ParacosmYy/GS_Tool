/**
 * @file FoursquareCode4.cpp
 * @brief FoursquareCode4 实现
 *
 * 实现四方密码：8x8 Polybius网格构建、坐标转置、关键字派生轴。
 */

#include "utils/code217/FoursquareCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode4::FoursquareCode4(QObject *parent) : QObject(parent)
{
    setKeywords("EMBED", "DEBUG");
}

FoursquareCode4::~FoursquareCode4() = default;

/* ---- Build 8x8 Polybius grid from keyword ---- */

QVector<QVector<int>> FoursquareCode4::buildGrid(const QString& keyword) const
{
    QVector<QVector<int>> grid(8, QVector<int>(8, 0));
    QVector<bool> used(64, false);

    // Keyword-derived axis: determine fill order permutation
    QVector<int> axisOrder(8);
    for (int i = 0; i < 8; ++i) axisOrder[i] = i;
    // Sort by keyword character positions cyclically
    if (!keyword.isEmpty()) {
        QString upper = keyword.toUpper();
        for (int i = 0; i < 8; ++i) {
            int shift = (i < upper.size()) ? (upper[i].unicode() % 8) : 0;
            std::swap(axisOrder[i], axisOrder[(i + shift) % 8]);
        }
    }

    int val = 0;
    // Place keyword chars first
    QString upper = keyword.toUpper();
    for (int row : axisOrder) {
        for (int col = 0; col < 8; ++col) {
            if (val >= 64) break;
            grid[row][col] = val;
            val++;
        }
    }
    return grid;
}

/* ---- Set keywords ---- */

void FoursquareCode4::setKeywords(const QString& keyword1,
                                   const QString& keyword2)
{
    m_keyword1 = keyword1;
    m_keyword2 = keyword2;

    // Grid TL and BR are standard (no keyword)
    QVector<QVector<int>> standard(8, QVector<int>(8));
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            standard[r][c] = r * 8 + c;

    m_gridTL = standard;
    m_gridBR = standard;
    m_gridTR = buildGrid(keyword1);
    m_gridBL = buildGrid(keyword2);
}

/* ---- Find position in grid ---- */

QPair<int, int> FoursquareCode4::findInGrid(
    const QVector<QVector<int>>& grid, int val) const
{
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (grid[r][c] == val) return {r, c};
    return {0, 0};
}

/* ---- Encode bigram via foursquare rules ---- */

QPair<int, int> FoursquareCode4::encodeBigram(
    int r1, int c1, int r2, int c2) const
{
    // Foursquare rule: TL(row1,col2) and BR(row2,col1)
    int outVal1 = m_gridTR[r1][c2];
    int outVal2 = m_gridBL[r2][c1];
    return {outVal1, outVal2};
}

/* ---- Decode bigram via foursquare rules ---- */

QPair<int, int> FoursquareCode4::decodeBigram(
    int r1, int c1, int r2, int c2) const
{
    // Inverse: TR(r1,c2)->TL, BL(r2,c1)->BR
    int outVal1 = m_gridTL[r1][c2];
    int outVal2 = m_gridBR[r2][c1];
    return {outVal1, outVal2};
}

/* ---- Encrypt ---- */

QString FoursquareCode4::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Convert to code values (mod 64)
    QVector<int> codes;
    for (auto ch : plaintext.toUpper()) {
        codes.append(ch.unicode() % 64);
    }
    // Pad to even length
    if (codes.size() % 2 != 0) codes.append(0);

    QString result;
    for (int i = 0; i < codes.size(); i += 2) {
        auto [r1, c1] = findInGrid(m_gridTL, codes[i]);
        auto [r2, c2] = findInGrid(m_gridBR, codes[i + 1]);
        auto [v1, v2] = encodeBigram(r1, c1, r2, c2);
        result.append(QChar(v1 + 32));
        result.append(QChar(v2 + 32));
    }

    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("encrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode4::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codes;
    for (auto ch : ciphertext.toUpper()) {
        codes.append(ch.unicode() % 64);
    }

    QString result;
    for (int i = 0; i + 1 < codes.size(); i += 2) {
        auto [r1, c1] = findInGrid(m_gridTR, codes[i]);
        auto [r2, c2] = findInGrid(m_gridBL, codes[i + 1]);
        auto [v1, v2] = decodeBigram(r1, c1, r2, c2);
        result.append(QChar(v1 + 32));
        result.append(QChar(v2 + 32));
    }

    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Grid state ---- */

QVector<QVector<int>> FoursquareCode4::gridState() const { return m_gridTL; }

/* ---- Reset ---- */

void FoursquareCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
