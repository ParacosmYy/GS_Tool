/**
 * @file DoubleTranspositionCode8.cpp
 * @brief DoubleTranspositionCode8 实现
 *
 * 实现双重置换密码：级联矩形网格与置换密钥调度实现复合列加密。
 */

#include "utils/code291/DoubleTranspositionCode8.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode8::DoubleTranspositionCode8(QObject *parent)
    : QObject(parent) {}

DoubleTranspositionCode8::~DoubleTranspositionCode8() = default;

/* ---- Configuration ---- */

void DoubleTranspositionCode8::setKey1(const QVector<int>& key) {
    m_key1 = normalizeKey(key);
}

void DoubleTranspositionCode8::setKey2(const QVector<int>& key) {
    m_key2 = normalizeKey(key);
}

void DoubleTranspositionCode8::setFillChar(QChar ch) {
    m_fillChar = ch;
}

/* ---- Normalize permutation key ---- */

QVector<int> DoubleTranspositionCode8::normalizeKey(const QVector<int>& key) const
{
    if (key.isEmpty()) return {0};
    // Rank the key values to produce a permutation [0..n-1]
    int n = key.size();
    QVector<QPair<int, int>> indexed(n);
    for (int i = 0; i < n; ++i)
        indexed[i] = {key[i], i};
    std::sort(indexed.begin(), indexed.end());

    QVector<int> perm(n);
    for (int rank = 0; rank < n; ++rank)
        perm[indexed[rank].second] = rank;
    return perm;
}

/* ---- Grid dimensions ---- */

void DoubleTranspositionCode8::gridDimensions(int textLen, int keyLen,
                                                int& rows, int& cols) const
{
    cols = qMax(keyLen, 1);
    rows = (textLen + cols - 1) / cols;
}

/* ---- Single columnar encrypt ---- */

QString DoubleTranspositionCode8::columnarEncrypt(const QString& text,
                                                     const QVector<int>& key,
                                                     int& rows, int& cols) const
{
    cols = key.size();
    int n = text.size();
    rows = (n + cols - 1) / cols;

    // Fill grid row by row
    QString padded = text;
    while (padded.size() < rows * cols)
        padded.append(m_fillChar);

    // Determine column read order from permutation key
    // key[i] = rank of column i
    QVector<int> order(cols);
    for (int i = 0; i < cols; ++i)
        order[key[i]] = i;

    // Read columns in rank order
    QString result;
    for (int rank = 0; rank < cols; ++rank) {
        int col = order[rank];
        for (int r = 0; r < rows; ++r)
            result.append(padded[r * cols + col]);
    }
    return result;
}

/* ---- Single columnar decrypt ---- */

QString DoubleTranspositionCode8::columnarDecrypt(const QString& text,
                                                     const QVector<int>& key,
                                                     int rows, int cols) const
{
    int n = text.size();
    if (rows <= 0 || cols <= 0) return text;

    // Determine column order from permutation key
    QVector<int> order(cols);
    for (int i = 0; i < cols; ++i)
        order[key[i]] = i;

    // Read the ciphertext into columns
    QVector<QString> columns(cols);
    int idx = 0;
    for (int rank = 0; rank < cols; ++rank) {
        int col = order[rank];
        for (int r = 0; r < rows && idx < n; ++r)
            columns[col].append(text[idx++]);
    }
    // Pad shorter columns
    for (int c = 0; c < cols; ++c)
        while (columns[c].size() < rows)
            columns[c].append(m_fillChar);

    // Read grid row by row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.append(columns[c][r]);

    return result;
}

/* ---- Encrypt ---- */

DoubleTranspositionCode8::CipherResult DoubleTranspositionCode8::encrypt(
    const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;

    // Default keys if not set
    QVector<int> k1 = m_key1.isEmpty() ? QVector<int>{2, 0, 1, 3} : m_key1;
    QVector<int> k2 = m_key2.isEmpty() ? QVector<int>{1, 3, 0, 2} : m_key2;

    // First pass: columnar transposition with key1
    int r1, c1;
    QString pass1 = columnarEncrypt(plaintext, k1, r1, c1);
    result.gridRows1 = r1;
    result.gridCols1 = c1;

    // Second pass: columnar transposition with key2
    int r2, c2;
    QString pass2 = columnarEncrypt(pass1, k2, r2, c2);
    result.gridRows2 = r2;
    result.gridCols2 = c2;

    result.output = pass2;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_stats.totalCharsProcessed += plaintext.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("encrypt"), plaintext.size(),
                       result.output.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

DoubleTranspositionCode8::CipherResult DoubleTranspositionCode8::decrypt(
    const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;

    QVector<int> k1 = m_key1.isEmpty() ? QVector<int>{2, 0, 1, 3} : m_key1;
    QVector<int> k2 = m_key2.isEmpty() ? QVector<int>{1, 3, 0, 2} : m_key2;

    // Reverse second pass
    int c2 = k2.size();
    int r2 = (ciphertext.size() + c2 - 1) / c2;
    QString pass1 = columnarDecrypt(ciphertext, k2, r2, c2);
    result.gridRows2 = r2;
    result.gridCols2 = c2;

    // Reverse first pass
    int c1 = k1.size();
    int r1 = (pass1.size() + c1 - 1) / c1;
    QString plain = columnarDecrypt(pass1, k1, r1, c1);
    result.gridRows1 = r1;
    result.gridCols1 = c1;

    result.output = plain;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_stats.totalCharsProcessed += ciphertext.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("decrypt"), ciphertext.size(),
                       result.output.size(), elapsed);
    return result;
}

/* ---- Reset ---- */

void DoubleTranspositionCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
