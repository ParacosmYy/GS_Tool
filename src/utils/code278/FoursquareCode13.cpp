/**
 * @file FoursquareCode13.cpp
 * @brief FoursquareCode13 实现
 *
 * 实现四方密码：扩展22x22网格与三元组替换的空间多图编码。
 */

#include "utils/code278/FoursquareCode13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode13::FoursquareCode13(QObject *parent)
    : QObject(parent)
{
    // Build 22x22 extended alphabet: A-Z, 0-9, and special chars
    m_alphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,;:!?-'()");
    m_config.gridSize = 22;
    m_config.key1 = QStringLiteral("KEYWORD");
    m_config.key2 = QStringLiteral("SECRET");
    m_config.trigramMode = true;
    precomputeGrids();
}

FoursquareCode13::~FoursquareCode13() = default;

/* ---- Configuration ---- */

void FoursquareCode13::setConfig(const Config& config)
{
    m_config = config;
    precomputeGrids();
}

/* ---- Character <-> index mapping ---- */

int FoursquareCode13::charToIndex(QChar c) const
{
    c = c.toUpper();
    int idx = m_alphabet.indexOf(c);
    return (idx >= 0) ? idx : 0;
}

QChar FoursquareCode13::indexToChar(int idx) const
{
    int gs = m_config.gridSize;
    idx = ((idx % gs) + gs) % gs;  // Wrap into grid
    int row = idx / gs;
    int col = idx % gs;
    int alphaIdx = row * gs + col;
    if (alphaIdx < m_alphabet.size()) return m_alphabet[alphaIdx];
    return QChar(' ');
}

/* ---- Build grid from key ---- */

QVector<QVector<int>> FoursquareCode13::buildGrid(const QString& key) const
{
    int gs = m_config.gridSize;
    QVector<QVector<int>> grid(gs, QVector<int>(gs, 0));
    QVector<bool> used(gs * gs, false);

    // Fill with key first (deduplicated)
    QString processed;
    for (QChar c : key.toUpper()) {
        int idx = charToIndex(c);
        if (!used[idx]) {
            used[idx] = true;
            processed.append(c);
        }
    }

    // Fill remaining alphabet
    for (int i = 0; i < m_alphabet.size() && i < gs * gs; ++i) {
        if (!used[i]) {
            used[i] = true;
            processed.append(m_alphabet[i]);
        }
    }

    int pos = 0;
    for (int r = 0; r < gs; ++r) {
        for (int c = 0; c < gs; ++c) {
            grid[r][c] = (pos < processed.size()) ? charToIndex(processed[pos]) : pos;
            ++pos;
        }
    }
    return grid;
}

/* ---- Precompute all four grids ---- */

void FoursquareCode13::precomputeGrids()
{
    int gs = m_config.gridSize;
    // Standard alphabet grids
    QString alphaKey;
    for (int i = 0; i < gs * gs && i < m_alphabet.size(); ++i)
        alphaKey.append(m_alphabet[i]);

    m_gridTL = buildGrid(alphaKey);
    m_gridBR = buildGrid(alphaKey);
    m_gridTR = buildGrid(m_config.key1);
    m_gridBL = buildGrid(m_config.key2);
}

/* ---- Find value in grid ---- */

void FoursquareCode13::findInGrid(const QVector<QVector<int>>& grid,
                                    int val, int& row, int& col) const
{
    int gs = m_config.gridSize;
    for (int r = 0; r < gs; ++r) {
        for (int c = 0; c < gs; ++c) {
            if (grid[r][c] == val) { row = r; col = c; return; }
        }
    }
    row = 0; col = 0;
}

/* ---- Encode bigram ---- */

QPair<QChar, QChar> FoursquareCode13::encodeBigram(QChar a, QChar b) const
{
    int r1, c1, r2, c2;
    findInGrid(m_gridTL, charToIndex(a), r1, c1);
    findInGrid(m_gridBR, charToIndex(b), r2, c2);

    // Foursquare rule: TL(r1,c2) and BR(r2,c1)
    int enc1 = m_gridTR[r1][c2];
    int enc2 = m_gridBL[r2][c1];
    return qMakePair(indexToChar(enc1), indexToChar(enc2));
}

