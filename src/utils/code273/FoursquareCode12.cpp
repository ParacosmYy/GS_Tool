/**
 * @file FoursquareCode12.cpp
 * @brief FoursquareCode12 实现
 *
 * 实现四方密码：扩展20x20网格与自动密钥驱动渐进替换增强多表编码。
 */

#include "utils/code273/FoursquareCode12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode12::FoursquareCode12(QObject *parent)
    : QObject(parent)
{
    m_grids.resize(4);
    m_reverseGrids.resize(4);
    for (int i = 0; i < 4; ++i) {
        m_grids[i].resize(20, QVector<int>(20, 0));
        m_reverseGrids[i].resize(400, 0);
    }
    buildGrids();
}

FoursquareCode12::~FoursquareCode12() = default;

/* ---- Configuration ---- */

void FoursquareCode12::setKey1(const QString& key) { m_key1 = key.toUpper(); buildGrids(); }
void FoursquareCode12::setKey2(const QString& key) { m_key2 = key.toUpper(); buildGrids(); }
void FoursquareCode12::setAutokeySeed(const QString& seed) { m_autokeySeed = seed.toUpper(); }

/* ---- Fill a single grid with keyword-driven mixed alphabet ---- */

void FoursquareCode12::fillGrid(int gridIdx, const QString& key)
{
    QVector<bool> used(400, false);
    int pos = 0;

    // Place keyword characters first (no duplicates)
    for (const QChar& ch : key) {
        int val = ch.unicode() % 400;
        if (!used[val]) {
            m_grids[gridIdx][pos / 20][pos % 20] = val;
            used[val] = true;
            pos++;
        }
    }

    // Fill remaining cells with unused values in order
    for (int v = 0; v < 400 && pos < 400; ++v) {
        if (!used[v]) {
            m_grids[gridIdx][pos / 20][pos % 20] = v;
            pos++;
        }
    }

    // Build reverse lookup
    for (int r = 0; r < 20; ++r)
        for (int c = 0; c < 20; ++c)
            m_reverseGrids[gridIdx][m_grids[gridIdx][r][c]] = r * 20 + c;
}

/* ---- Build all four squares ---- */

void FoursquareCode12::buildGrids()
{
    // Square 0 (top-left): standard plain square
    for (int i = 0; i < 400; ++i)
        m_grids[0][i / 20][i % 20] = i;
    for (int r = 0; r < 20; ++r)
        for (int c = 0; c < 20; ++c)
            m_reverseGrids[0][m_grids[0][r][c]] = r * 20 + c;

    // Square 1 (top-right): key1-driven
    fillGrid(1, m_key1.isEmpty() ? QStringLiteral("FOURSQUARE") : m_key1);

    // Square 2 (bottom-left): key2-driven
    fillGrid(2, m_key2.isEmpty() ? QStringLiteral("CIPHER") : m_key2);

    // Square 3 (bottom-right): standard plain square (same as 0)
    for (int i = 0; i < 400; ++i)
        m_grids[3][i / 20][i % 20] = i;
    for (int r = 0; r < 20; ++r)
        for (int c = 0; c < 20; ++c)
            m_reverseGrids[3][m_grids[3][r][c]] = r * 20 + c;
}

/* ---- Coordinate mapping helpers ---- */

void FoursquareCode12::charToCoords(int gridIdx, int ch, int& row, int& col) const
{
    int pos = m_reverseGrids[gridIdx][ch % 400];
    row = pos / 20;
    col = pos % 20;
}

int FoursquareCode12::coordsToChar(int gridIdx, int row, int col) const
{
    row = ((row % 20) + 20) % 20;
    col = ((col % 20) + 20) % 20;
    return m_grids[gridIdx][row][col];
}

/* ---- Autokey generation ---- */

QString FoursquareCode12::generateAutokey(const QString& input, const QString& seed) const
{
    QString autokey = seed;
    // Autokey: append plaintext characters to extend the key
    for (int i = 0; i < input.size(); ++i) {
        if (autokey.size() >= input.size()) break;
        autokey.append(input[i]);
    }
    return autokey;
}

/* ---- Progressive autokey shift ---- */

int FoursquareCode12::applyAutokeyShift(int ch, int shift, bool forward) const
{
    if (forward)
        return (ch + shift) % 400;
    else
        return ((ch - shift) % 400 + 400) % 400;
}

/* ---- Encode ---- */

QString FoursquareCode12::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString text = plaintext.toUpper();
    // Ensure even length by padding
    if (text.size() % 2 != 0) text.append(QLatin1Char('X'));

    QString autokey = generateAutokey(text, m_autokeySeed.isEmpty()
        ? QStringLiteral("AUTOKEY") : m_autokeySeed);

    QString result;
    for (int i = 0; i < text.size(); i += 2) {
        int a = text[i].unicode() % 400;
        int b = text[i + 1].unicode() % 400;

        // Progressive autokey shift
        int shift = (i / 2 < autokey.size()) ? (autokey[i / 2].unicode() % 20) : 0;
        a = applyAutokeyShift(a, shift, true);
        b = applyAutokeyShift(b, shift, true);

        // Get coordinates in plain squares
        int r1, c1, r2, c2;
        charToCoords(0, a, r1, c1);  // Top-left plain
        charToCoords(3, b, r2, c2);  // Bottom-right plain

        // Foursquare digraph substitution: swap columns
        int ch1 = coordsToChar(1, r1, c2);  // Top-right keyed
        int ch2 = coordsToChar(2, r2, c1);  // Bottom-left keyed

        result.append(QChar(ch1 < 256 ? ch1 : (ch1 % 26 + 65)));
        result.append(QChar(ch2 < 256 ? ch2 : (ch2 % 26 + 65)));
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = text.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodingDone(text.size(), result.size(), elapsed);

    return result;
}

/* ---- Decode ---- */

QString FoursquareCode12::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString text = ciphertext.toUpper();
    QString autokey = generateAutokey(text, m_autokeySeed.isEmpty()
        ? QStringLiteral("AUTOKEY") : m_autokeySeed);

    QString result;
    for (int i = 0; i < text.size(); i += 2) {
        int ch1 = text[i].unicode() % 400;
        int ch2 = text[i + 1].unicode() % 400;

        // Get coordinates in keyed squares
        int r1, c1, r2, c2;
        charToCoords(1, ch1, r1, c1);  // Top-right keyed
        charToCoords(2, ch2, r2, c2);  // Bottom-left keyed

        // Reverse foursquare: swap columns back
        int a = coordsToChar(0, r1, c2);  // Top-left plain
        int b = coordsToChar(3, r2, c1);  // Bottom-right plain

        // Reverse progressive autokey shift
        int shift = (i / 2 < autokey.size()) ? (autokey[i / 2].unicode() % 20) : 0;
        a = applyAutokeyShift(a, shift, false);
        b = applyAutokeyShift(b, shift, false);

        result.append(QChar(a < 256 ? a : (a % 26 + 65)));
        result.append(QChar(b < 256 ? b : (b % 26 + 65)));
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = text.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Get grid state ---- */

QVector<QVector<int>> FoursquareCode12::grid(int squareIndex) const
{
    if (squareIndex < 0 || squareIndex >= 4) return {};
    return m_grids[squareIndex];
}

/* ---- Reset ---- */

void FoursquareCode12::resetStatistics()
{
    m_key1.clear();
    m_key2.clear();
    m_autokeySeed.clear();
    buildGrids();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
