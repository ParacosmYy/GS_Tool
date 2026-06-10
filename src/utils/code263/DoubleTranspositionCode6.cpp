/**
 * @file DoubleTranspositionCode6.cpp
 * @brief DoubleTranspositionCode6 实现
 *
 * 实现双重置换密码：Nihilist复合列置换与关键字栅栏分层排列。
 */

#include "utils/code263/DoubleTranspositionCode6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode6::DoubleTranspositionCode6(QObject *parent)
    : QObject(parent) {}

DoubleTranspositionCode6::~DoubleTranspositionCode6() = default;

/* ---- Configuration ---- */

void DoubleTranspositionCode6::setColumnKeyword(const QString& keyword)
{
    m_colKeyword = keyword.toUpper();
}

void DoubleTranspositionCode6::setRowKeyword(const QString& keyword)
{
    m_rowKeyword = keyword.toUpper();
}

void DoubleTranspositionCode6::setRailFenceRails(int rails)
{
    m_rails = qMax(2, rails);
}

/* ---- Normalize text ---- */

QString DoubleTranspositionCode6::normalize(const QString& text)
{
    QString out;
    for (const QChar& c : text) {
        if (c.isLetter()) out.append(c.toUpper());
    }
    return out;
}

/* ---- Keyword order via stable sort ---- */

QVector<int> DoubleTranspositionCode6::keywordOrder(const QString& keyword) const
{
    int n = keyword.size();
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    // Stable sort by character value
    std::stable_sort(indices.begin(), indices.end(), [&keyword](int a, int b) {
        return keyword[a] < keyword[b];
    });
    // Map from original position to rank
    QVector<int> order(n);
    for (int rank = 0; rank < n; ++rank)
        order[indices[rank]] = rank;
    return order;
}

/* ---- Columnar transposition encrypt ---- */

QString DoubleTranspositionCode6::columnarEncrypt(const QString& text,
                                                    const QString& keyword) const
{
    if (keyword.isEmpty() || text.isEmpty()) return text;
    int cols = keyword.size();
    int rows = qCeil(static_cast<double>(text.size()) / cols);

    // Build grid row by row
    QVector<QString> grid(rows);
    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols && idx < text.size(); ++c)
            grid[r].append(text[idx++]);
        while (grid[r].size() < cols)
            grid[r].append('X');  // Padding
    }

    // Read columns in keyword order
    QVector<int> order = keywordOrder(keyword);
    QString result;
    for (int rank = 0; rank < cols; ++rank) {
        // Find column with this rank
        for (int c = 0; c < cols; ++c) {
            if (order[c] == rank) {
                for (int r = 0; r < rows; ++r)
                    result.append(grid[r][c]);
                break;
            }
        }
    }
    return result;
}

/* ---- Columnar transposition decrypt ---- */

QString DoubleTranspositionCode6::columnarDecrypt(const QString& text,
                                                    const QString& keyword) const
{
    if (keyword.isEmpty() || text.isEmpty()) return text;
    int cols = keyword.size();
    int rows = qCeil(static_cast<double>(text.size()) / cols);
    int total = rows * cols;

    // Determine column read order
    QVector<int> order = keywordOrder(keyword);

    // Calculate column lengths (last row may be partial)
    int fullCells = text.size();
    QVector<int> colLen(cols, rows);
    int excess = total - fullCells;
    // Columns with highest order get shorter
    QVector<int> sortedCols(cols);
    for (int i = 0; i < cols; ++i) sortedCols[order[i]] = i;
    for (int i = 0; i < excess; ++i)
        colLen[sortedCols[cols - 1 - i]] = rows - 1;

    // Read back column by column in keyword order
    QVector<QString> colData(cols);
    int idx = 0;
    for (int rank = 0; rank < cols; ++rank) {
        for (int c = 0; c < cols; ++c) {
            if (order[c] != rank) continue;
            for (int r = 0; r < colLen[c] && idx < text.size(); ++r)
                colData[c].append(text[idx++]);
        }
    }

    // Reconstruct grid row by row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (r < colData[c].size())
                result.append(colData[c][r]);

    return result;
}

/* ---- Rail fence encrypt ---- */

QString DoubleTranspositionCode6::railFenceEncrypt(const QString& text, int rails) const
{
    if (rails <= 1 || text.isEmpty()) return text;
    int n = text.size();
    QVector<QString> fence(rails);

    int rail = 0, dir = 1;
    for (int i = 0; i < n; ++i) {
        fence[rail].append(text[i]);
        if (rail == 0) dir = 1;
        else if (rail == rails - 1) dir = -1;
        rail += dir;
    }

    QString result;
    for (const auto& s : fence) result.append(s);
    return result;
}

/* ---- Rail fence decrypt ---- */

QString DoubleTranspositionCode6::railFenceDecrypt(const QString& text, int rails) const
{
    if (rails <= 1 || text.isEmpty()) return text;
    int n = text.size();

    // Compute length of each rail
    QVector<int> railLen(rails, 0);
    int rail = 0, dir = 1;
    for (int i = 0; i < n; ++i) {
        railLen[rail]++;
        if (rail == 0) dir = 1;
        else if (rail == rails - 1) dir = -1;
        rail += dir;
    }

    // Split ciphertext into rails
    QVector<QString> fence(rails);
    int idx = 0;
    for (int r = 0; r < rails; ++r) {
        fence[r] = text.mid(idx, railLen[r]);
        idx += railLen[r];
    }

    // Read zigzag
    QString result;
    QVector<int> railIdx(rails, 0);
    rail = 0; dir = 1;
    for (int i = 0; i < n; ++i) {
        result.append(fence[rail][railIdx[rail]++]);
        if (rail == 0) dir = 1;
        else if (rail == rails - 1) dir = -1;
        rail += dir;
    }
    return result;
}

/* ---- Encrypt ---- */

QString DoubleTranspositionCode6::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString normalized = normalize(plaintext);

    // Layer 1: Columnar transposition with column keyword
    QString afterCol = columnarEncrypt(normalized, m_colKeyword);

    // Layer 2: Rail fence interleaving
    QString afterRail = railFenceEncrypt(afterCol, m_rails);

    // Layer 3: Second columnar transposition with row keyword
    QString result = columnarEncrypt(afterRail, m_rowKeyword);

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.numCols = m_colKeyword.size();
    m_stats.numRows = m_rowKeyword.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherApplied(plaintext.size(), true, elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString DoubleTranspositionCode6::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Reverse order: undo Layer 3, then Layer 2, then Layer 1
    QString step1 = columnarDecrypt(ciphertext, m_rowKeyword);
    QString step2 = railFenceDecrypt(step1, m_rails);
    QString result = columnarDecrypt(step2, m_colKeyword);

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.size();
    m_stats.numCols = m_colKeyword.size();
    m_stats.numRows = m_rowKeyword.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherApplied(ciphertext.size(), false, elapsed);
    return result;
}

/* ---- Reset ---- */

void DoubleTranspositionCode6::resetStatistics()
{
    m_colKeyword.clear();
    m_rowKeyword.clear();
    m_rails = 3;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
