/**
 * @file DoubleTranspositionCode.cpp
 * @brief DoubleTranspositionCode 实现
 *
 * 实现双重列置换密码：单/双列置换、乘积组合、已知明文攻击。
 */

#include "utils/code190/DoubleTranspositionCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode::DoubleTranspositionCode(QObject *parent)
    : QObject(parent) {}
DoubleTranspositionCode::~DoubleTranspositionCode() = default;

/* ---- Configuration ---- */

void DoubleTranspositionCode::setKeys(const QString& key1, const QString& key2)
{
    m_key1 = key1.toUpper();
    m_key2 = key2.toUpper();
}

/* ---- Derive column order from keyword ---- */

QVector<int> DoubleTranspositionCode::keyOrder(const QString& key) const
{
    int n = key.size();
    if (n == 0) return {};

    // Create index-key pairs, sort by character then by position for stability
    QVector<QPair<QChar, int>> pairs;
    for (int i = 0; i < n; ++i)
        pairs.append({key[i], i});

    std::stable_sort(pairs.begin(), pairs.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

    QVector<int> order(n);
    for (int rank = 0; rank < n; ++rank)
        order[pairs[rank].second] = rank;
    return order;
}

/* ---- Fill grid row-wise ---- */

QVector<QVector<QChar>> DoubleTranspositionCode::fillRowWise(
    const QString& text, int cols) const
{
    int rows = qCeil(static_cast<double>(text.size()) / cols);
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));

    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (idx < text.size())
                grid[r][c] = text[idx++];
        }
    }
    return grid;
}

/* ---- Read off columns in key order ---- */

QString DoubleTranspositionCode::readOffColumns(
    const QVector<QVector<QChar>>& grid,
    const QVector<int>& order) const
{
    QString result;
    int cols = order.size();
    int rows = grid.size();

    // Create reverse mapping: rank -> original column
    QVector<int> rankToCol(cols);
    for (int c = 0; c < cols; ++c)
        rankToCol[order[c]] = c;

    for (int rank = 0; rank < cols; ++rank) {
        int col = rankToCol[rank];
        for (int r = 0; r < rows; ++r)
            result += grid[r][col];
    }
    return result;
}

/* ---- Single columnar transposition encrypt ---- */

QString DoubleTranspositionCode::singleEncrypt(const QString& text,
                                                 const QString& key) const
{
    if (key.isEmpty() || text.isEmpty()) return text;
    auto order = keyOrder(key);
    int cols = key.size();
    auto grid = fillRowWise(text, cols);
    return readOffColumns(grid, order);
}

/* ---- Single columnar transposition decrypt ---- */

QString DoubleTranspositionCode::singleDecrypt(const QString& text,
                                                 const QString& key) const
{
    if (key.isEmpty() || text.isEmpty()) return text;
    int cols = key.size();
    int rows = qCeil(static_cast<double>(text.size()) / cols);

    auto order = keyOrder(key);

    // Create reverse mapping: rank -> original column
    QVector<int> rankToCol(cols);
    for (int c = 0; c < cols; ++c)
        rankToCol[order[c]] = c;

    // Fill columns by reading in ranked order
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
    int idx = 0;
    for (int rank = 0; rank < cols; ++rank) {
        int col = rankToCol[rank];
        for (int r = 0; r < rows; ++r) {
            if (idx < text.size())
                grid[r][col] = text[idx++];
        }
    }

    // Read off row-wise
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result += grid[r][c];
    return result;
}

/* ---- Double encrypt ---- */

QString DoubleTranspositionCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // First transposition
    QString intermediate = singleEncrypt(plaintext, m_key1);
    // Second transposition (product cipher)
    QString ciphertext = singleEncrypt(intermediate, m_key2);

    m_stats.encryptCount++;
    m_stats.totalOperations++;
    m_stats.key1Length = m_key1.size();
    m_stats.key2Length = m_key2.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", plaintext.size(), timer.elapsed());
    return ciphertext;
}

/* ---- Double decrypt ---- */

QString DoubleTranspositionCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Reverse second transposition first
    QString intermediate = singleDecrypt(ciphertext, m_key2);
    // Reverse first transposition
    QString plaintext = singleDecrypt(intermediate, m_key1);

    m_stats.decryptCount++;
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decrypt", ciphertext.size(), timer.elapsed());
    return plaintext;
}

/* ---- Score a candidate by matching crib position ---- */

double DoubleTranspositionCode::scoreCandidate(const QString& candidate,
                                                 const QString& crib) const
{
    if (candidate.size() < crib.size()) return std::numeric_limits<double>::max();

    double score = 0.0;
    for (int i = 0; i <= candidate.size() - crib.size(); ++i) {
        double match = 0.0;
        for (int j = 0; j < crib.size(); ++j) {
            if (candidate[i + j].toUpper() == crib[j].toUpper())
                match += 1.0;
        }
        double ratio = match / crib.size();
        if (ratio > score) score = ratio;
    }
    return 1.0 - score; // Lower is better
}

/* ---- Crib-based attack ---- */

QVector<QPair<QString, QString>> DoubleTranspositionCode::cribAttack(
    const QString& ciphertext, const QString& crib, int maxKeyLen)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QString, QString>> candidates;

    // Try all key length combinations for first and second transposition
    for (int k1 = 2; k1 <= maxKeyLen; ++k1) {
        for (int k2 = 2; k2 <= maxKeyLen; ++k2) {
            // Generate permutations of [0..k1-1] for key1 order
            QVector<int> perm1(k1);
            for (int i = 0; i < k1; ++i) perm1[i] = i;

            int attempts = 0;
            int maxAttempts = qMin(120, k1 * k1 * k2 * k2);

            do {
                // Build key string from permutation
                QString testKey1;
                for (int p : perm1) testKey1 += QChar('A' + p);

                // For key2, try a few fixed permutations
                for (int p2 = 0; p2 < qMin(k2, 3); ++p2) {
                    QString testKey2;
                    QVector<int> perm2(k2);
                    for (int i = 0; i < k2; ++i) perm2[i] = (i + p2) % k2;
                    for (int p : perm2) testKey2 += QChar('A' + p);

                    // Try decrypting with these keys
                    QString result = singleDecrypt(
                        singleDecrypt(ciphertext, testKey2), testKey1);

                    double s = scoreCandidate(result, crib);
                    if (s < 0.2) {
                        candidates.append({testKey1, testKey2});
                    }
                }

                attempts++;
                if (attempts >= maxAttempts) break;
            } while (std::next_permutation(perm1.begin(), perm1.end()));
        }
    }

    // Sort candidates by score (best first)
    std::sort(candidates.begin(), candidates.end(),
        [this, &ciphertext, &crib](const auto& a, const auto& b) {
            auto da = singleDecrypt(singleDecrypt(ciphertext, a.second), a.first);
            auto db = singleDecrypt(singleDecrypt(ciphertext, b.second), b.first);
            return scoreCandidate(da, crib) < scoreCandidate(db, crib);
        });

    // Keep top 10
    if (candidates.size() > 10)
        candidates.resize(10);

    m_stats.attackCount++;
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("cribAttack", ciphertext.size(), timer.elapsed());
    return candidates;
}

/* ---- Reset ---- */

void DoubleTranspositionCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
