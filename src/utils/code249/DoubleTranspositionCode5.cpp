/**
 * @file DoubleTranspositionCode5.cpp
 * @brief DoubleTranspositionCode5 实现
 *
 * 实现双重置换密码：Myszkowski重复关键字处理与交错偏移列式回读。
 */

#include "utils/code249/DoubleTranspositionCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode5::DoubleTranspositionCode5(QObject *parent)
    : QObject(parent) {}
DoubleTranspositionCode5::~DoubleTranspositionCode5() = default;

/* ---- Configuration ---- */

void DoubleTranspositionCode5::setKeywords(const QString& key1,
                                             const QString& key2)
{
    m_key1 = key1.toUpper().remove(QChar(' '));
    m_key2 = key2.toUpper().remove(QChar(' '));
}
void DoubleTranspositionCode5::setStaggerOffset(int offset)
{
    m_staggerOffset = qMax(0, offset);
}

/* ---- Build Myszkowski column order ---- */

QVector<QVector<int>> DoubleTranspositionCode5::buildMyszkowskiOrder(
    const QString& keyword) const
{
    int len = keyword.size();
    if (len == 0) return {};

    // Rank each character: sorted unique chars get rank 0,1,2,...
    // Tied characters share the same rank
    QVector<QPair<QChar, int>> indexed;
    for (int i = 0; i < len; ++i)
        indexed.append({keyword[i], i});

    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first;
              });

    QVector<int> ranks(len, 0);
    int curRank = 0;
    ranks[indexed[0].second] = 0;
    for (int i = 1; i < len; ++i) {
        if (indexed[i].first != indexed[i - 1].first) curRank++;
        ranks[indexed[i].second] = curRank;
    }

    // Group columns by rank (Myszkowski: tied cols read together)
    int maxRank = *std::max_element(ranks.begin(), ranks.end());
    QVector<QVector<int>> order(maxRank + 1);
    for (int i = 0; i < len; ++i)
        order[ranks[i]].append(i);

    return order;
}

/* ---- Fill grid row-wise ---- */

QVector<QVector<QChar>> DoubleTranspositionCode5::fillGrid(
    const QString& text, int cols) const
{
    int rows = qCeil(static_cast<double>(text.size()) / qMax(1, cols));
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
    for (int i = 0; i < text.size(); ++i)
        grid[i / cols][i % cols] = text[i];
    return grid;
}

/* ---- Apply staggered offset ---- */

QVector<int> DoubleTranspositionCode5::applyStagger(
    const QVector<int>& cols) const
{
    QVector<int> result = cols;
    for (int i = 0; i < result.size(); ++i)
        result[i] = (result[i] + m_staggerOffset * i) % cols.size();
    return result;
}

/* ---- Read grid columns in Myszkowski order ---- */

QString DoubleTranspositionCode5::readGridMyszkowski(
    const QVector<QVector<QChar>>& grid,
    const QVector<QVector<int>>& order) const
{
    QString result;
    int rows = grid.size();

    for (const auto& group : order) {
        // Myszkowski: for tied columns, read row by row across all tied cols
        for (int r = 0; r < rows; ++r)
            for (int c : group)
                result.append(grid[r][c]);
    }
    return result;
}

/* ---- Inverse Myszkowski read ---- */

QString DoubleTranspositionCode5::inverseMyszkowskiRead(
    const QString& cipher, int rows, int cols,
    const QVector<QVector<int>>& order) const
{
    // Reconstruct grid from cipher read in Myszkowski order
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));

    int pos = 0;
    for (const auto& group : order) {
        for (int r = 0; r < rows; ++r)
            for (int c : group) {
                if (pos < cipher.size())
                    grid[r][c] = cipher[pos++];
            }
    }

    // Read grid row-wise
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.append(grid[r][c]);
    return result;
}

/* ---- Single columnar transposition ---- */

QString DoubleTranspositionCode5::columnarTransposition(
    const QString& text, const QString& keyword, bool encrypt) const
{
    int cols = keyword.size();
    if (cols == 0 || text.isEmpty()) return text;

    auto order = buildMyszkowskiOrder(keyword);

    if (encrypt) {
        auto grid = fillGrid(text, cols);
        return readGridMyszkowski(grid, order);
    } else {
        int rows = qCeil(static_cast<double>(text.size()) / qMax(1, cols));
        return inverseMyszkowskiRead(text, rows, cols, order);
    }
}

/* ---- Encrypt via double transposition ---- */

QString DoubleTranspositionCode5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_key1.isEmpty() || m_key2.isEmpty() || plaintext.isEmpty())
        return plaintext;

    // First transposition with key1
    QString afterFirst = columnarTransposition(plaintext, m_key1, true);

    // Second transposition with key2
    QString ciphertext = columnarTransposition(afterFirst, m_key2, true);

    m_stats.inputLength = plaintext.size();
    m_stats.keyLength1 = m_key1.size();
    m_stats.keyLength2 = m_key2.size();
    m_stats.numEncryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptionCompleted(ciphertext.size(), timer.elapsed());
    return ciphertext;
}

/* ---- Decrypt via inverse double transposition ---- */

QString DoubleTranspositionCode5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_key1.isEmpty() || m_key2.isEmpty() || ciphertext.isEmpty())
        return ciphertext;

    // Reverse second transposition with key2
    QString afterSecond = columnarTransposition(ciphertext, m_key2, false);

    // Reverse first transposition with key1
    QString plaintext = columnarTransposition(afterSecond, m_key1, false);

    m_stats.inputLength = ciphertext.size();
    m_stats.numDecryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(plaintext.size(), timer.elapsed());
    return plaintext;
}

/* ---- Reset ---- */

void DoubleTranspositionCode5::resetStatistics()
{
    m_key1.clear();
    m_key2.clear();
    m_staggerOffset = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
