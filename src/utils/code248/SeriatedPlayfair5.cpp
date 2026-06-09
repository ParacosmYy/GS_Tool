/**
 * @file SeriatedPlayfair5.cpp
 * @brief SeriatedPlayfair5 实现
 *
 * 实现序列化普莱费尔密码：随机化Polybius方格填充与双字母有向图可逆性验证。
 */

#include "utils/code248/SeriatedPlayfair5.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair5::SeriatedPlayfair5(QObject *parent) : QObject(parent)
{
    buildSquare();
}

SeriatedPlayfair5::~SeriatedPlayfair5() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair5::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper().trimmed();
    buildSquare();
}

void SeriatedPlayfair5::setSeriationPeriod(int period) { m_period = qMax(1, period); }

/* ---- Letter index mapping (J -> I) ---- */

int SeriatedPlayfair5::letterIndex(QChar ch) const
{
    ch = ch.toUpper();
    if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
    int idx = ch.toLatin1() - 'A';
    return (idx >= 0 && idx < 26) ? idx : -1;
}

/* ---- Build 5x5 Polybius square from keyword ---- */

void SeriatedPlayfair5::buildSquare()
{
    m_square = QVector<QVector<QChar>>(5, QVector<QChar>(5));
    m_rowOf.resize(26, -1);
    m_colOf.resize(26, -1);

    QVector<bool> used(26, false);
    used['J' - 'A'] = true; // J merged into I

    // Insert keyword letters first
    QVector<QChar> seq;
    for (QChar ch : m_keyword) {
        int idx = letterIndex(ch);
        if (idx >= 0 && !used[idx]) {
            seq.append(ch.toUpper() == QLatin1Char('J') ? QLatin1Char('I') : ch.toUpper());
            used[idx] = true;
        }
    }

    // Fill remaining with randomized order
    QVector<int> remaining;
    for (int i = 0; i < 26; ++i)
        if (!used[i]) remaining.append(i);

    // Fisher-Yates shuffle for randomization
    for (int i = remaining.size() - 1; i > 0; --i) {
        int j = qrand() % (i + 1);
        qSwap(remaining[i], remaining[j]);
    }
    for (int idx : remaining)
        seq.append(QChar(QLatin1Char('A') + idx));

    // Fill 5x5 square
    int pos = 0;
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 5; ++c) {
            QChar ch = seq[pos++];
            m_square[r][c] = ch;
            int li = letterIndex(ch);
            if (li >= 0) { m_rowOf[li] = r; m_colOf[li] = c; }
        }
    }
}

/* ---- Find position of letter in square ---- */

bool SeriatedPlayfair5::findPosition(QChar ch, int& row, int& col) const
{
    int idx = letterIndex(ch);
    if (idx < 0) return false;
    row = m_rowOf[idx];
    col = m_colOf[idx];
    return (row >= 0 && col >= 0);
}

/* ---- Encode a single digraph ---- */

QPair<QChar, QChar> SeriatedPlayfair5::encodeDigraph(QChar a, QChar b) const
{
    int r1, c1, r2, c2;
    findPosition(a, r1, c1);
    findPosition(b, r2, c2);

    if (r1 == r2) {
        // Same row: shift right
        return {m_square[r1][(c1 + 1) % 5], m_square[r2][(c2 + 1) % 5]};
    } else if (c1 == c2) {
        // Same column: shift down
        return {m_square[(r1 + 1) % 5][c1], m_square[(r2 + 1) % 5][c2]};
    } else {
        // Rectangle: swap columns
        return {m_square[r1][c2], m_square[r2][c1]};
    }
}

/* ---- Decode a single digraph ---- */

QPair<QChar, QChar> SeriatedPlayfair5::decodeDigraph(QChar a, QChar b) const
{
    int r1, c1, r2, c2;
    findPosition(a, r1, c1);
    findPosition(b, r2, c2);

    if (r1 == r2) {
        return {m_square[r1][(c1 + 4) % 5], m_square[r2][(c2 + 4) % 5]};
    } else if (c1 == c2) {
        return {m_square[(r1 + 4) % 5][c1], m_square[(r2 + 4) % 5][c2]};
    } else {
        return {m_square[r1][c2], m_square[r2][c1]};
    }
}

