/**
 * @file TapirCode.cpp
 * @brief TapirCode 实现
 *
 * 实现Tapir密码：Polybius方阵分层、水平/垂直读出转位、文本编解码。
 */

#include "utils/code197/TapirCode.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TapirCode::TapirCode(QObject *parent) : QObject(parent)
{
    buildAlphaSet();
    buildPolybiusGrid();
}

TapirCode::~TapirCode() = default;

/* ---- Configuration ---- */

void TapirCode::setGridSize(int size) { m_gridSize = qBound(2, size, 10); }
void TapirCode::setKeyword(const QString& kw) { m_keyword = kw.toUpper(); buildAlphaSet(); buildPolybiusGrid(); }
void TapirCode::setTranspositionMode(TranspositionMode mode) { m_mode = mode; }
void TapirCode::setColumnKey(const QString& key) { m_columnKey = key.toUpper(); }

/* ---- Build alphabet set ---- */

void TapirCode::buildAlphaSet()
{
    // Standard: A-Z with J merged into I for 5x5 grid
    QString base;
    for (char c = 'A'; c <= 'Z'; ++c) base.append(c);

    // Remove duplicates from keyword first
    QString seen;
    for (QChar ch : m_keyword) {
        if (ch == 'J') ch = 'I';
        if (!seen.contains(ch) && base.contains(ch))
            seen.append(ch);
    }

    // Append remaining alphabet
    for (QChar ch : base) {
        if (ch == 'J') continue;  // merge J into I
        if (!seen.contains(ch))
            seen.append(ch);
    }

    m_alphaSet.clear();
    for (QChar ch : seen) m_alphaSet.append(ch);
}

/* ---- Build Polybius grid ---- */

QVector<QVector<QChar>> TapirCode::buildPolybiusGrid() const
{
    QVector<QVector<QChar>> grid(m_gridSize);
    int idx = 0;
    for (int r = 0; r < m_gridSize; ++r) {
        grid[r].resize(m_gridSize);
        for (int c = 0; c < m_gridSize; ++c) {
            if (idx < m_alphaSet.size())
                grid[r][c] = m_alphaSet[idx++];
            else
                grid[r][c] = QChar('?');
        }
    }
    m_grid = grid;
    return grid;
}

/* ---- Find in grid ---- */

QPair<int, int> TapirCode::findInGrid(QChar ch) const
{
    ch = ch.toUpper();
    if (ch == 'J') ch = 'I';
    for (int r = 0; r < m_gridSize; ++r)
        for (int c = 0; c < m_gridSize; ++c)
            if (m_grid[r][c] == ch) return {r, c};
    return {-1, -1};
}

/* ---- Column order ---- */

QVector<int> TapirCode::columnOrder() const
{
    int n = m_columnKey.isEmpty() ? m_gridSize : m_columnKey.size();
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;

    if (!m_columnKey.isEmpty()) {
        // Sort by key character order
        QVector<QPair<QChar, int>> pairs;
        for (int i = 0; i < n; ++i)
            pairs.append({m_columnKey[i], i});
        std::stable_sort(pairs.begin(), pairs.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        for (int i = 0; i < n; ++i)
            order[i] = pairs[i].second;
    }
    return order;
}

/* ---- Transpose ---- */

QVector<int> TapirCode::transpose(const QVector<int>& indices, int width) const
{
    int n = indices.size();
    if (n == 0 || width <= 0) return indices;
    int rows = qCeil(static_cast<double>(n) / width);

    QVector<int> result;
    result.reserve(n);

    switch (m_mode) {
    case HorizontalRead:
        // Read row by row
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < width; ++c) {
                int idx = r * width + c;
                if (idx < n) result.append(indices[idx]);
            }
        break;
    case VerticalRead:
        // Read column by column (using column key order)
        for (int c : columnOrder())
            for (int r = 0; r < rows; ++r) {
                int idx = r * width + c;
                if (idx < n) result.append(indices[idx]);
            }
        break;
    case DiagonalRead:
        // Read diagonals
        for (int d = 0; d < rows + width - 1; ++d)
            for (int r = 0; r < rows; ++r) {
                int c = d - r;
                if (c >= 0 && c < width) {
                    int idx = r * width + c;
                    if (idx < n) result.append(indices[idx]);
                }
            }
        break;
    case SpiralRead: {
        // Spiral outward from center
        int total = n;
        int r = rows / 2, c = width / 2;
        QVector<bool> visited(n, false);
        int dx = 0, dy = 1, steps = 1, stepCount = 0, turns = 0;
        for (int i = 0; i < total; ++i) {
            int idx = r * width + c;
            if (r >= 0 && r < rows && c >= 0 && c < width && idx < n && !visited[idx]) {
                result.append(indices[idx]);
                visited[idx] = true;
            }
            r += dx; c += dy;
            stepCount++;
            if (stepCount >= steps) {
                stepCount = 0;
                int tmp = dx; dx = -dy; dy = tmp;
                turns++;
                if (turns % 2 == 0) steps++;
            }
        }
        break;
    }
    }
    return result;
}

