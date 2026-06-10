/**
 * @file FoursquareCode11.cpp
 * @brief FoursquareCode11 实现
 *
 * 实现四方密码：Polybius坐标交织与对角转置增强空间编码。
 */

#include "utils/code264/FoursquareCode11.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode11::FoursquareCode11(QObject *parent)
    : QObject(parent)
{
    buildStandardSquare(m_squareTL);
    buildStandardSquare(m_squareTR);
    buildStandardSquare(m_squareBL);
    buildStandardSquare(m_squareBR);
}

FoursquareCode11::~FoursquareCode11() = default;

/* ---- Character mapping ---- */

int FoursquareCode11::charToIndex(QChar c)
{
    char ch = c.toUpper().toLatin1();
    if (ch == 'J') ch = 'I';
    if (ch >= 'A' && ch <= 'Z') {
        int idx = ch - 'A';
        if (idx > 8) idx--;  // Skip J
        return idx;
    }
    return -1;
}

QChar FoursquareCode11::indexToChar(int idx)
{
    if (idx < 0 || idx >= 25) return 'X';
    if (idx >= 9) idx++;  // Account for J skip
    return QChar('A' + idx);
}

/* ---- Build standard square ---- */

void FoursquareCode11::buildStandardSquare(Square& sq) const
{
    for (int i = 0; i < 25; ++i) {
        int r = i / 5, c = i % 5;
        sq.grid[r][c] = i;
        sq.pos[i][0] = r;
        sq.pos[i][1] = c;
    }
}

/* ---- Build keyed square ---- */

void FoursquareCode11::buildSquare(Square& sq, const QString& key) const
{
    QVector<bool> used(25, false);
    int idx = 0;

    // Place key characters first
    for (QChar c : key.toUpper()) {
        int ci = charToIndex(c);
        if (ci < 0 || used[ci]) continue;
        used[ci] = true;
        int r = idx / 5, col = idx % 5;
        sq.grid[r][col] = ci;
        sq.pos[ci][0] = r;
        sq.pos[ci][1] = col;
        idx++;
    }

    // Fill remaining alphabet
    for (int i = 0; i < 25; ++i) {
        if (used[i]) continue;
        int r = idx / 5, col = idx % 5;
        sq.grid[r][col] = i;
        sq.pos[i][0] = r;
        sq.pos[i][1] = col;
        idx++;
    }
}

/* ---- Prepare text ---- */

QString FoursquareCode11::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c.isLetter()) {
            char ch = c.toLatin1();
            if (ch == 'J') ch = 'I';
            result += QChar(ch);
        }
    }
    // Pad to even length
    if (result.size() % 2 != 0)
        result += 'X';
    return result;
}

/* ---- Diagonal transposition ---- */

QVector<QPair<int,int>> FoursquareCode11::diagonalTranspose(
    const QVector<QPair<int,int>>& coords) const
{
    int n = coords.size();
    QVector<QPair<int,int>> result(n);
    for (int i = 0; i < n; ++i) {
        int r = coords[i].first;
        int c = coords[i].second;
        // Diagonal shift: (r,c) -> ((r+c)%5, (r+2*c)%5)
        result[i] = {(r + c) % 5, (r + 2 * c) % 5};
    }
    return result;
}

QVector<QPair<int,int>> FoursquareCode11::inverseDiagonalTranspose(
    const QVector<QPair<int,int>>& coords) const
{
    int n = coords.size();
    QVector<QPair<int,int>> result(n);
    for (int i = 0; i < n; ++i) {
        int r = coords[i].first;
        int c = coords[i].second;
        // Inverse of (r+c, r+2c) mod 5: solve for original (r,c)
        // new_r = (r+c)%5, new_c = (r+2c)%5
        // c = (new_c - new_r + 10) % 5, r = (new_r - c + 5) % 5
        int origC = (c - r + 10) % 5;
        int origR = (r - origC + 10) % 5;
        result[i] = {origR, origC};
    }
    return result;
}

/* ---- Set keys ---- */

void FoursquareCode11::setKeys(const QString& key1, const QString& key2)
{
    buildSquare(m_squareTR, key1);
    buildSquare(m_squareBL, key2);
    // TL and BR remain standard
    buildStandardSquare(m_squareTL);
    buildStandardSquare(m_squareBR);
}

/* ---- Encrypt ---- */

QString FoursquareCode11::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(plaintext);
    QString result;
    int pairs = prepared.size() / 2;

    for (int p = 0; p < pairs; ++p) {
        int a = charToIndex(prepared[p * 2]);
        int b = charToIndex(prepared[p * 2 + 1]);
        if (a < 0) a = 9;  // Fallback to 'I'
        if (b < 0) b = 9;

        // Get coordinates from TL and BR squares
        int r1 = m_squareTL.pos[a][0], c1 = m_squareTL.pos[a][1];
        int r2 = m_squareBR.pos[b][0], c2 = m_squareBR.pos[b][1];

        // Apply diagonal transposition
        auto transCoords = diagonalTranspose({{r1, c1}, {r2, c2}});
        int tr1 = transCoords[0].first, tc1 = transCoords[0].second;
        int tr2 = transCoords[1].first, tc2 = transCoords[1].second;

        // Foursquare rule: read from TR (row from TL, col from BR) and BL
        int enc1 = m_squareTR.grid[tr1][tc2];
        int enc2 = m_squareBL.grid[tr2][tc1];
        result += indexToChar(enc1);
        result += indexToChar(enc2);
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherComputed(plaintext.size(), result.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode11::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(ciphertext);
    QString result;
    int pairs = prepared.size() / 2;

    for (int p = 0; p < pairs; ++p) {
        int a = charToIndex(prepared[p * 2]);
        int b = charToIndex(prepared[p * 2 + 1]);
        if (a < 0) a = 9;
        if (b < 0) b = 9;

        // Find position in TR and BL squares
        int r1 = m_squareTR.pos[a][0], c1 = m_squareTR.pos[a][1];
        int r2 = m_squareBL.pos[b][0], c2 = m_squareBL.pos[b][1];

        // Inverse diagonal transposition
        auto origCoords = inverseDiagonalTranspose({{r1, c2}, {r2, c1}});
        int or1 = origCoords[0].first, oc1 = origCoords[0].second;
        int or2 = origCoords[1].first, oc2 = origCoords[1].second;

        // Read from TL and BR
        int dec1 = m_squareTL.grid[or1][oc1];
        int dec2 = m_squareBR.grid[or2][oc2];
        result += indexToChar(dec1);
        result += indexToChar(dec2);
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherComputed(ciphertext.size(), result.size(), elapsed);
    return result;
}

/* ---- Get squares ---- */

QVector<QVector<int>> FoursquareCode11::squares() const
{
    QVector<QVector<int>> result;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            result.append({m_squareTL.grid[r][c], m_squareTR.grid[r][c],
                           m_squareBL.grid[r][c], m_squareBR.grid[r][c]});
    return result;
}

/* ---- Reset ---- */

void FoursquareCode11::resetStatistics()
{
    buildStandardSquare(m_squareTL);
    buildStandardSquare(m_squareTR);
    buildStandardSquare(m_squareBL);
    buildStandardSquare(m_squareBR);
    m_stats = Stats{};
    m_timeSum = 0.0;
}
