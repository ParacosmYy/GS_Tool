/**
 * @file BazeleriesCode8.cpp
 * @brief BazeleriesCode8 实现
 *
 * 实现Bazeleries密码：交错关键字转置与旋转矩形网格实现嵌套双层加密。
 */

#include "utils/code299/BazeleriesCode8.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode8::BazeleriesCode8(QObject *parent)
    : QObject(parent) {}

BazeleriesCode8::~BazeleriesCode8() = default;

/* ---- Configuration ---- */

void BazeleriesCode8::setKeyword(const QString& keyword) { m_keyword = keyword.toUpper(); }
void BazeleriesCode8::setGridRotation(int degrees)
{
    // Normalize to 0, 90, 180, 270
    int normalized = ((degrees % 360) + 360) % 360;
    m_rotation = (normalized / 90) * 90;
}

/* ---- Keyword-based permutation ---- */

QVector<int> BazeleriesCode8::keywordPermutation(const QString& keyword) const
{
    int n = keyword.size();
    if (n == 0) return {0};

    // Create indexed pairs (character, original position)
    QVector<QPair<QChar, int>> indexed;
    indexed.reserve(n);
    for (int i = 0; i < n; ++i)
        indexed.append({keyword[i], i});

    // Sort by character, then by position for stability
    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) {
                  if (a.first != b.first) return a.first < b.first;
                  return a.second < b.second;
              });

    // Build permutation: perm[i] = new position of original column i
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i)
        perm[indexed[i].second] = i;

    return perm;
}

/* ---- Layer 1: Interleaved keyword columnar transposition ---- */

QString BazeleriesCode8::keywordTranspose(const QString& text,
                                           const QVector<int>& perm,
                                           bool encrypt) const
{
    int cols = perm.size();
    int rows = (text.size() + cols - 1) / cols;

    if (encrypt) {
        // Write text into grid row by row, read column by column in perm order
        QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
        for (int i = 0; i < text.size(); ++i)
            grid[i / cols][i % cols] = text[i];

        // Read columns in keyword permutation order
        // Interleave: alternate between first and second half of perm
        QString result;
        int half = cols / 2;
        for (int r = 0; r < rows; ++r) {
            // First half of perm columns forward
            for (int i = 0; i < half; ++i)
                result.append(grid[r][perm[i]]);
            // Second half of perm columns interleaved
            for (int i = half; i < cols; ++i)
                result.append(grid[r][perm[i]]);
        }
        return result;
    } else {
        // Reverse: read interleaved order, write back in original order
        QString padded = text;
        while (padded.size() < rows * cols) padded.append('X');

        QVector<QVector<QChar>> grid(rows, QVector<QChar>(cols, QChar('X')));
        int pos = 0;
        for (int r = 0; r < rows && pos < padded.size(); ++r) {
            for (int i = 0; i < cols && pos < padded.size(); ++i)
                grid[r][perm[i]] = padded[pos++];
        }

        // Read row by row
        QString result;
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                if (grid[r][c] != QChar('X'))
                    result.append(grid[r][c]);
        return result;
    }
}

/* ---- Compute grid dimensions ---- */

void BazeleriesCode8::computeGridDims(int length, int& rows, int& cols) const
{
    // Find a rectangular grid close to square
    int side = static_cast<int>(qSqrt(qMax(1, length)));
    rows = side;
    cols = (length + rows - 1) / rows;
    if (rows * cols < length) rows++;
}

/* ---- Fill rotated rectangular grid ---- */

BazeleriesCode8::GridState BazeleriesCode8::fillGrid(const QString& text, int rows, int cols) const
{
    GridState grid;
    grid.rows = rows;
    grid.cols = cols;
    grid.rotation = m_rotation;

    grid.cells.resize(rows);
    for (auto& row : grid.cells)
        row.resize(cols, QChar('X'));

    int pos = 0;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols && pos < text.size(); ++c)
            grid.cells[r][c] = text[pos++];

    return grid;
}

/* ---- Read text from rotated grid ---- */

