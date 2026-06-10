/**
 * @file BazeleriesCode6.cpp
 * @brief BazeleriesCode6 实现
 *
 * 实现Bazeleries密码：对角转置与双列置换增强多表编码。
 */

#include "utils/code271/BazeleriesCode6.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode6::BazeleriesCode6(QObject *parent)
    : QObject(parent) {}

BazeleriesCode6::~BazeleriesCode6() = default;

/* ---- Configuration ---- */

void BazeleriesCode6::setKey(const QString& key)
{
    m_key = normalize(key);
}

void BazeleriesCode6::setSecondKey(const QString& key)
{
    m_secondKey = normalize(key);
}

/* ---- Normalize: uppercase alpha only ---- */

QString BazeleriesCode6::normalize(const QString& text) const
{
    QString result;
    result.reserve(text.size());
    for (QChar ch : text) {
        if (ch.isLetter())
            result.append(ch.toUpper());
    }
    return result;
}

/* ---- Key order: alphabetical ranking of characters ---- */

QVector<int> BazeleriesCode6::keyOrder(const QString& key) const
{
    int n = key.size();
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;

    // Stable sort by character value to get alphabetical ranking
    std::stable_sort(order.begin(), order.end(), [&key](int a, int b) {
        return key[a] < key[b];
    });

    // Convert to rank: order[i] = position of column i in sorted sequence
    QVector<int> rank(n);
    for (int i = 0; i < n; ++i)
        rank[order[i]] = i;
    return rank;
}

/* ---- Inverse key order for decryption ---- */

QVector<int> BazeleriesCode6::inverseOrder(const QVector<int>& order) const
{
    int n = order.size();
    QVector<int> inv(n);
    for (int i = 0; i < n; ++i)
        inv[order[i]] = i;
    return inv;
}

/* ---- Diagonal transposition: write text into grid diagonally ---- */

QVector<QVector<QChar>> BazeleriesCode6::diagonalWrite(const QString& text, int cols) const
{
    int len = text.size();
    int rows = (len + cols - 1) / cols;

    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));

    int idx = 0;
    for (int diag = 0; diag < rows + cols - 1 && idx < len; ++diag) {
        for (int r = 0; r < rows && idx < len; ++r) {
            int c = diag - r;
            if (c >= 0 && c < cols) {
                grid[r][c] = text[idx++];
            }
        }
    }
    return grid;
}

/* ---- Diagonal transposition: read grid diagonally ---- */

QString BazeleriesCode6::diagonalRead(const QVector<QVector<QChar>>& grid, int cols) const
{
    QString result;
    int rows = grid.size();

    for (int diag = 0; diag < rows + cols - 1; ++diag) {
        for (int r = 0; r < rows; ++r) {
            int c = diag - r;
            if (c >= 0 && c < cols)
                result.append(grid[r][c]);
        }
    }
    return result;
}

/* ---- Columnar transposition (encrypt direction) ---- */

QString BazeleriesCode6::columnarTransposition(const QString& text, const QVector<int>& order) const
{
    int cols = order.size();
    int len = text.size();
    int rows = (len + cols - 1) / cols;

    // Pad text to fill grid
    QString padded = text;
    while (padded.size() < rows * cols)
        padded.append(QChar('X'));

    // Read columns in order dictated by key ranking
    QString result;
    result.reserve(len);
    for (int rank = 0; rank < cols; ++rank) {
        // Find which column has this rank
        for (int c = 0; c < cols; ++c) {
            if (order[c] == rank) {
                for (int r = 0; r < rows; ++r)
                    result.append(padded[r * cols + c]);
                break;
            }
        }
    }
    return result;
}

/* ---- Inverse columnar transposition (decrypt direction) ---- */

QString BazeleriesCode6::columnarInverse(const QString& text, const QVector<int>& order) const
{
    int cols = order.size();
    int len = text.size();
    int rows = (len + cols - 1) / cols;

    // Determine how many characters go in each column
    QVector<int> colOrder(cols);
    for (int c = 0; c < cols; ++c)
        colOrder[c] = order[c];

    QVector<int> sortedCols = colOrder;
    std::sort(sortedCols.begin(), sortedCols.end());

    QVector<int> colLengths(cols, rows);

    QString result;
    result.resize(rows * cols, QChar('X'));

    int srcPos = 0;
    for (int rank = 0; rank < cols; ++rank) {
        // Find column with this rank
        for (int c = 0; c < cols; ++c) {
            if (order[c] == rank) {
                for (int r = 0; r < colLengths[c]; ++r)
                    result[r * cols + c] = text[srcPos++];
                break;
            }
        }
    }

    // Remove padding
    while (result.size() > len)
        result.chop(1);
    return result;
}

/* ---- Encrypt: normalize -> diagonal write -> columnar transposition -> double columnar ---- */

QString BazeleriesCode6::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString norm = normalize(plaintext);
    if (norm.isEmpty() || m_key.isEmpty()) return norm;

    int cols = m_key.size();

    // Phase 1: Diagonal transposition
    auto grid = diagonalWrite(norm, cols);
    QString diagText = diagonalRead(grid, cols);

    // Phase 2: First columnar transposition
    QVector<int> order1 = keyOrder(m_key);
    QString colText = columnarTransposition(diagText, order1);

    // Phase 3: Double columnar with second key (if available)
    QString result;
    if (!m_secondKey.isEmpty()) {
        QVector<int> order2 = keyOrder(m_secondKey);
        result = columnarTransposition(colText, order2);
    } else {
        result = colText;
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = norm.size();
    m_stats.keyLength = m_key.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherDone(norm.size(), m_key.size(), elapsed);

    return result;
}

/* ---- Decrypt: inverse double columnar -> inverse columnar -> inverse diagonal ---- */

QString BazeleriesCode6::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString norm = normalize(ciphertext);
    if (norm.isEmpty() || m_key.isEmpty()) return norm;

    int cols = m_key.size();

    // Phase 3 inverse: Double columnar inverse
    QString afterDouble;
    if (!m_secondKey.isEmpty()) {
        QVector<int> order2 = keyOrder(m_secondKey);
        QVector<int> invOrder2 = inverseOrder(order2);
        afterDouble = columnarInverse(norm, invOrder2);
    } else {
        afterDouble = norm;
    }

    // Phase 2 inverse: Columnar transposition inverse
    QVector<int> order1 = keyOrder(m_key);
    QVector<int> invOrder1 = inverseOrder(order1);
    QString afterCol = columnarInverse(afterDouble, invOrder1);

    // Phase 1 inverse: Diagonal transposition inverse
    int len = afterCol.size();
    int rows = (len + cols - 1) / cols;
    auto grid = QVector<QVector<QChar>>(rows, QVector<QChar>(cols, QChar('X')));

    int idx = 0;
    for (int diag = 0; diag < rows + cols - 1 && idx < len; ++diag) {
        for (int r = 0; r < rows && idx < len; ++r) {
            int c = diag - r;
            if (c >= 0 && c < cols)
                grid[r][c] = afterCol[idx++];
        }
    }

    // Read row by row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.append(grid[r][c]);

    // Trim padding X's from end
    while (result.endsWith(QChar('X')))
        result.chop(1);

    double elapsed = timer.elapsed();
    m_stats.inputLength = norm.size();
    m_stats.keyLength = m_key.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherDone(norm.size(), m_key.size(), elapsed);

    return result;
}

/* ---- Reset ---- */

void BazeleriesCode6::resetStatistics()
{
    m_key.clear();
    m_secondKey.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
