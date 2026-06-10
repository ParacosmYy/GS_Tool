/**
 * @file DoubleTranspositionCode7.cpp
 * @brief DoubleTranspositionCode7 实现
 *
 * 实现双重置换密码：Myszkowski变体与重复密钥列置换增强排列复杂度的双阶段加密。
 */

#include "utils/code277/DoubleTranspositionCode7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode7::DoubleTranspositionCode7(QObject *parent)
    : QObject(parent) {}

DoubleTranspositionCode7::~DoubleTranspositionCode7() = default;

/* ---- Myszkowski column order (handles repeated key chars) ---- */

QVector<QVector<int>> DoubleTranspositionCode7::myszkowskiOrder(const QString& key) const
{
    int n = key.size();
    if (n == 0) return {};

    // Build (char, original_index) pairs
    QVector<QPair<QChar, int>> pairs;
    for (int i = 0; i < n; ++i)
        pairs.append({key[i].toUpper(), i});

    // Sort by character
    std::sort(pairs.begin(), pairs.end(),
              [](const QPair<QChar, int>& a, const QPair<QChar, int>& b) {
                  return a.first < b.first;
              });

    // Group indices by character (for repeated chars)
    QVector<QVector<int>> groups;
    int i = 0;
    while (i < pairs.size()) {
        QVector<int> group;
        QChar ch = pairs[i].first;
        while (i < pairs.size() && pairs[i].first == ch) {
            group.append(pairs[i].second);
            ++i;
        }
        groups.append(group);
    }
    return groups;
}

/* ---- Standard column order (alphabetical ranking) ---- */

QVector<int> DoubleTranspositionCode7::columnOrder(const QString& key) const
{
    int n = key.size();
    QVector<QPair<QChar, int>> pairs;
    for (int i = 0; i < n; ++i)
        pairs.append({key[i].toUpper(), i});

    std::sort(pairs.begin(), pairs.end(),
              [](const QPair<QChar, int>& a, const QPair<QChar, int>& b) {
                  return a.first < b.first;
              });

    QVector<int> order(n);
    for (int i = 0; i < n; ++i)
        order[pairs[i].second] = i;
    return order;
}

/* ---- Myszkowski transpose encrypt ---- */

QString DoubleTranspositionCode7::transposeEncrypt(const QString& text, const QString& key) const
{
    int nCols = key.size();
    if (nCols == 0) return text;

    int nRows = qCeil(static_cast<double>(text.size()) / nCols);

    // Fill grid row by row
    QVector<QVector<QChar>> grid(nRows, QVector<QChar>(nCols, QChar('X')));
    int idx = 0;
    for (int r = 0; r < nRows; ++r)
        for (int c = 0; c < nCols; ++c)
            grid[r][c] = (idx < text.size()) ? text[idx++] : QChar('X');

    // Read columns in Myszkowski order (groups of repeated-key columns)
    auto groups = myszkowskiOrder(key);
    QString result;
    for (const auto& group : groups) {
        int groupSize = group.size();
        for (int r = 0; r < nRows; ++r) {
            // Read columns in this group row by row (interleaved)
            for (int c : group)
                result += grid[r][c];
        }
    }
    return result;
}

/* ---- Myszkowski transpose decrypt ---- */

QString DoubleTranspositionCode7::transposeDecrypt(const QString& text, const QString& key) const
{
    int nCols = key.size();
    if (nCols == 0) return text;

    int nRows = qCeil(static_cast<double>(text.size()) / nCols);
    int fullCells = text.size();

    auto groups = myszkowskiOrder(key);

    // Calculate cell count per group
    QVector<QVector<QChar>> grid(nRows, QVector<QChar>(nCols, QChar('X')));

    int idx = 0;
    for (const auto& group : groups) {
        for (int r = 0; r < nRows; ++r) {
            for (int c : group) {
                if (idx < text.size())
                    grid[r][c] = text[idx++];
            }
        }
    }

    // Read row by row
    QString result;
    for (int r = 0; r < nRows; ++r)
        for (int c = 0; c < nCols; ++c)
            result += grid[r][c];

    return result;
}

/* ---- Standard columnar encrypt ---- */

QString DoubleTranspositionCode7::columnarEncrypt(const QString& text, const QString& key) const
{
    int nCols = key.size();
    if (nCols == 0) return text;

    int nRows = qCeil(static_cast<double>(text.size()) / nCols);

    QVector<QVector<QChar>> grid(nRows, QVector<QChar>(nCols, QChar('X')));
    int idx = 0;
    for (int r = 0; r < nRows; ++r)
        for (int c = 0; c < nCols; ++c)
            grid[r][c] = (idx < text.size()) ? text[idx++] : QChar('X');

    auto order = columnOrder(key);
    QString result;
    for (int rank = 0; rank < nCols; ++rank) {
        // Find column with this rank
        for (int c = 0; c < nCols; ++c) {
            if (order[c] == rank) {
                for (int r = 0; r < nRows; ++r)
                    result += grid[r][c];
                break;
            }
        }
    }
    return result;
}

/* ---- Standard columnar decrypt ---- */

QString DoubleTranspositionCode7::columnarDecrypt(const QString& text, const QString& key) const
{
    int nCols = key.size();
    if (nCols == 0) return text;

    int nRows = qCeil(static_cast<double>(text.size()) / nCols);
    auto order = columnOrder(key);

    QVector<QVector<QChar>> grid(nRows, QVector<QChar>(nCols, QChar('X')));

    int idx = 0;
    for (int rank = 0; rank < nCols; ++rank) {
        for (int c = 0; c < nCols; ++c) {
            if (order[c] == rank) {
                for (int r = 0; r < nRows; ++r) {
                    if (idx < text.size())
                        grid[r][c] = text[idx++];
                }
                break;
            }
        }
    }

    QString result;
    for (int r = 0; r < nRows; ++r)
        for (int c = 0; c < nCols; ++c)
            result += grid[r][c];

    return result;
}

/* ---- Encrypt: two-stage (Myszkowski then columnar) ---- */

DoubleTranspositionCode7::CipherResult DoubleTranspositionCode7::encrypt(
    const QString& plaintext, const QString& key1, const QString& key2)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;

    // Stage 1: Myszkowski transposition with key1
    QString intermediate = transposeEncrypt(plaintext, key1);

    // Stage 2: Standard columnar transposition with key2
    result.ciphertext = columnarEncrypt(intermediate, key2);
    result.key1Order = key1;
    result.key2Order = key2;
    result.numRows1 = qCeil(static_cast<double>(plaintext.size()) / qMax(1, key1.size()));
    result.numRows2 = qCeil(static_cast<double>(intermediate.size()) / qMax(1, key2.size()));

    double elapsed = timer.elapsed();
    m_stats.textLength = plaintext.size();
    m_stats.key1Length = key1.size();
    m_stats.key2Length = key2.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptDone(plaintext.size(), key1.size(), key2.size(), elapsed);

    return result;
}

/* ---- Decrypt: reverse two-stage ---- */

QString DoubleTranspositionCode7::decrypt(
    const QString& ciphertext, const QString& key1, const QString& key2)
{
    QElapsedTimer timer;
    timer.start();

    // Reverse stage 2: columnar decrypt with key2
    QString intermediate = columnarDecrypt(ciphertext, key2);

    // Reverse stage 1: Myszkowski decrypt with key1
    QString plaintext = transposeDecrypt(intermediate, key1);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptDone(ciphertext.size(), elapsed);

    return plaintext;
}

/* ---- Reset ---- */

void DoubleTranspositionCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
