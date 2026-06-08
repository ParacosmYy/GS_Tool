/**
 * @file FoursquareCode5.cpp
 * @brief FoursquareCode5 实现
 *
 * 实现四方密码：10×10扩展网格、Polybius坐标压缩、关键字洗牌。
 */

#include "utils/code222/FoursquareCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode5::FoursquareCode5(QObject *parent) : QObject(parent) {}
FoursquareCode5::~FoursquareCode5() = default;

/* ---- Preprocess text ---- */

QString FoursquareCode5::preprocess(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (ch.isLetter() || ch.isDigit())
            result.append(ch);
        else if (ch == ' ')
            result.append('X'); // Pad with X
    }
    // Pad to even length
    if (result.size() % 2 != 0)
        result.append('X');
    return result;
}

/* ---- Keyword shuffle ---- */

QString FoursquareCode5::shuffleAlphabet(const QString& keyword) const
{
    QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString seen;
    // Remove duplicates from keyword, keep order
    for (QChar ch : keyword.toUpper()) {
        if (!seen.contains(ch) && alphabet.contains(ch))
            seen.append(ch);
    }
    // Append remaining alphabet chars
    for (QChar ch : alphabet) {
        if (!seen.contains(ch))
            seen.append(ch);
    }
    return seen;
}

/* ---- Build standard 10x10 grid ---- */

void FoursquareCode5::buildStandardGrid(QVector<QVector<QChar>>& grid) const
{
    grid.resize(10);
    QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    // Fill 10x10 = 100 cells; alphabet is 36 chars, pad with special markers
    for (int r = 0; r < 10; ++r) {
        grid[r].resize(10);
        for (int c = 0; c < 10; ++c) {
            int idx = r * 10 + c;
            if (idx < alphabet.size())
                grid[r][c] = alphabet[idx];
            else
                grid[r][c] = QChar('A' + (idx % 26)); // Wrap for filler
        }
    }
}

/* ---- Build keyword-shuffled 10x10 grid ---- */

void FoursquareCode5::buildKeywordGrid(const QString& keyword,
                                          QVector<QVector<QChar>>& grid) const
{
    QString shuffled = shuffleAlphabet(keyword);
    // Extend to 100 chars for 10x10
    while (shuffled.size() < 100) {
        shuffled.append(QChar('A' + (shuffled.size() % 26)));
    }

    grid.resize(10);
    for (int r = 0; r < 10; ++r) {
        grid[r].resize(10);
        for (int c = 0; c < 10; ++c)
            grid[r][c] = shuffled[r * 10 + c];
    }
}

/* ---- Find character position ---- */

QPair<int, int> FoursquareCode5::findPosition(
    const QVector<QVector<QChar>>& grid, QChar ch) const
{
    ch = ch.toUpper();
    for (int r = 0; r < grid.size(); ++r)
        for (int c = 0; c < grid[r].size(); ++c)
            if (grid[r][c] == ch)
                return qMakePair(r, c);
    return qMakePair(0, 0); // Default fallback
}

/* ---- Get character at position ---- */

QChar FoursquareCode5::charAt(const QVector<QVector<QChar>>& grid,
                                int row, int col) const
{
    if (row >= 0 && row < grid.size() && col >= 0 && col < grid[row].size())
        return grid[row][col];
    return 'X';
}

/* ---- Set keyword ---- */

void FoursquareCode5::setKeyword(const QString& keyword1, const QString& keyword2)
{
    m_keyword1 = keyword1;
    m_keyword2 = keyword2;
    m_stats.keywordLength = qMax(keyword1.size(), keyword2.size());

    buildStandardGrid(m_gridTL);
    buildKeywordGrid(keyword1, m_gridTR);
    buildKeywordGrid(keyword2, m_gridBL);
    buildStandardGrid(m_gridBR);
}

/* ---- Encrypt ---- */

QString FoursquareCode5::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = preprocess(plaintext);
    QString result;

    for (int i = 0; i + 1 < prepared.size(); i += 2) {
        // First char: find in TL grid
        auto pos1 = findPosition(m_gridTL, prepared[i]);
        // Second char: find in BR grid
        auto pos2 = findPosition(m_gridBR, prepared[i + 1]);

        // Foursquare rule: same row/col swap
        // Encrypted pair: TR[pos1.row][pos2.col], BL[pos2.row][pos1.col]
        QChar enc1 = charAt(m_gridTR, pos1.first, pos2.second);
        QChar enc2 = charAt(m_gridBL, pos2.first, pos1.second);
        result.append(enc1);
        result.append(enc2);
    }

    const_cast<FoursquareCode5*>(this)->m_stats.inputLength = plaintext.size();
    const_cast<FoursquareCode5*>(this)->m_stats.outputLength = result.size();
    const_cast<FoursquareCode5*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode5*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<FoursquareCode5*>(this)->cipherCompleted(
        "encrypt", plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode5::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = preprocess(ciphertext);
    QString result;

    for (int i = 0; i + 1 < prepared.size(); i += 2) {
        // First encrypted char: find in TR grid
        auto pos1 = findPosition(m_gridTR, prepared[i]);
        // Second encrypted char: find in BL grid
        auto pos2 = findPosition(m_gridBL, prepared[i + 1]);

        // Reverse foursquare: TL[pos1.row][pos2.col], BR[pos2.row][pos1.col]
        QChar dec1 = charAt(m_gridTL, pos1.first, pos2.second);
        QChar dec2 = charAt(m_gridBR, pos2.first, pos1.second);
        result.append(dec1);
        result.append(dec2);
    }

    const_cast<FoursquareCode5*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode5*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<FoursquareCode5*>(this)->cipherCompleted(
        "decrypt", ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Generate grid ---- */

QVector<QVector<QChar>> FoursquareCode5::generateGrid(const QString& keyword) const
{
    QVector<QVector<QChar>> grid;
    buildKeywordGrid(keyword, grid);
    return grid;
}

/* ---- Polybius coordinate compression ---- */

QByteArray FoursquareCode5::compressCoordinates(
    const QVector<QPair<int, int>>& coords) const
{
    QByteArray result;
    for (const auto& p : coords) {
        // Pack two 0-9 values into one byte: high nibble = row, low nibble = col
        char packed = static_cast<char>((p.first << 4) | p.second);
        result.append(packed);
    }
    return result;
}

/* ---- Decompress coordinates ---- */

QVector<QPair<int, int>> FoursquareCode5::decompressCoordinates(
    const QByteArray& compressed) const
{
    QVector<QPair<int, int>> coords;
    for (int i = 0; i < compressed.size(); ++i) {
        unsigned char packed = static_cast<unsigned char>(compressed[i]);
        int row = (packed >> 4) & 0x0F;
        int col = packed & 0x0F;
        coords.append(qMakePair(row, col));
    }
    return coords;
}

/* ---- Reset ---- */

void FoursquareCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_keyword1.clear();
    m_keyword2.clear();
    m_gridTL.clear();
    m_gridTR.clear();
    m_gridBL.clear();
    m_gridBR.clear();
}
