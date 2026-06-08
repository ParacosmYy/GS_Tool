/**
 * @file SeriatedPlayfair4.cpp
 * @brief SeriatedPlayfair4 实现
 *
 * 实现序列化Playfair密码：关键词驱动Polybius网格与扩展二字母组替换。
 */

#include "utils/code234/SeriatedPlayfair4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair4::SeriatedPlayfair4(QObject *parent) : QObject(parent) {}
SeriatedPlayfair4::~SeriatedPlayfair4() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair4::setKeyword(const QString& keyword) { m_keyword = keyword.toUpper(); }
void SeriatedPlayfair4::setFillerChar(QChar ch) { m_fillerChar = ch.toUpper(); }
void SeriatedPlayfair4::setGridSize(int size) { m_gridSize = qBound(5, size, 6); }

/* ---- Generate alphabet ---- */

QVector<QChar> SeriatedPlayfair4::generateAlphabet() const
{
    QVector<QChar> alpha;
    if (m_gridSize == 5) {
        // A-Z without J (I/J merged)
        for (ushort c = 'A'; c <= 'Z'; ++c) {
            if (c == 'J') continue;
            alpha.append(QChar(c));
        }
    } else {
        // A-Z + 0-9 for 6x6
        for (ushort c = 'A'; c <= 'Z'; ++c)
            alpha.append(QChar(c));
        for (ushort c = '0'; c <= '9'; ++c)
            alpha.append(QChar(c));
    }
    return alpha;
}

/* ---- Normalize char (I/J merge) ---- */

QChar SeriatedPlayfair4::normalizeChar(QChar ch) const
{
    ch = ch.toUpper();
    if (m_gridSize == 5 && ch == QLatin1Char('J'))
        return QLatin1Char('I');
    return ch;
}

/* ---- Build Polybius grid ---- */

void SeriatedPlayfair4::buildGrid()
{
    m_alphabet = generateAlphabet();
    m_grid.resize(m_gridSize);
    for (int i = 0; i < m_gridSize; ++i)
        m_grid[i].resize(m_gridSize);

    // Place keyword characters first (deduplicated)
    QVector<QChar> used;
    QString kwNorm = m_keyword.toUpper();
    for (int i = 0; i < kwNorm.size(); ++i) {
        QChar ch = normalizeChar(kwNorm[i]);
        if (!ch.isLetter() && !(m_gridSize == 6 && ch.isDigit())) continue;
        if (!used.contains(ch))
            used.append(ch);
    }

    // Append remaining alphabet characters
    for (QChar ch : m_alphabet) {
        if (!used.contains(ch))
            used.append(ch);
    }

    // Fill grid
    int idx = 0;
    for (int r = 0; r < m_gridSize; ++r) {
        for (int c = 0; c < m_gridSize; ++c) {
            if (idx < used.size())
                m_grid[r][c] = used[idx++];
            else
                m_grid[r][c] = QLatin1Char('A');
        }
    }

    m_stats.gridSize = m_gridSize;
    emit gridBuilt(m_gridSize, m_keyword);
}

/* ---- Find character in grid ---- */

SeriatedPlayfair4::GridCell SeriatedPlayfair4::findChar(QChar ch) const
{
    ch = normalizeChar(ch.toUpper());
    GridCell cell;
    for (int r = 0; r < m_gridSize; ++r) {
        for (int c = 0; c < m_gridSize; ++c) {
            if (m_grid[r][c] == ch) {
                cell.ch = ch;
                cell.row = r;
                cell.col = c;
                return cell;
            }
        }
    }
    return cell;
}

/* ---- Preprocess text into digraphs ---- */

QVector<QPair<QChar, QChar>> SeriatedPlayfair4::preprocessText(const QString& text, bool isEncrypt) const
{
    QVector<QPair<QChar, QChar>> digraphs;
    QString cleaned;

    // Filter valid characters
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = normalizeChar(text[i]);
        if (ch.isLetter() || (m_gridSize == 6 && ch.isDigit()))
            cleaned.append(ch);
    }

    // Split into digraphs with filler optimization
    int i = 0;
    while (i < cleaned.size()) {
        QChar a = cleaned[i];
        QChar b;
        if (i + 1 < cleaned.size()) {
            b = cleaned[i + 1];
            if (isEncrypt && a == b) {
                // Insert filler between repeated chars
                b = m_fillerChar;
                i++;  // Only advance by 1
            } else {
                i += 2;
            }
        } else {
            // Odd length: append filler
            b = m_fillerChar;
            i++;
        }
        digraphs.append({a, b});
    }
    return digraphs;
}

/* ---- Substitute digraph ---- */

QPair<QChar, QChar> SeriatedPlayfair4::substituteDigraph(QChar a, QChar b, bool encrypt) const
{
    GridCell ca = findChar(a);
    GridCell cb = findChar(b);

    int shift = encrypt ? 1 : -1;

    if (ca.row == cb.row) {
        // Same row: shift columns
        int c1 = (ca.col + shift + m_gridSize) % m_gridSize;
        int c2 = (cb.col + shift + m_gridSize) % m_gridSize;
        return {m_grid[ca.row][c1], m_grid[cb.row][c2]};
    }

    if (ca.col == cb.col) {
        // Same column: shift rows
        int r1 = (ca.row + shift + m_gridSize) % m_gridSize;
        int r2 = (cb.row + shift + m_gridSize) % m_gridSize;
        return {m_grid[r1][ca.col], m_grid[r2][cb.col]};
    }

    // Rectangle: swap corners
    return {m_grid[ca.row][cb.col], m_grid[cb.row][ca.col]};
}

/* ---- Encrypt ---- */

SeriatedPlayfair4::CipherResult SeriatedPlayfair4::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_grid.isEmpty()) buildGrid();

    QVector<QPair<QChar, QChar>> digraphs = preprocessText(plaintext, true);
    QString result;
    int fillerCount = 0;

    for (const auto& dg : digraphs) {
        auto sub = substituteDigraph(dg.first, dg.second, true);
        result.append(sub.first);
        result.append(sub.second);
        if (dg.second == m_fillerChar) fillerCount++;
    }

    m_stats.numEncryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    CipherResult cr;
    cr.text = result;
    cr.numDigraphs = digraphs.size();
    cr.numFillers = fillerCount;
    emit encryptCompleted(digraphs.size(), fillerCount, timer.elapsed());
    return cr;
}

/* ---- Decrypt ---- */

SeriatedPlayfair4::CipherResult SeriatedPlayfair4::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_grid.isEmpty()) buildGrid();

    QVector<QPair<QChar, QChar>> digraphs = preprocessText(ciphertext, false);
    QString result;

    for (const auto& dg : digraphs) {
        auto sub = substituteDigraph(dg.first, dg.second, false);
        result.append(sub.first);
        result.append(sub.second);
    }

    m_stats.numDecryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    CipherResult cr;
    cr.text = result;
    cr.numDigraphs = digraphs.size();
    cr.numFillers = 0;
    emit decryptCompleted(digraphs.size(), timer.elapsed());
    return cr;
}

/* ---- Accessors ---- */

QVector<QVector<QChar>> SeriatedPlayfair4::grid() const { return m_grid; }

/* ---- Reset ---- */

void SeriatedPlayfair4::resetStatistics()
{
    m_grid.clear();
    m_alphabet.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
