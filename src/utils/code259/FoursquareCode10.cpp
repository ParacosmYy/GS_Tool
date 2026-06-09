/**
 * @file FoursquareCode10.cpp
 * @brief FoursquareCode10 实现
 *
 * 实现四方密码：18x18扩展网格三关键字多层替换编码。
 */

#include "utils/code259/FoursquareCode10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode10::FoursquareCode10(QObject *parent)
    : QObject(parent)
{
    m_grids.resize(4);
    // Build default grids
    m_grids[0] = buildPlainGrid();
    m_grids[1] = buildGrid(m_kw1);
    m_grids[2] = buildGrid(m_kw2);
    m_grids[3] = buildGrid(m_kw3);
}
FoursquareCode10::~FoursquareCode10() = default;

/* ---- Extended alphabet ---- */

QVector<QChar> FoursquareCode10::buildExtendedAlphabet() const
{
    // 18x18 = 324 chars: A-Z (26), a-z (26), 0-9 (10), common punctuation + CJK supplement
    QVector<QChar> alpha;
    // Uppercase A-Z
    for (ushort c = 'A'; c <= 'Z'; ++c) alpha.append(QChar(c));
    // Lowercase a-z
    for (ushort c = 'a'; c <= 'z'; ++c) alpha.append(QChar(c));
    // Digits 0-9
    for (ushort c = '0'; c <= '9'; ++c) alpha.append(QChar(c));
    // Punctuation and special chars to fill 324
    const QString extra = QStringLiteral(" !?.,;:'-\"()[]{}/<>@#$%^&*+=|\\~_");
    for (QChar ch : extra) alpha.append(ch);
    // Fill remaining with Unicode block
    ushort fill = 0x4E00; // CJK Unified Ideographs start
    while (alpha.size() < ALPHABET_SIZE) {
        alpha.append(QChar(fill++));
    }
    return alpha;
}

/* ---- Build plain grid ---- */

QVector<QChar> FoursquareCode10::buildPlainGrid()
{
    return buildExtendedAlphabet();
}

/* ---- Build keyword grid ---- */

QVector<QChar> FoursquareCode10::buildGrid(const QString& keyword)
{
    QVector<QChar> alpha = buildExtendedAlphabet();
    QVector<QChar> grid;
    grid.reserve(ALPHABET_SIZE);
    // Track used characters
    QVector<bool> used(65536, false);
    // Add keyword chars first (unique)
    for (QChar ch : keyword) {
        if (!used[ch.unicode()]) {
            grid.append(ch);
            used[ch.unicode()] = true;
        }
    }
    // Fill remaining from alphabet
    for (QChar ch : alpha) {
        if (!used[ch.unicode()]) {
            grid.append(ch);
            used[ch.unicode()] = true;
        }
        if (grid.size() >= ALPHABET_SIZE) break;
    }
    // Pad if needed
    while (grid.size() < ALPHABET_SIZE)
        grid.append(QChar('?'));
    return grid;
}

/* ---- Set keywords ---- */

void FoursquareCode10::setKeywords(const QString& kw1, const QString& kw2, const QString& kw3)
{
    m_kw1 = kw1;
    m_kw2 = kw2;
    m_kw3 = kw3;
    m_grids[0] = buildPlainGrid();
    m_grids[1] = buildGrid(m_kw1);
    m_grids[2] = buildGrid(m_kw2);
    m_grids[3] = buildGrid(m_kw3);
}

/* ---- Find in grid ---- */

bool FoursquareCode10::findInGrid(const QVector<QChar>& grid, QChar ch, int& row, int& col) const
{
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] == ch) {
            row = i / GRID_SIZE;
            col = i % GRID_SIZE;
            return true;
        }
    }
    return false;
}

/* ---- Prepare text ---- */

QString FoursquareCode10::prepareText(const QString& text) const
{
    QString result;
    for (QChar ch : text) {
        // Check if char exists in any grid
        int r, c;
        if (findInGrid(m_grids[0], ch, r, c))
            result.append(ch);
    }
    // Pad to even length
    if (result.size() % 2 != 0)
        result.append(QChar(' '));
    return result;
}

/* ---- Process digraph ---- */

QPair<QChar, QChar> FoursquareCode10::processDigraph(QChar a, QChar b, bool encrypt) const
{
    // In foursquare: TL=plain, TR=kw1, BL=kw2, BR=kw3
    // Encrypt: a from TL, b from BR
    int rowA, colA, rowB, colB;
    if (!findInGrid(m_grids[0], a, rowA, colA)) return {a, b};
    if (!findInGrid(m_grids[0], b, rowB, colB)) return {a, b};

    QChar outA, outB;
    if (encrypt) {
        // a: same row as in TL, col from BR
        int idxA = rowA * GRID_SIZE + colB;
        outA = (idxA < m_grids[3].size()) ? m_grids[3][idxA] : a;
        // b: same row as in BR, col from TL
        int idxB = rowB * GRID_SIZE + colA;
        outB = (idxB < m_grids[3].size()) ? m_grids[3][idxB] : b;
    } else {
        // Decrypt: reverse mapping
        int idxA = rowA * GRID_SIZE + colB;
        outA = (idxA < m_grids[0].size()) ? m_grids[0][idxA] : a;
        int idxB = rowB * GRID_SIZE + colA;
        outB = (idxB < m_grids[0].size()) ? m_grids[0][idxB] : b;
    }
    return {outA, outB};
}

/* ---- Encrypt ---- */

QString FoursquareCode10::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(plaintext);
    QString result;
    for (int i = 0; i + 1 < prepared.size(); i += 2) {
        auto pair = processDigraph(prepared[i], prepared[i + 1], true);
        result.append(pair.first);
        result.append(pair.second);
    }

    double elapsed = timer.elapsed();
    m_stats.numEncryptions++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptionCompleted(plaintext.size(), result.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode10::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (int i = 0; i + 1 < ciphertext.size(); i += 2) {
        auto pair = processDigraph(ciphertext[i], ciphertext[i + 1], false);
        result.append(pair.first);
        result.append(pair.second);
    }

    double elapsed = timer.elapsed();
    m_stats.numDecryptions++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptionCompleted(ciphertext.size(), result.size(), elapsed);
    return result;
}

/* ---- Accessors ---- */

QVector<QChar> FoursquareCode10::grid(int index) const
{
    if (index < 0 || index >= m_grids.size()) return {};
    return m_grids[index];
}

/* ---- Reset ---- */

void FoursquareCode10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
