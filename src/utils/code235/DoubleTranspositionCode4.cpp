/**
 * @file DoubleTranspositionCode4.cpp
 * @brief DoubleTranspositionCode4 实现
 *
 * 实现双重置换密码：双独立关键字列置换与位置扰乱。
 */

#include "utils/code235/DoubleTranspositionCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode4::DoubleTranspositionCode4(QObject *parent) : QObject(parent) {}
DoubleTranspositionCode4::~DoubleTranspositionCode4() = default;

/* ---- Configuration ---- */

void DoubleTranspositionCode4::setKeyword1(const QString& key) { m_keyword1 = key.toUpper(); }
void DoubleTranspositionCode4::setKeyword2(const QString& key) { m_keyword2 = key.toUpper(); }
void DoubleTranspositionCode4::setPaddingChar(QChar ch) { m_paddingChar = ch; }
void DoubleTranspositionCode4::setScrambleEnabled(bool enable) { m_scrambleEnabled = enable; }

/* ---- Column order from keyword ---- */

QVector<int> DoubleTranspositionCode4::columnOrder(const QString& keyword) const
{
    int n = keyword.size();
    if (n == 0) return {};

    // Build (char, original_index) pairs
    QVector<QPair<QChar, int>> pairs;
    for (int i = 0; i < n; ++i)
        pairs.append({keyword[i], i});

    // Sort by character, then by position for stability
    std::stable_sort(pairs.begin(), pairs.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });

    // Map original index to sorted position
    QVector<int> order(n);
    for (int i = 0; i < n; ++i)
        order[pairs[i].second] = i;
    return order;
}

/* ---- Single columnar transposition encrypt ---- */

QString DoubleTranspositionCode4::columnEncrypt(const QString& text, const QVector<int>& order) const
{
    int cols = order.size();
    if (cols == 0) return text;

    // Pad text to fill complete rectangle
    int rows = qCeil(static_cast<double>(text.size()) / cols);
    int totalLen = rows * cols;

    QString padded = text;
    while (padded.size() < totalLen)
        padded.append(m_paddingChar);

    // Build columns in original order, read in sorted order
    QVector<QString> columns(cols);
    for (int c = 0; c < cols; ++c)
        for (int r = 0; r < rows; ++r)
            columns[c].append(padded[r * cols + c]);

    // Read columns in key-sorted order
    QString result;
    // Create inverse mapping: order[c] = rank, so invRank[rank] = c
    QVector<int> invRank(cols);
    for (int c = 0; c < cols; ++c)
        invRank[order[c]] = c;

    for (int rank = 0; rank < cols; ++rank)
        result.append(columns[invRank[rank]]);

    return result;
}

/* ---- Single columnar transposition decrypt ---- */

QString DoubleTranspositionCode4::columnDecrypt(const QString& text, const QVector<int>& order) const
{
    int cols = order.size();
    if (cols == 0) return text;

    int totalLen = text.size();
    int rows = qCeil(static_cast<double>(totalLen) / cols);

    // Split ciphertext into columns (in sorted order)
    QVector<int> invRank(cols);
    for (int c = 0; c < cols; ++c)
        invRank[order[c]] = c;

    QVector<QString> columns(cols);
    int idx = 0;
    for (int rank = 0; rank < cols; ++rank) {
        int colLen = qMin(rows, totalLen - idx);
        columns[invRank[rank]] = text.mid(idx, rows);
        idx += rows;
    }

    // Reconstruct row by row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (r < columns[c].size())
                result.append(columns[c][r]);

    return result;
}

/* ---- Scramble pattern from combined key ---- */

QVector<int> DoubleTranspositionCode4::scramblePattern(int length) const
{
    // Seed from combined key
    quint32 seed = 0;
    for (QChar ch : m_keyword1 + m_keyword2)
        seed = seed * 31 + static_cast<quint32>(ch.unicode());

    QVector<int> perm(length);
    for (int i = 0; i < length; ++i) perm[i] = i;

    // Fisher-Yates shuffle with LCG
    for (int i = length - 1; i > 0; --i) {
        seed = seed * 1103515245 + 12345;
        int j = static_cast<int>((seed >> 16) % (i + 1));
        if (j < 0) j += i + 1;
        std::swap(perm[i], perm[j]);
    }
    return perm;
}

/* ---- Apply positional scramble ---- */

QString DoubleTranspositionCode4::scramble(const QString& text) const
{
    if (!m_scrambleEnabled) return text;
    QVector<int> pattern = scramblePattern(text.size());
    QString result;
    result.resize(text.size());
    for (int i = 0; i < text.size(); ++i)
        result[pattern[i]] = text[i];
    return result;
}

/* ---- Reverse positional scramble ---- */

QString DoubleTranspositionCode4::unscramble(const QString& text) const
{
    if (!m_scrambleEnabled) return text;
    QVector<int> pattern = scramblePattern(text.size());
    QString result;
    result.resize(text.size());
    for (int i = 0; i < text.size(); ++i)
        result[i] = text[pattern[i]];
    return result;
}

/* ---- Encrypt ---- */

DoubleTranspositionCode4::CipherResult DoubleTranspositionCode4::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;

    if (m_keyword1.isEmpty() || m_keyword2.isEmpty()) {
        result.text = plaintext;
        return result;
    }

    // First transposition with keyword 1
    QVector<int> order1 = columnOrder(m_keyword1);
    QString afterFirst = columnEncrypt(plaintext, order1);

    // Positional scramble
    QString afterScramble = scramble(afterFirst);

    // Second transposition with keyword 2
    QVector<int> order2 = columnOrder(m_keyword2);
    result.text = columnEncrypt(afterScramble, order2);

    result.key1OrderLen = order1.size();
    result.key2OrderLen = order2.size();
    result.paddingAdded = result.text.size() - plaintext.size();

    m_stats.numEncryptions++;
    m_stats.totalCharsProcessed += plaintext.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptionCompleted(plaintext.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

DoubleTranspositionCode4::CipherResult DoubleTranspositionCode4::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;

    if (m_keyword1.isEmpty() || m_keyword2.isEmpty()) {
        result.text = ciphertext;
        return result;
    }

    // Reverse second transposition
    QVector<int> order2 = columnOrder(m_keyword2);
    QString afterFirstDecrypt = columnDecrypt(ciphertext, order2);

    // Reverse scramble
    QString afterUnscramble = unscramble(afterFirstDecrypt);

    // Reverse first transposition
    QVector<int> order1 = columnOrder(m_keyword1);
    result.text = columnDecrypt(afterUnscramble, order1);

    result.key1OrderLen = order1.size();
    result.key2OrderLen = order2.size();

    m_stats.numDecryptions++;
    m_stats.totalCharsProcessed += ciphertext.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(ciphertext.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DoubleTranspositionCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