/* ---- Prepare text: uppercase, J->I, pad ---- */

QString SeriatedPlayfair5::prepareText(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (ch.isLetter()) {
            if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
            result.append(ch);
        }
    }
    return result;
}

/* ---- Break into digraphs (pad double letters with X) ---- */

QVector<QPair<QChar, QChar>> SeriatedPlayfair5::makeDigraphs(const QString& text) const
{
    QVector<QPair<QChar, QChar>> digraphs;
    QString t = text;
    int i = 0;
    while (i < t.size()) {
        QChar a = t[i];
        QChar b = (i + 1 < t.size()) ? t[i + 1] : QLatin1Char('X');
        if (a == b) {
            b = QLatin1Char('X'); // Pad with X for double letters
            digraphs.append({a, b});
            i++; // Only advance by 1
        } else {
            digraphs.append({a, b});
            i += 2;
        }
    }
    return digraphs;
}

/* ---- Apply seriation (columnar transposition) ---- */

QString SeriatedPlayfair5::applySeriation(const QString& text, int period) const
{
    if (period <= 1 || text.isEmpty()) return text;
    int rows = qCeil(static_cast<double>(text.size()) / period);
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < period; ++c) {
            int idx = c * rows + r;
            if (idx < text.size()) result.append(text[idx]);
        }
    return result;
}

/* ---- Reverse seriation ---- */

QString SeriatedPlayfair5::reverseSeriation(const QString& text, int period) const
{
    if (period <= 1 || text.isEmpty()) return text;
    int rows = qCeil(static_cast<double>(text.size()) / period);
    QString result;
    result.resize(text.size(), QLatin1Char('X'));
    int srcIdx = 0;
    for (int c = 0; c < period; ++c)
        for (int r = 0; r < rows; ++r) {
            int dstIdx = c * rows + r;
            if (dstIdx < text.size() && srcIdx < text.size())
                result[dstIdx] = text[srcIdx++];
        }
    return result;
}

/* ---- Encrypt ---- */

SeriatedPlayfair5::CipherResult SeriatedPlayfair5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString prepared = prepareText(plaintext);

    // Apply seriation before encoding
    QString seriated = applySeriation(prepared, m_period);
    auto digraphs = makeDigraphs(seriated);

    QString output;
    for (const auto& dg : digraphs) {
        auto encoded = encodeDigraph(dg.first, dg.second);
        output.append(encoded.first);
        output.append(encoded.second);
    }

    result.output = output;
    result.numDigraphs = digraphs.size();

    // Verify reversibility
    CipherResult decResult;
    decResult.output = output;
    auto decDigraphs = makeDigraphs(output);
    QString decoded;
    for (const auto& dg : decDigraphs) {
        auto d = decodeDigraph(dg.first, dg.second);
        decoded.append(d.first);
        decoded.append(d.second);
    }
    decoded = reverseSeriation(decoded, m_period);
    result.reversible = (decoded == prepared);

    m_stats.numEncryptions++;
    m_stats.numDigraphsProcessed += digraphs.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted("encrypt", digraphs.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

SeriatedPlayfair5::CipherResult SeriatedPlayfair5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    auto digraphs = makeDigraphs(ciphertext);

    QString decoded;
    for (const auto& dg : digraphs) {
        auto d = decodeDigraph(dg.first, dg.second);
        decoded.append(d.first);
        decoded.append(d.second);
    }

    // Reverse seriation
    result.output = reverseSeriation(decoded, m_period);
    result.numDigraphs = digraphs.size();
    result.reversible = true;

    m_stats.numDecryptions++;
    m_stats.numDigraphsProcessed += digraphs.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted("decrypt", digraphs.size(), timer.elapsed());
    return result;
}

/* ---- Verify round-trip ---- */

bool SeriatedPlayfair5::verifyReversibility(const QString& plaintext)
{
    auto enc = encrypt(plaintext);
    auto dec = decrypt(enc.output);
    QString origPrep = prepareText(plaintext);
    return (dec.output == origPrep);
}

/* ---- Get current square ---- */

QVector<QVector<QChar>> SeriatedPlayfair5::square() const { return m_square; }

/* ---- Reset ---- */

void SeriatedPlayfair5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
