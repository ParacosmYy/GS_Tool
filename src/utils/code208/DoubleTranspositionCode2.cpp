/**
 * @file DoubleTranspositionCode2.cpp
 * @brief DoubleTranspositionCode2 实现
 *
 * 实现双重置换密码：列-行乘积加密、基于已知明文的密钥重建。
 */

#include "utils/code208/DoubleTranspositionCode2.h"

#include <QElapsedTimer>
#include <QSet>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode2::DoubleTranspositionCode2(QObject *parent) : QObject(parent) {}
DoubleTranspositionCode2::~DoubleTranspositionCode2() = default;

/* ---- Validate key permutation ---- */

bool DoubleTranspositionCode2::validateKey(const QVector<int>& key)
{
    int n = key.size();
    if (n == 0) return false;
    QSet<int> seen;
    for (int v : key) {
        if (v < 0 || v >= n || seen.contains(v)) return false;
        seen.insert(v);
    }
    return true;
}

/* ---- Invert permutation ---- */

QVector<int> DoubleTranspositionCode2::invertKey(const QVector<int>& key)
{
    int n = key.size();
    QVector<int> inv(n, 0);
    for (int i = 0; i < n; ++i)
        inv[key[i]] = i;
    return inv;
}

/* ---- Column transposition ---- */

QString DoubleTranspositionCode2::columnTransposition(
    const QString& text, const QVector<int>& key, bool encrypt) const
{
    int cols = key.size();
    if (cols == 0) return text;
    int rows = (text.size() + cols - 1) / cols;
    int totalCells = rows * cols;

    // Pad with spaces if needed
    QString padded = text;
    while (padded.size() < totalCells) padded += QLatin1Char(' ');

    auto effectiveKey = encrypt ? key : invertKey(key);

    // Read/write by permuted column order
    QString result;
    result.reserve(totalCells);
    for (int c = 0; c < cols; ++c) {
        int srcCol = effectiveKey[c];
        for (int r = 0; r < rows; ++r)
            result += padded[r * cols + srcCol];
    }
    return result;
}

/* ---- Row transposition ---- */

QString DoubleTranspositionCode2::rowTransposition(
    const QString& text, const QVector<int>& key, bool encrypt) const
{
    int rows = key.size();
    if (rows == 0) return text;
    int cols = (text.size() + rows - 1) / rows;
    int totalCells = rows * cols;

    QString padded = text;
    while (padded.size() < totalCells) padded += QLatin1Char(' ');

    auto effectiveKey = encrypt ? key : invertKey(key);

    QString result;
    result.reserve(totalCells);
    for (int r = 0; r < rows; ++r) {
        int srcRow = effectiveKey[r];
        for (int c = 0; c < cols; ++c)
            result += padded[srcRow * cols + c];
    }
    return result;
}

/* ---- Encrypt ---- */

QString DoubleTranspositionCode2::encrypt(const QString& plaintext,
                                           const QVector<int>& colKey,
                                           const QVector<int>& rowKey) const
{
    QElapsedTimer timer;
    timer.start();

    if (!validateKey(colKey) || !validateKey(rowKey)) return plaintext;

    // Column transposition first, then row transposition
    QString afterCol = columnTransposition(plaintext, colKey, true);
    QString result = rowTransposition(afterCol, rowKey, true);

    auto self = const_cast<DoubleTranspositionCode2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.lastInputLen = plaintext.size();
    self->m_stats.lastColKeyLen = colKey.size();
    self->m_stats.lastRowKeyLen = rowKey.size();
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit operationCompleted("encrypt", plaintext.size(), timer.elapsed());

    return result;
}

/* ---- Decrypt ---- */

QString DoubleTranspositionCode2::decrypt(const QString& ciphertext,
                                           const QVector<int>& colKey,
                                           const QVector<int>& rowKey) const
{
    QElapsedTimer timer;
    timer.start();

    if (!validateKey(colKey) || !validateKey(rowKey)) return ciphertext;

    // Reverse order: row inverse first, then column inverse
    QString afterRow = rowTransposition(ciphertext, rowKey, false);
    QString result = columnTransposition(afterRow, colKey, false);

    auto self = const_cast<DoubleTranspositionCode2*>(this);
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit operationCompleted("decrypt", ciphertext.size(), timer.elapsed());

    return result.trimmed();
}

/* ---- Generate permutations ---- */

QVector<QVector<int>> DoubleTranspositionCode2::generatePermutations(
    int len, int maxLen) const
{
    QVector<QVector<int>> results;
    if (len <= 0 || len > maxLen || len > 7) return results;

    QVector<int> perm(len);
    for (int i = 0; i < len; ++i) perm[i] = i;

    do {
        results.append(perm);
    } while (std::next_permutation(perm.begin(), perm.end()));

    return results;
}

/* ---- N-gram frequency score ---- */

double DoubleTranspositionCode2::ngramScore(const QString& text) const
{
    // Simple English letter frequency scoring
    static const double freq[26] = {
        8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094, 6.966,
        0.153, 0.772, 4.025, 2.406, 6.749, 7.507, 1.929, 0.095, 5.987,
        6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.074
    };
    double score = 0.0;
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();
        if (ch >= 'A' && ch <= 'Z')
            score += freq[ch.toLatin1() - 'A'];
    }
    return score;
}

/* ---- Score decryption quality ---- */

double DoubleTranspositionCode2::scoreDecryption(const QString& candidate,
                                                   const QString& expected) const
{
    if (candidate.size() != expected.size()) return 0.0;
    int matches = 0;
    for (int i = 0; i < candidate.size(); ++i)
        if (candidate[i] == expected[i]) ++matches;
    return static_cast<double>(matches) / qMax(1, candidate.size());
}

/* ---- Crib-based key reconstruction ---- */

QVector<QPair<QVector<int>, QVector<int>>> DoubleTranspositionCode2::reconstructKeys(
    const QString& plaintext, const QString& ciphertext,
    int maxColKey, int maxRowKey) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QVector<int>, QVector<int>>> candidates;

    // Try different key lengths
    for (int cLen = 2; cLen <= qMin(maxColKey, 6); ++cLen) {
        for (int rLen = 2; rLen <= qMin(maxRowKey, 6); ++rLen) {
            auto colPerms = generatePermutations(cLen, maxColKey);
            auto rowPerms = generatePermutations(rLen, maxRowKey);

            for (const auto& ck : colPerms) {
                for (const auto& rk : rowPerms) {
                    QString decrypted = decrypt(ciphertext, ck, rk);
                    double score = scoreDecryption(decrypted, plaintext);
                    if (score > 0.95)
                        candidates.append({ck, rk});
                }
            }
        }
    }

    auto self = const_cast<DoubleTranspositionCode2*>(this);
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return candidates;
}

/* ---- Reset ---- */

void DoubleTranspositionCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
