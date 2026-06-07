/**
 * @file TwoSquareCode2.cpp
 * @brief TwoSquareCode2 实现
 *
 * 实现双方阵密码：纵横配对网格生成、交叉坐标双字母映射、加密解密。
 */

#include "utils/code199/TwoSquareCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TwoSquareCode2::TwoSquareCode2(QObject *parent)
    : QObject(parent), m_alphabet("ABCDEFGHIKLMNOPQRSTUVWXYZ")
{
    m_square1 = buildKeySquare(m_key1.isEmpty() ? m_alphabet : m_key1);
    m_square2 = buildKeySquare(m_key2.isEmpty() ? m_alphabet : m_key2);
}

TwoSquareCode2::~TwoSquareCode2() = default;

/* ---- Configuration ---- */

void TwoSquareCode2::setKey1(const QString& key)
{
    m_key1 = key;
    m_square1 = buildKeySquare(key);
}

void TwoSquareCode2::setKey2(const QString& key)
{
    m_key2 = key;
    m_square2 = buildKeySquare(key);
}

void TwoSquareCode2::setLayout(Layout layout) { m_layout = layout; }
void TwoSquareCode2::setAlphabet(const QString& alpha) { m_alphabet = alpha; }

/* ---- Build 5x5 key square ---- */

QVector<QVector<QChar>> TwoSquareCode2::buildKeySquare(const QString& key) const
{
    QVector<QVector<QChar>> square(5, QVector<QChar>(5));
    QString used;
    QString alpha = m_alphabet;

    // Fill key characters first (unique only)
    for (QChar ch : key.toUpper()) {
        if (ch == 'J') ch = 'I';
        if (!used.contains(ch) && alpha.contains(ch)) used.append(ch);
    }
    // Fill remaining alphabet
    for (QChar ch : alpha) {
        if (!used.contains(ch)) used.append(ch);
    }

    int idx = 0;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            square[r][c] = used[idx++];
    return square;
}

/* ---- Find position ---- */

QPair<int, int> TwoSquareCode2::findPosition(const QVector<QVector<QChar>>& square, QChar ch) const
{
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            if (square[r][c] == ch) return {r, c};
    return {0, 0};
}

/* ---- Char index ---- */

int TwoSquareCode2::charIndex(QChar ch) const
{
    return m_alphabet.indexOf(ch.toUpper());
}

/* ---- Preprocess text ---- */

QString TwoSquareCode2::preprocess(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (ch.isLetter()) {
            if (ch == 'J') ch = 'I';
            result.append(ch);
        }
    }
    // Pad with 'X' if odd length
    if (result.size() % 2 != 0) result.append('X');
    return result;
}

/* ---- Map digraph via cross-coordinate ---- */

QPair<QChar, QChar> TwoSquareCode2::mapDigraph(QChar a, QChar b, bool enc) const
{
    auto pos1 = findPosition(m_square1, a);
    auto pos2 = findPosition(m_square2, b);
    int r1 = pos1.first, c1 = pos1.second;
    int r2 = pos2.first, c2 = pos2.second;

    if (m_layout == Layout::Vertical) {
        // Vertical layout: both grids share columns
        // Cross-map: row of A with col of B, row of B with col of A
        int newC1 = c2;
        int newC2 = c1;
        // If same column, shift down/up
        if (c1 == c2) {
            if (enc) { newC1 = (c1 + 1) % 5; newC2 = (c2 + 1) % 5; }
            else { newC1 = (c1 + 4) % 5; newC2 = (c2 + 4) % 5; }
        }
        return {m_square1[r1][newC1], m_square2[r2][newC2]};
    } else {
        // Horizontal layout: both grids share rows
        int newR1 = r2;
        int newR2 = r1;
        if (r1 == r2) {
            if (enc) { newR1 = (r1 + 1) % 5; newR2 = (r2 + 1) % 5; }
            else { newR1 = (r1 + 4) % 5; newR2 = (r2 + 4) % 5; }
        }
        return {m_square1[newR1][c1], m_square2[newR2][c2]};
    }
}

/* ---- Encrypt ---- */

QString TwoSquareCode2::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString processed = preprocess(plaintext);
    QString result;

    for (int i = 0; i < processed.size(); i += 2) {
        auto mapped = mapDigraph(processed[i], processed[i + 1], true);
        result.append(mapped.first);
        result.append(mapped.second);
    }

    const_cast<TwoSquareCode2*>(this)->m_stats.totalOperations++;
    const_cast<TwoSquareCode2*>(this)->m_stats.inputLength = plaintext.size();
    const_cast<TwoSquareCode2*>(this)->m_stats.outputLength = result.size();
    const_cast<TwoSquareCode2*>(this)->m_timeSum += timer.elapsed();
    const_cast<TwoSquareCode2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOperations;

    emit const_cast<TwoSquareCode2*>(this)->operationCompleted(
        "encrypt", plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString TwoSquareCode2::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString processed = preprocess(ciphertext);
    QString result;

    for (int i = 0; i < processed.size(); i += 2) {
        auto mapped = mapDigraph(processed[i], processed[i + 1], false);
        result.append(mapped.first);
        result.append(mapped.second);
    }

    const_cast<TwoSquareCode2*>(this)->m_stats.totalOperations++;
    const_cast<TwoSquareCode2*>(this)->m_stats.inputLength = ciphertext.size();
    const_cast<TwoSquareCode2*>(this)->m_stats.outputLength = result.size();
    const_cast<TwoSquareCode2*>(this)->m_timeSum += timer.elapsed();
    const_cast<TwoSquareCode2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOperations;

    emit const_cast<TwoSquareCode2*>(this)->operationCompleted(
        "decrypt", ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void TwoSquareCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
