/**
 * @file TranspositionCode.cpp
 * @brief TranspositionCode 实现
 *
 * 实现列置换密码：密钥列序、多轮置换、重合指数分析、自动破解。
 */

#include "utils/code176/TranspositionCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TranspositionCode::TranspositionCode(QObject *parent)
    : QObject(parent)
{
}

TranspositionCode::~TranspositionCode() = default;

/* ---- Configuration ---- */

void TranspositionCode::setKey(const QString& key) { m_key = key.toUpper(); }
void TranspositionCode::setRounds(int rounds) { m_rounds = qMax(1, rounds); }

/* ---- Key ordering ---- */

QVector<int> TranspositionCode::keyOrder() const
{
    int n = m_key.size();
    if (n == 0) return {};

    /* Sort characters by rank, stable for duplicates */
    QVector<QPair<QChar, int>> pairs;
    for (int i = 0; i < n; ++i)
        pairs.append({m_key[i], i});

    std::stable_sort(pairs.begin(), pairs.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    QVector<int> order(n);
    for (int i = 0; i < n; ++i)
        order[pairs[i].second] = i;
    return order;
}

/* ---- Single round encrypt ---- */

QString TranspositionCode::encryptSingle(const QString& text,
                                          const QVector<int>& order) const
{
    int cols = order.size();
    if (cols == 0) return text;

    int rows = qCeil(static_cast<double>(text.size()) / cols);

    /* Fill grid row by row */
    QVector<QString> grid(rows);
    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (idx < text.size())
                grid[r] += text[idx++];
            else
                grid[r] += QChar('X'); /* Padding */
        }
    }

    /* Read columns in key order */
    QString result;
    for (int c = 0; c < cols; ++c) {
        /* Find which original column maps to position c */
        int origCol = -1;
        for (int j = 0; j < order.size(); ++j) {
            if (order[j] == c) { origCol = j; break; }
        }
        if (origCol < 0) continue;
        for (int r = 0; r < rows; ++r)
            result += grid[r][origCol];
    }
    return result;
}

/* ---- Single round decrypt ---- */

QString TranspositionCode::decryptSingle(const QString& text,
                                          const QVector<int>& order) const
{
    int cols = order.size();
    if (cols == 0) return text;

    int rows = qCeil(static_cast<double>(text.size()) / cols);

    /* Determine column lengths */
    QVector<int> colLen(cols, rows);

    /* Read columns in key order to reconstruct grid */
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols));
    int idx = 0;
    for (int c = 0; c < cols; ++c) {
        int origCol = -1;
        for (int j = 0; j < order.size(); ++j) {
            if (order[j] == c) { origCol = j; break; }
        }
        if (origCol < 0) continue;
        for (int r = 0; r < colLen[origCol] && idx < text.size(); ++r)
            grid[r][origCol] = text[idx++];
    }

    /* Read grid row by row */
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result += grid[r][c];
    return result;
}

/* ---- Encrypt / Decrypt (multi-round) ---- */

QString TranspositionCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString result = plaintext.toUpper();
    auto order = keyOrder();

    for (int r = 0; r < m_rounds; ++r)
        result = encryptSingle(result, order);

    m_stats.totalEncryptions++;
    m_stats.lastKeyLength = m_key.size();
    m_stats.lastMessageLength = plaintext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit encryptionCompleted(plaintext.size(), m_key.size());
    return result;
}

QString TranspositionCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString result = ciphertext.toUpper();
    auto order = keyOrder();

    for (int r = 0; r < m_rounds; ++r)
        result = decryptSingle(result, order);

    m_stats.totalDecryptions++;
    m_stats.lastMessageLength = ciphertext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptionCompleted(ciphertext.size());
    return result;
}

/* ---- Index of Coincidence ---- */

double TranspositionCode::indexCoincidence(const QString& text) const
{
    int n = text.size();
    if (n <= 1) return 0.0;

    /* Count letter frequencies (A-Z) */
    int freq[26] = {};
    int total = 0;
    for (const QChar& ch : text) {
        if (ch.isLetter()) {
            freq[ch.toUpper().toLatin1() - 'A']++;
            total++;
        }
    }

    if (total <= 1) return 0.0;
    double ic = 0.0;
    for (int i = 0; i < 26; ++i)
        ic += freq[i] * (freq[i] - 1);
    ic /= total * (total - 1);
    return ic;
}

/* ---- Key length analysis ---- */

QVector<QPair<int, double>> TranspositionCode::analyzeKeyLength(
    const QString& ciphertext, int maxKeyLen) const
{
    QVector<QPair<int, double>> results;

    for (int kl = 2; kl <= maxKeyLen; ++kl) {
        /* Split ciphertext into kl groups and compute average IC */
        double avgIC = 0.0;
        for (int offset = 0; offset < kl; ++offset) {
            QString group;
            for (int i = offset; i < ciphertext.size(); i += kl)
                group += ciphertext[i];
            avgIC += indexCoincidence(group);
        }
        avgIC /= kl;

        /* English IC ≈ 0.0667; score = closeness to English IC */
        double score = 1.0 / (1.0 + qAbs(avgIC - 0.0667));
        results.append({kl, score});
    }

    /* Sort by score descending */
    std::sort(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    if (!results.isEmpty())
        emit analysisCompleted(results.first().first, results.first().second);

    return results;
}

/* ---- Auto-crack ---- */

QVector<QString> TranspositionCode::crack(const QString& ciphertext,
                                           int maxKeyLen) const
{
    QVector<QString> candidates;
    auto analysis = analyzeKeyLength(ciphertext, maxKeyLen);

    /* Try top-3 key length candidates */
    int tries = qMin(3, analysis.size());
    for (int t = 0; t < tries; ++t) {
        int kl = analysis[t].first;

        /* Try all permutations for small key lengths */
        if (kl <= 6) {
            QVector<int> order(kl);
            for (int i = 0; i < kl; ++i) order[i] = i;

            auto perm = order;
            int bestScore = 0;
            QString bestText;

            do {
                /* Decrypt with this column order */
                QString plain;
                int rows = qCeil(static_cast<double>(ciphertext.size()) / kl);
                QVector<QVector<QChar>> grid(rows, QVector<QChar>(kl));
                int idx = 0;
                for (int c = 0; c < kl; ++c)
                    for (int r = 0; r < rows && idx < ciphertext.size(); ++r)
                        grid[r][perm[c]] = ciphertext[idx++];
                for (int r = 0; r < rows; ++r)
                    for (int c = 0; c < kl; ++c)
                        plain += grid[r][c];

                /* Score by letter frequency (ETAON shrdr) */
                int score = 0;
                for (const QChar& ch : plain) {
                    char c = ch.toUpper().toLatin1();
                    if (c == 'E' || c == 'T' || c == 'A' || c == 'O' || c == 'N')
                        score++;
                }
                if (score > bestScore) {
                    bestScore = score;
                    bestText = plain;
                }
            } while (std::next_permutation(perm.begin(), perm.end()));

            if (!bestText.isEmpty())
                candidates.append(bestText);
        }
    }
    return candidates;
}

void TranspositionCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
