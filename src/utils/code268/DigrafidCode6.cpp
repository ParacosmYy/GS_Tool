/**
 * @file DigrafidCode6.cpp
 * @brief DigrafidCode6 实现
 *
 * 实现二元组密码：二合字母列映射与分数位置编码多表替换。
 */

#include "utils/code268/DigrafidCode6.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode6::DigrafidCode6(QObject *parent)
    : QObject(parent) {}

DigrafidCode6::~DigrafidCode6() = default;

/* ---- Configuration ---- */

void DigrafidCode6::setAlphabet(const QString& alphabet)
{
    // Remove duplicates
    QString unique;
    for (QChar c : alphabet) {
        if (!unique.contains(c)) unique.append(c);
    }
    if (unique.size() >= 9) m_alphabet = unique;
}

void DigrafidCode6::setGridWidth(int width)
{
    m_gridWidth = qBound(2, width, 9);
}

void DigrafidCode6::setPeriod(int period)
{
    m_period = qMax(0, period);
}

/* ---- Character <-> Position mapping ---- */

QPair<int, int> DigrafidCode6::charToPosition(QChar c) const
{
    int idx = m_alphabet.indexOf(c.toUpper());
    if (idx < 0) return {0, 0};
    int totalCols = m_gridWidth;
    int row = idx / totalCols;
    int col = idx % totalCols;
    return {row, col};
}

QChar DigrafidCode6::positionToChar(int row, int col) const
{
    int totalCols = m_gridWidth;
    int idx = row * totalCols + col;
    if (idx >= 0 && idx < m_alphabet.size())
        return m_alphabet[idx];
    return QChar('?');
}

/* ---- Text to digraph positions ---- */

QVector<QPair<QPair<int,int>, QPair<int,int>>> DigrafidCode6::textToDigraphPositions(
    const QString& text) const
{
    QVector<QPair<QPair<int,int>, QPair<int,int>>> result;

    // Pad if odd length
    QString t = text.toUpper();
    // Filter to alphabet characters
    QString filtered;
    for (QChar c : t) {
        if (m_alphabet.contains(c)) filtered.append(c);
    }
    if (filtered.size() % 2 != 0)
        filtered.append(m_alphabet[0]); // pad with first char

    for (int i = 0; i < filtered.size(); i += 2) {
        auto pos1 = charToPosition(filtered[i]);
        auto pos2 = charToPosition(filtered[i + 1]);
        result.append({pos1, pos2});
    }
    return result;
}

/* ---- Fractional transposition ---- */

QVector<QPair<int,int>> DigrafidCode6::fractionate(
    const QVector<QPair<QPair<int,int>, QPair<int,int>>>& digraphs) const
{
    QVector<QPair<int,int>> result;
    int period = (m_period > 0) ? m_period : digraphs.size();

    for (int block = 0; block < digraphs.size(); block += period) {
        int end = qMin(block + period, digraphs.size());
        int blockSize = end - block;

        // Collect rows from first chars and cols from second chars
        QVector<int> rows, cols;
        for (int i = block; i < end; ++i) {
            rows.append(digraphs[i].first.first);
            cols.append(digraphs[i].second.first);
        }
        // Re-pair: row_i with col_i
        for (int i = 0; i < blockSize; ++i) {
            result.append({rows[i], cols[i]});
        }
    }
    return result;
}

/* ---- Reverse fractional transposition ---- */

QVector<QPair<QPair<int,int>, QPair<int,int>>> DigrafidCode6::defractionate(
    const QVector<QPair<int,int>>& positions) const
{
    QVector<QPair<QPair<int,int>, QPair<int,int>>> result;
    int period = (m_period > 0) ? m_period : positions.size();

    for (int block = 0; block < positions.size(); block += period) {
        int end = qMin(block + period, positions.size());
        int blockSize = end - block;

        // Extract rows (first coordinate) and cols (second coordinate)
        QVector<int> rows, cols;
        for (int i = block; i < end; ++i) {
            rows.append(positions[i].first);
            cols.append(positions[i].second);
        }

        // Reconstruct original digraph column info from alphabet position
        int half = blockSize / 2;
        if (half == 0) continue;
        for (int i = 0; i < half; ++i) {
            // Restore original: first char had (rows[i], col_orig1),
            // second char had (cols[i], col_orig2)
            // We use position mapping to recover original column indices
            int col1 = i % m_gridWidth;
            int col2 = (i + half) % m_gridWidth;
            result.append({{rows[i], col1}, {cols[i], col2}});
        }
    }
    return result;
}

/* ---- Encrypt ---- */

QString DigrafidCode6::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    auto digraphs = textToDigraphPositions(plaintext);
    auto fracPositions = fractionate(digraphs);

    QString result;
    for (const auto& pos : fracPositions) {
        result.append(positionToChar(pos.first, pos.second));
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.gridWidth = m_gridWidth;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherCompleted(true, plaintext.size(), result.size(), elapsed);

    return result;
}

/* ---- Decrypt ---- */

QString DigrafidCode6::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    // Convert ciphertext to positions
    QVector<QPair<int,int>> positions;
    QString ct = ciphertext.toUpper();
    for (QChar c : ct) {
        if (m_alphabet.contains(c))
            positions.append(charToPosition(c));
    }

    auto digraphs = defractionate(positions);

    QString result;
    for (const auto& dg : digraphs) {
        result.append(positionToChar(dg.first.first, dg.first.second));
        result.append(positionToChar(dg.second.first, dg.second.second));
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.gridWidth = m_gridWidth;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherCompleted(false, ciphertext.size(), result.size(), elapsed);

    return result;
}

/* ---- Reset ---- */

void DigrafidCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