/* ---- Decode bigram ---- */

QPair<QChar, QChar> FoursquareCode13::decodeBigram(QChar a, QChar b) const
{
    int r1, c1, r2, c2;
    findInGrid(m_gridTR, charToIndex(a), r1, c1);
    findInGrid(m_gridBL, charToIndex(b), r2, c2);

    int dec1 = m_gridTL[r1][c2];
    int dec2 = m_gridBR[r2][c1];
    return qMakePair(indexToChar(dec1), indexToChar(dec2));
}

/* ---- Trigram encode/decode ---- */

QString FoursquareCode13::encodeTrigram(const QString& trig) const
{
    if (trig.size() < 3) return trig;
    // Extended: encode first two chars as bigram, then shift third by grid position
    auto pair = encodeBigram(trig[0], trig[1]);
    int r, c;
    findInGrid(m_gridTL, charToIndex(trig[2]), r, c);
    int shifted = (r + c) % m_config.gridSize;
    QString result;
    result.append(pair.first);
    result.append(pair.second);
    result.append(indexToChar(shifted));
    return result;
}

QString FoursquareCode13::decodeTrigram(const QString& trig) const
{
    if (trig.size() < 3) return trig;
    auto pair = decodeBigram(trig[0], trig[1]);
    int shifted = charToIndex(trig[2]);
    int r = shifted;
    int c = 0;
    // Reverse the shift
    for (int cc = 0; cc < m_config.gridSize; ++cc) {
        if ((r + cc) % m_config.gridSize == shifted) { c = cc; break; }
    }
    QString result;
    result.append(pair.first);
    result.append(pair.second);
    result.append(indexToChar(r * m_config.gridSize + c));
    return result;
}

/* ---- Prepare text ---- */

QString FoursquareCode13::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (m_alphabet.contains(c)) result.append(c);
    }
    // Pad to even length for bigram mode
    if (result.size() % 2 != 0) result.append('X');
    return result;
}

/* ---- Encrypt ---- */

FoursquareCode13::CipherResult FoursquareCode13::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString prepared = prepareText(plaintext);
    QString output;
    int blocks = 0;

    if (m_config.trigramMode) {
        // Pad to multiple of 3
        while (prepared.size() % 3 != 0) prepared.append('X');
        for (int i = 0; i < prepared.size() - 2; i += 3) {
            output += encodeTrigram(prepared.mid(i, 3));
            ++blocks;
        }
    } else {
        for (int i = 0; i < prepared.size() - 1; i += 2) {
            auto pair = encodeBigram(prepared[i], prepared[i + 1]);
            output.append(pair.first);
            output.append(pair.second);
            ++blocks;
        }
    }

    result.output = output;
    result.inputLength = plaintext.size();
    result.outputLength = output.size();
    result.numBlocks = blocks;
    result.processingTimeMs = timer.elapsed();

    m_stats.totalOps++;
    m_timeSum += result.processingTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherDone(QStringLiteral("encrypt"), blocks, result.processingTimeMs);

    return result;
}

/* ---- Decrypt ---- */

FoursquareCode13::CipherResult FoursquareCode13::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString output;
    int blocks = 0;

    if (m_config.trigramMode) {
        for (int i = 0; i < ciphertext.size() - 2; i += 3) {
            output += decodeTrigram(ciphertext.mid(i, 3));
            ++blocks;
        }
    } else {
        for (int i = 0; i < ciphertext.size() - 1; i += 2) {
            auto pair = decodeBigram(ciphertext[i], ciphertext[i + 1]);
            output.append(pair.first);
            output.append(pair.second);
            ++blocks;
        }
    }

    result.output = output;
    result.inputLength = ciphertext.size();
    result.outputLength = output.size();
    result.numBlocks = blocks;
    result.processingTimeMs = timer.elapsed();

    m_stats.totalOps++;
    m_timeSum += result.processingTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherDone(QStringLiteral("decrypt"), blocks, result.processingTimeMs);

    return result;
}

/* ---- Reset ---- */

void FoursquareCode13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