QString BazeleriesCode8::readGrid(const GridState& grid) const
{
    QString result;
    int r = grid.rows, c = grid.cols;

    // Read in rotated order: 90° clockwise reads columns right-to-left top-to-bottom
    switch (grid.rotation % 360) {
    case 0:
        for (int row = 0; row < r; ++row)
            for (int col = 0; col < c; ++col)
                result.append(grid.cells[row][col]);
        break;
    case 90:
        for (int col = c - 1; col >= 0; --col)
            for (int row = 0; row < r; ++row)
                result.append(grid.cells[row][col]);
        break;
    case 180:
        for (int row = r - 1; row >= 0; --row)
            for (int col = c - 1; col >= 0; --col)
                result.append(grid.cells[row][col]);
        break;
    case 270:
        for (int col = 0; col < c; ++col)
            for (int row = r - 1; row >= 0; --row)
                result.append(grid.cells[row][col]);
        break;
    }
    return result;
}

/* ---- Layer 2: Rotated rectangular grid transposition ---- */

QString BazeleriesCode8::gridTranspose(const QString& text, bool encrypt) const
{
    int rows, cols;
    computeGridDims(text.size(), rows, cols);

    if (encrypt) {
        // Fill grid normally, read with rotation
        GridState grid = fillGrid(text, rows, cols);
        return readGrid(grid);
    } else {
        // Inverse: fill in rotated order, read normally
        GridState grid;
        grid.rows = rows;
        grid.cols = cols;
        grid.rotation = 0;
        grid.cells.resize(rows);
        for (auto& row : grid.cells)
            row.resize(cols, QChar('X'));

        // Fill in the rotation order (inverse reading direction)
        int pos = 0;
        int r = rows, c = cols;
        switch (m_rotation % 360) {
        case 0:
            for (int row = 0; row < r && pos < text.size(); ++row)
                for (int col = 0; col < c && pos < text.size(); ++col)
                    grid.cells[row][col] = text[pos++];
            break;
        case 90:
            for (int col = c - 1; col >= 0 && pos < text.size(); --col)
                for (int row = 0; row < r && pos < text.size(); ++row)
                    grid.cells[row][col] = text[pos++];
            break;
        case 180:
            for (int row = r - 1; row >= 0 && pos < text.size(); --row)
                for (int col = c - 1; col >= 0 && pos < text.size(); --col)
                    grid.cells[row][col] = text[pos++];
            break;
        case 270:
            for (int col = 0; col < c && pos < text.size(); ++col)
                for (int row = r - 1; row >= 0 && pos < text.size(); --row)
                    grid.cells[row][col] = text[pos++];
            break;
        }

        // Read row by row (normal order)
        QString result;
        for (int row = 0; row < r; ++row)
            for (int col = 0; col < c; ++col)
                if (grid.cells[row][col] != QChar('X'))
                    result.append(grid.cells[row][col]);
        return result;
    }
}

/* ---- Encrypt: nested double-layer ---- */

BazeleriesCode8::CipherResult BazeleriesCode8::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    if (plaintext.isEmpty() || m_keyword.isEmpty()) {
        result.success = false;
        return result;
    }

    // Layer 1: Keyword interleaved transposition
    QVector<int> perm = keywordPermutation(m_keyword);
    QString layer1 = keywordTranspose(plaintext.toUpper(), perm, true);

    // Layer 2: Rotated rectangular grid transposition
    QString layer2 = gridTranspose(layer1, true);

    result.output = layer2;
    computeGridDims(layer1.size(), result.gridRows, result.gridCols);
    result.rotations = m_rotation;
    result.success = true;

    m_stats.totalEncryptions++;
    m_stats.lastInputLength = plaintext.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit encryptDone(plaintext.size(), elapsed);
    return result;
}

/* ---- Decrypt: reverse nested double-layer ---- */

BazeleriesCode8::CipherResult BazeleriesCode8::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    if (ciphertext.isEmpty() || m_keyword.isEmpty()) {
        result.success = false;
        return result;
    }

    // Reverse Layer 2: Grid transposition inverse
    QString layer1 = gridTranspose(ciphertext.toUpper(), false);

    // Reverse Layer 1: Keyword transposition inverse
    QVector<int> perm = keywordPermutation(m_keyword);
    QString original = keywordTranspose(layer1, perm, false);

    result.output = original;
    computeGridDims(layer1.size(), result.gridRows, result.gridCols);
    result.rotations = m_rotation;
    result.success = true;

    m_stats.totalDecryptions++;
    m_stats.lastInputLength = ciphertext.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptDone(ciphertext.size(), elapsed);
    return result;
}

/* ---- Reset ---- */

void BazeleriesCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
