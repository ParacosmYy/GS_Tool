/**
 * @file SeriatedPlayfair6.cpp
 * @brief SeriatedPlayfair6 实现
 *
 * 实现序列化Playfair密码：Bifid分数化转置嵌入序列化增强有向图扰乱。
 */

#include "utils/code262/SeriatedPlayfair6.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair6::SeriatedPlayfair6(QObject *parent)
    : QObject(parent)
{
    buildSquare("KEYWORD");
}

SeriatedPlayfair6::~SeriatedPlayfair6() = default;

/* ---- Build 5x5 key square ---- */

void SeriatedPlayfair6::buildSquare(const QString& keyword)
{
    QString alpha = "ABCDEFGHIKLMNOPQRSTUVWXYZ"; // No J
    QString used;
    QString key;

    // Add unique keyword letters
    for (QChar c : keyword.toUpper()) {
        if (c == 'J') c = 'I';
        if (alpha.contains(c) && !used.contains(c)) {
            used.append(c);
            key.append(c);
        }
    }
    // Fill remaining alphabet
    for (QChar c : alpha) {
        if (!used.contains(c)) key.append(c);
    }
    m_square = key.left(25);
}

/* ---- Set key ---- */

void SeriatedPlayfair6::setKey(const QString& keyword, int period)
{
    m_period = qMax(2, period);
    buildSquare(keyword);
}

/* ---- Find char position ---- */

QPair<int, int> SeriatedPlayfair6::findChar(QChar c) const
{
    if (c == 'J') c = 'I';
    int idx = m_square.indexOf(c);
    if (idx < 0) return qMakePair(0, 0);
    return qMakePair(idx / 5, idx % 5);
}

/* ---- Encode digraph ---- */

QPair<QChar, QChar> SeriatedPlayfair6::encodeDigraph(QChar a, QChar b) const
{
    auto [r1, c1] = findChar(a);
    auto [r2, c2] = findChar(b);

    if (r1 == r2) {
        // Same row: shift right
        return qMakePair(m_square[r1 * 5 + (c1 + 1) % 5],
                         m_square[r2 * 5 + (c2 + 1) % 5]);
    }
    if (c1 == c2) {
        // Same column: shift down
        return qMakePair(m_square[((r1 + 1) % 5) * 5 + c1],
                         m_square[((r2 + 1) % 5) * 5 + c2]);
    }
    // Rectangle: swap columns
    return qMakePair(m_square[r1 * 5 + c2], m_square[r2 * 5 + c1]);
}

/* ---- Decode digraph ---- */

QPair<QChar, QChar> SeriatedPlayfair6::decodeDigraph(QChar a, QChar b) const
{
    auto [r1, c1] = findChar(a);
    auto [r2, c2] = findChar(b);

    if (r1 == r2) {
        return qMakePair(m_square[r1 * 5 + (c1 + 4) % 5],
                         m_square[r2 * 5 + (c2 + 4) % 5]);
    }
    if (c1 == c2) {
        return qMakePair(m_square[((r1 + 4) % 5) * 5 + c1],
                         m_square[((r2 + 4) % 5) * 5 + c2]);
    }
    return qMakePair(m_square[r1 * 5 + c2], m_square[r2 * 5 + c1]);
}

/* ---- Prepare text ---- */

QString SeriatedPlayfair6::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c >= 'A' && c <= 'Z') {
            if (c == 'J') c = 'I';
            result.append(c);
        }
    }
    // Pad with X if odd length
    if (result.size() % 2 != 0)
        result.append('X');
    return result;
}

/* ---- Bifid fractionation with seriation ---- */

QString SeriatedPlayfair6::applySeriation(const QString& text, bool forEncrypt) const
{
    int len = text.size();
    if (len == 0) return text;

    // Step 1: Extract row/col coordinates (Bifid fractionation)
    QVector<int> rows(len), cols(len);
    for (int i = 0; i < len; ++i) {
        auto [r, c] = findChar(text[i]);
        rows[i] = r;
        cols[i] = c;
    }

    // Step 2: Seriation - interleave within period blocks
    // For period P, group coordinates into blocks of 2*P chars
    QVector<int> seriatedRows(len), seriatedCols(len);
    int p2 = m_period * 2;

    for (int block = 0; block < len; block += p2) {
        int end = qMin(block + p2, len);
        int blockSize = end - block;

        // Collect rows then cols within block (transposition-embedded seriation)
        QVector<int> combined;
        for (int i = block; i < end; ++i) combined.append(rows[i]);
        for (int i = block; i < end; ++i) combined.append(cols[i]);

        // Read back in pairs
        int idx = 0;
        for (int i = block; i < end && idx + 1 < combined.size(); ++i) {
            seriatedRows[i] = combined[idx++];
            seriatedCols[i] = combined[idx++];
        }
    }

    // Step 3: Reconstruct text from new coordinates
    QString result;
    for (int i = 0; i < len; ++i)
        result.append(m_square[seriatedRows[i] * 5 + seriatedCols[i]]);

    return result;
}

/* ---- Encrypt ---- */

QString SeriatedPlayfair6::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(plaintext);

    // Step 1: Standard Playfair digraph encoding
    QString encoded;
    for (int i = 0; i < prepared.size(); i += 2) {
        auto [a, b] = encodeDigraph(prepared[i], prepared[i + 1]);
        encoded.append(a);
        encoded.append(b);
    }

    // Step 2: Apply bifid seriation for enhanced disruption
    QString result = applySeriation(encoded, true);

    double elapsed = timer.elapsed();
    m_stats.keyLength = m_square.size();
    m_stats.period = m_period;
    m_stats.textSize = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherUpdated(result.size(), m_period, elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString SeriatedPlayfair6::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString text = ciphertext.toUpper();
    if (text.size() % 2 != 0) text.chop(1);

    // Step 1: Reverse bifid seriation
    QString deseriated = applySeriation(text, false);

    // Step 2: Standard Playfair digraph decoding
    QString result;
    for (int i = 0; i < deseriated.size(); i += 2) {
        auto [a, b] = decodeDigraph(deseriated[i], deseriated[i + 1]);
        result.append(a);
        result.append(b);
    }

    double elapsed = timer.elapsed();
    m_stats.textSize = text.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherUpdated(text.size(), m_period, elapsed);
    return result;
}

/* ---- Accessors ---- */

QString SeriatedPlayfair6::keySquare() const { return m_square; }

/* ---- Reset ---- */

void SeriatedPlayfair6::resetStatistics()
{
    m_period = 5;
    buildSquare("KEYWORD");
    m_stats = Stats{};
    m_timeSum = 0.0;
}