/* ---- Reverse transpose ---- */

QVector<int> TapirCode::reverseTranspose(const QVector<int>& indices, int width) const
{
    int n = indices.size();
    if (n == 0 || width <= 0) return indices;
    int rows = qCeil(static_cast<double>(n) / width);

    // Build the forward order map
    QVector<int> forwardOrder;
    switch (m_mode) {
    case HorizontalRead:
        for (int i = 0; i < n; ++i) forwardOrder.append(i);
        break;
    case VerticalRead:
        for (int c : columnOrder())
            for (int r = 0; r < rows; ++r) {
                int idx = r * width + c;
                if (idx < n) forwardOrder.append(idx);
            }
        break;
    case DiagonalRead:
        for (int d = 0; d < rows + width - 1; ++d)
            for (int r = 0; r < rows; ++r) {
                int c = d - r;
                if (c >= 0 && c < width) {
                    int idx = r * width + c;
                    if (idx < n) forwardOrder.append(idx);
                }
            }
        break;
    case SpiralRead:
        forwardOrder = transpose(QVector<int>(n, 0), width);
        for (int i = 0; i < n; ++i) forwardOrder[i] = i;
        break;
    }

    // Invert the permutation
    QVector<int> result(n);
    for (int i = 0; i < forwardOrder.size() && i < n; ++i)
        result[forwardOrder[i]] = indices[i];
    return result;
}

/* ---- Encode ---- */

QString TapirCode::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Fractionate via Polybius grid
    QString upper = plaintext.toUpper();
    QVector<int> fractionated;
    for (QChar ch : upper) {
        if (ch.isLetter()) {
            auto pos = findInGrid(ch);
            if (pos.first >= 0) {
                fractionated.append(pos.first);
                fractionated.append(pos.second);
            }
        }
    }

    // Step 2: Transpose
    int width = m_gridSize;
    auto transposed = transpose(fractionated, width);

    // Step 3: Re-encode back to letters using grid
    QString result;
    for (int i = 0; i + 1 < transposed.size(); i += 2) {
        int r = transposed[i], c = transposed[i + 1];
        if (r >= 0 && r < m_gridSize && c >= 0 && c < m_gridSize)
            result.append(m_grid[r][c]);
    }

    m_stats.totalOperations++;
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encode", plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QString TapirCode::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString upper = ciphertext.toUpper();

    // Step 1: Fractionate ciphertext via grid
    QVector<int> fractionated;
    for (QChar ch : upper) {
        auto pos = findInGrid(ch);
        if (pos.first >= 0) {
            fractionated.append(pos.first);
            fractionated.append(pos.second);
        }
    }

    // Step 2: Reverse transpose
    int width = m_gridSize;
    auto recovered = reverseTranspose(fractionated, width);

    // Step 3: Recover original letters
    QString result;
    for (int i = 0; i + 1 < recovered.size(); i += 2) {
        int r = recovered[i], c = recovered[i + 1];
        if (r >= 0 && r < m_gridSize && c >= 0 && c < m_gridSize)
            result.append(m_grid[r][c]);
    }

    m_stats.totalOperations++;
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decode", ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void TapirCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
