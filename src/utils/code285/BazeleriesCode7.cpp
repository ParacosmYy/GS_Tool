/**
 * @file BazeleriesCode7.cpp
 * @brief BazeleriesCode7 实现
 *
 * 实现Bazeleries密码：嵌套对角列置换与关键字派生矩形网格的多层排列。
 */

#include "utils/code285/BazeleriesCode7.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode7::BazeleriesCode7(QObject *parent)
    : QObject(parent) {}

BazeleriesCode7::~BazeleriesCode7() = default;

/* ---- Configuration ---- */

void BazeleriesCode7::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper().remove(QLatin1Char(' '));
}

/* ---- Derive column order from keyword ---- */

QVector<int> BazeleriesCode7::deriveColumnOrder(const QString& keyword) const
{
    int n = keyword.size();
    if (n == 0) return {};

    QVector<QPair<QChar, int>> indexed;
    indexed.reserve(n);
    for (int i = 0; i < n; ++i)
        indexed.append(qMakePair(keyword[i], i));

    // Sort by character, then by original position for stability
    std::stable_sort(indexed.begin(), indexed.end(),
                     [](const auto& a, const auto& b) {
                         return a.first < b.first;
                     });

    QVector<int> order(n);
    for (int i = 0; i < n; ++i)
        order[indexed[i].second] = i;
    return order;
}

/* ---- Build rectangular grid ---- */

QVector<QVector<QChar>> BazeleriesCode7::buildGrid(const QString& text, int cols) const
{
    int rows = (text.size() + cols - 1) / cols;
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QLatin1Char('X')));
    for (int i = 0; i < text.size(); ++i)
        grid[i / cols][i % cols] = text[i];
    return grid;
}

/* ---- Read grid in diagonal (Bazeleries pattern) ---- */

QString BazeleriesCode7::readDiagonal(const QVector<QVector<QChar>>& grid, int cols) const
{
    QString result;
    int rows = grid.size();
    // Read diagonals starting from first column and first row
    for (int diag = 0; diag < rows + cols - 1; ++diag) {
        for (int r = 0; r < rows; ++r) {
            int c = diag - r;
            if (c >= 0 && c < cols)
                result.append(grid[r][c]);
        }
    }
    return result;
}

/* ---- Fill grid from diagonal-ordered text ---- */

QVector<QVector<QChar>> BazeleriesCode7::fillDiagonal(const QString& text, int rows, int cols) const
{
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QLatin1Char('X')));
    int idx = 0;
    for (int diag = 0; diag < rows + cols - 1; ++diag) {
        for (int r = 0; r < rows; ++r) {
            int c = diag - r;
            if (c >= 0 && c < cols && idx < text.size())
                grid[r][c] = text[idx++];
        }
    }
    return grid;
}

/* ---- Columnar transposition ---- */

QString BazeleriesCode7::columnarTranspose(const QString& text, const QVector<int>& order, int cols) const
{
    int rows = (text.size() + cols - 1) / cols;
    // Build grid row-by-row
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QLatin1Char('X')));
    for (int i = 0; i < text.size(); ++i)
        grid[i / cols][i % cols] = text[i];

    // Read columns in keyword order
    QString result;
    for (int rank = 0; rank < cols; ++rank) {
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

/* ---- Reverse columnar transposition ---- */

QString BazeleriesCode7::columnarReverse(const QString& text, const QVector<int>& order, int rows, int cols) const
{
    // Determine how many characters per column
    QVector<int> colLens(cols, rows);

    // Fill columns in keyword order
    QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QLatin1Char('X')));
    int idx = 0;
    for (int rank = 0; rank < cols; ++rank) {
        for (int c = 0; c < cols; ++c) {
            if (order[c] == rank) {
                for (int r = 0; r < colLens[c] && idx < text.size(); ++r)
                    grid[r][c] = text[idx++];
                break;
            }
        }
    }

    // Read row-by-row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.append(grid[r][c]);
    return result;
}

/* ---- Compute grid dimensions ---- */

QPair<int, int> BazeleriesCode7::computeDimensions(int textLen) const
{
    int cols = qMax(2, m_keyword.size());
    int rows = (textLen + cols - 1) / cols;
    return qMakePair(rows, cols);
}

/* ---- Encrypt ---- */

BazeleriesCode7::CipherResult BazeleriesCode7::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString prepared = plaintext.toUpper().remove(QLatin1Char(' '));
    if (prepared.isEmpty() || m_keyword.isEmpty()) return result;

    auto [rows, cols] = computeDimensions(prepared.size());
    result.gridRows = rows;
    result.gridCols = cols;

    // Step 1: Build grid and read diagonally (Bazeleries first layer)
    auto grid = buildGrid(prepared, cols);
    QString diagText = readDiagonal(grid, cols);

    // Step 2: Columnar transposition using keyword order
    QVector<int> order = deriveColumnOrder(m_keyword);
    result.columnOrder = order;
    QString transposed = columnarTranspose(diagText, order, cols);

    // Step 3: Second diagonal pass (nested transposition)
    auto grid2 = buildGrid(transposed, cols);
    result.text = readDiagonal(grid2, cols);

    double elapsed = timer.elapsed();
    m_stats.numEncrypts++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptDone(result.text.size(), elapsed);

    return result;
}

/* ---- Decrypt ---- */

BazeleriesCode7::CipherResult BazeleriesCode7::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    if (ciphertext.isEmpty() || m_keyword.isEmpty()) return result;

    auto [rows, cols] = computeDimensions(ciphertext.size());
    result.gridRows = rows;
    result.gridCols = cols;

    QVector<int> order = deriveColumnOrder(m_keyword);
    result.columnOrder = order;

    // Reverse step 3: undo second diagonal pass
    auto grid2 = fillDiagonal(ciphertext, rows, cols);
    QString unDiag2;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            unDiag2.append(grid2[r][c]);

    // Reverse step 2: undo columnar transposition
    QString unTransposed = columnarReverse(unDiag2, order, rows, cols);

    // Reverse step 1: undo first diagonal pass
    auto grid1 = fillDiagonal(unTransposed, rows, cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            result.text.append(grid1[r][c]);

    double elapsed = timer.elapsed();
    m_stats.numDecrypts++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptDone(result.text.size(), elapsed);

    return result;
}

/* ---- Reset ---- */

void BazeleriesCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_keyword.clear();
}
