/**
 * @file BazeleriesCode5.cpp
 * @brief BazeleriesCode5 实现
 *
 * 实现巴泽里密码：嵌套列置换与密钥派生乱序读取。
 */

#include "utils/code257/BazeleriesCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode5::BazeleriesCode5(QObject *parent)
    : QObject(parent) {}
BazeleriesCode5::~BazeleriesCode5() = default;

/* ---- Configuration ---- */

void BazeleriesCode5::setPrimaryKey(const QString& key) { m_primaryKey = key.toUpper(); }
void BazeleriesCode5::setSecondaryKey(const QString& key) { m_secondaryKey = key.toUpper(); }
void BazeleriesCode5::setDisruptionPattern(const QString& pattern) { m_disruption = pattern; }

/* ---- Derive column permutation from key ---- */

QVector<int> BazeleriesCode5::keyPermutation(const QString& key) const
{
    int n = key.length();
    if (n == 0) return {};

    // Build index-value pairs and sort alphabetically
    QVector<QPair<QChar, int>> pairs;
    for (int i = 0; i < n; ++i)
        pairs.append({key[i], i});

    std::stable_sort(pairs.begin(), pairs.end(),
                     [](const QPair<QChar, int>& a, const QPair<QChar, int>& b) {
                         return a.first < b.first;
                     });

    // Permutation: position in sorted order
    QVector<int> perm(n);
    for (int rank = 0; rank < n; ++rank)
        perm[pairs[rank].second] = rank;
    return perm;
}

/* ---- Single columnar transposition ---- */

QString BazeleriesCode5::columnarTranspose(const QString& text, const QString& key, bool encrypt) const
{
    if (key.isEmpty()) return text;
    int cols = key.length();
    QVector<int> perm = keyPermutation(key);
    int rows = qCeil(static_cast<double>(text.length()) / cols);
    int totalLen = rows * cols;

    // Pad text
    QString padded = text;
    while (padded.length() < totalLen) padded += 'X';

    if (encrypt) {
        // Write by row, read by permuted column
        QString result;
        for (int rank = 0; rank < cols; ++rank) {
            // Find column with this rank
            int col = -1;
            for (int c = 0; c < cols; ++c)
                if (perm[c] == rank) { col = c; break; }
            for (int r = 0; r < rows; ++r)
                result += padded[r * cols + col];
        }
        return result;
    } else {
        // Read by permuted column, write by row
        QString filled(totalLen, ' ');
        int idx = 0;
        for (int rank = 0; rank < cols; ++rank) {
            int col = -1;
            for (int c = 0; c < cols; ++c)
                if (perm[c] == rank) { col = c; break; }
            for (int r = 0; r < rows; ++r)
                filled[r * cols + col] = text[idx++];
        }
        return filled;
    }
}

/* ---- Build disrupted grid ---- */

QVector<QVector<QChar>> BazeleriesCode5::buildDisruptedGrid(
    const QString& text, int rows, int cols, const QVector<int>& disruptOrder) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, ' '));
    int idx = 0;

    // Fill grid with disrupted pattern
    // Disrupted reading: fill certain diagonals/blocks first
    for (int d = 0; d < static_cast<int>(disruptOrder.size()) && idx < text.length(); ++d) {
        int startCol = disruptOrder[d];
        for (int r = 0; r < rows && idx < text.length(); ++r) {
            int c = (startCol + r) % cols;
            if (grid[r][c] == ' ') {
                grid[r][c] = text[idx++];
            }
        }
    }

    // Fill remaining cells row by row
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols && idx < text.length(); ++c)
            if (grid[r][c] == ' ')
                grid[r][c] = text[idx++];

    return grid;
}

/* ---- Disrupted read ---- */

QString BazeleriesCode5::disruptedRead(const QString& text, int cols,
                                         const QVector<int>& order, bool byRow) const
{
    if (text.isEmpty() || cols <= 0) return text;
    int rows = qCeil(static_cast<double>(text.length()) / cols);
    auto grid = buildDisruptedGrid(text, rows, cols, order);

    QString result;
    if (byRow) {
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                if (grid[r][c] != ' ') result += grid[r][c];
    } else {
        for (int c : order)
            for (int r = 0; r < rows; ++r)
                if (grid[r][c] != ' ') result += grid[r][c];
    }
    return result;
}

/* ---- Encrypt ---- */

QString BazeleriesCode5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString text = plaintext.toUpper();
    // Remove non-alpha characters for cipher operation
    QString cleaned;
    for (const QChar& ch : text)
        if (ch.isLetter()) cleaned += ch;

    // First transposition with primary key
    QString first = columnarTranspose(cleaned, m_primaryKey, true);

    // Apply disrupted reading if pattern is set
    QString disrupted = first;
    if (!m_disruption.isEmpty()) {
        QVector<int> disruptOrder;
        for (const QChar& ch : m_disruption)
            disruptOrder.append(ch.digitValue() - 1);
        disrupted = disruptedRead(first, m_primaryKey.length(), disruptOrder, false);
    }

    // Second (nested) transposition with secondary key
    QString result = columnarTranspose(disrupted, m_secondaryKey, true);

    double elapsed = timer.elapsed();
    m_stats.inputLength = cleaned.length();
    m_stats.outputLength = result.length();
    m_stats.numColumns = m_primaryKey.length();
    m_stats.numRows = qCeil(static_cast<double>(cleaned.length()) / qMax(1, m_primaryKey.length()));
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(true, cleaned.length(), result.length(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString BazeleriesCode5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Reverse second transposition
    QString second = columnarTranspose(ciphertext, m_secondaryKey, false);

    // Reverse disrupted reading
    QString undisrupted = second;
    if (!m_disruption.isEmpty()) {
        QVector<int> disruptOrder;
        for (const QChar& ch : m_disruption)
            disruptOrder.append(ch.digitValue() - 1);
        undisrupted = disruptedRead(second, m_primaryKey.length(), disruptOrder, true);
    }

    // Reverse first transposition
    QString result = columnarTranspose(undisrupted, m_primaryKey, false);

    // Strip padding
    while (!result.isEmpty() && result.endsWith('X'))
        result.chop(1);

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit cipherCompleted(false, ciphertext.length(), result.length(), elapsed);
    return result;
}

/* ---- Reset ---- */

void BazeleriesCode5::resetStatistics()
{
    m_primaryKey.clear();
    m_secondaryKey.clear();
    m_disruption.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
