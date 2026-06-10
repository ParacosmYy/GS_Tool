/**
 * @file MorbitCode7.cpp
 * @brief MorbitCode7 实现
 *
 * 实现Morbit密码：扩展4x4网格与关键词驱动排列的增强紧凑视觉编码。
 */

#include "utils/code283/MorbitCode7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MorbitCode7::MorbitCode7(QObject *parent)
    : QObject(parent)
{
    initSymbols();
}

MorbitCode7::~MorbitCode7() = default;

/* ---- Configuration ---- */

void MorbitCode7::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper().trimmed();
}

void MorbitCode7::setGridSize(int size)
{
    m_gridRows = qBound(2, size, 8);
    m_gridCols = m_gridRows;
    m_stats.gridSize = m_gridRows * m_gridCols;
}

/* ---- Initialize symbol set ---- */

void MorbitCode7::initSymbols()
{
    m_symbols.clear();
    // Extended symbol set for 4x4 grid (16 symbols)
    QChar base[] = {
        QLatin1Char('A'), QLatin1Char('B'), QLatin1Char('C'), QLatin1Char('D'),
        QLatin1Char('E'), QLatin1Char('F'), QLatin1Char('G'), QLatin1Char('H'),
        QLatin1Char('I'), QLatin1Char('J'), QLatin1Char('K'), QLatin1Char('L'),
        QLatin1Char('M'), QLatin1Char('N'), QLatin1Char('O'), QLatin1Char('P')
    };
    for (auto ch : base)
        m_symbols.append(ch);
}

/* ---- Keyword permutation ---- */

QVector<int> MorbitCode7::keywordPermutation(const QString& kw) const
{
    int n = m_gridRows * m_gridCols;
    QString key = kw.isEmpty() ? QString("MORBIT") : kw;

    // Create (char, original_index) pairs
    QVector<QPair<QChar, int>> pairs;
    pairs.reserve(key.size());
    for (int i = 0; i < key.size(); ++i)
        pairs.append(qMakePair(key[i], i));

    // Sort by character, then by original position (stable)
    std::stable_sort(pairs.begin(), pairs.end(),
        [](const QPair<QChar, int>& a, const QPair<QChar, int>& b) {
            return a.first < b.first;
        });

    // Build permutation: position in sorted order
    QVector<int> perm(key.size());
    for (int i = 0; i < pairs.size(); ++i)
        perm[pairs[i].second] = i;

    // Extend permutation to grid size by repeating pattern
    QVector<int> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = perm[i % key.size()] + (i / key.size()) * key.size();

    // Normalize to [0, n)
    int maxVal = *std::max_element(result.begin(), result.end());
    if (maxVal >= n) {
        for (int i = 0; i < n; ++i)
            result[i] = result[i] % n;
    }

    return result;
}

/* ---- Build 4x4 grid ---- */

QVector<MorbitCode7::GridCell> MorbitCode7::buildGrid() const
{
    int n = m_gridRows * m_gridCols;
    QVector<GridCell> grid(n);
    QVector<int> perm = keywordPermutation(m_keyword);

    for (int r = 0; r < m_gridRows; ++r) {
        for (int c = 0; c < m_gridCols; ++c) {
            int idx = r * m_gridCols + c;
            grid[idx].row = r;
            grid[idx].col = c;
            grid[idx].symbol = m_symbols[idx % m_symbols.size()];
            grid[idx].permIndex = (idx < perm.size()) ? perm[idx] : idx;
        }
    }
    return grid;
}

/* ---- Map to cell ---- */

MorbitCode7::GridCell MorbitCode7::mapToCell(int row, int col, int permIdx) const
{
    GridCell cell;
    cell.row = row;
    cell.col = col;
    int idx = row * m_gridCols + col;
    cell.symbol = m_symbols[idx % m_symbols.size()];
    cell.permIndex = permIdx;
    return cell;
}

/* ---- Text to digit pairs ---- */

QVector<int> MorbitCode7::textToDigits(const QString& text) const
{
    QVector<int> digits;
    int n = text.size();
    for (int i = 0; i < n; ++i) {
        int val = text[i].unicode() % (m_gridRows * m_gridCols);
        digits.append(val);
    }
    return digits;
}

/* ---- Digits to text ---- */

QString MorbitCode7::digitsToText(const QVector<int>& digits) const
{
    QString result;
    int total = m_gridRows * m_gridCols;
    for (int d : digits) {
        int idx = d % total;
        result.append(m_symbols[idx % m_symbols.size()]);
    }
    return result;
}

/* ---- Encode ---- */

MorbitCode7::MorbitResult MorbitCode7::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    MorbitResult result;
    int n = plaintext.size();
    if (n == 0) return result;

    result.inputLength = n;
    QVector<int> perm = keywordPermutation(m_keyword);
    QVector<GridCell> grid = buildGrid();

    // Permute plaintext via keyword-driven grid
    QString encoded;
    for (int i = 0; i < n; ++i) {
        int gridIdx = i % (m_gridRows * m_gridCols);
        int permIdx = perm[gridIdx % perm.size()];

        // XOR-like mixing: combine char with grid position
        int mixed = (plaintext[i].unicode() + permIdx * 7) % 65536;
        QChar encodedChar(mixed);
        encoded.append(encodedChar);

        GridCell cell = mapToCell(gridIdx / m_gridCols,
                                  gridIdx % m_gridCols, permIdx);
        result.gridLayout.append(cell);
    }

    result.output = encoded;
    result.gridCells = result.gridLayout.size();

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodeDone(n, result.gridCells, elapsed);

    return result;
}

/* ---- Decode ---- */

MorbitCode7::MorbitResult MorbitCode7::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    MorbitResult result;
    int n = ciphertext.size();
    if (n == 0) return result;

    result.inputLength = n;
    QVector<int> perm = keywordPermutation(m_keyword);

    // Reverse the XOR-like mixing
    QString decoded;
    for (int i = 0; i < n; ++i) {
        int gridIdx = i % (m_gridRows * m_gridCols);
        int permIdx = perm[gridIdx % perm.size()];

        int original = (ciphertext[i].unicode() - permIdx * 7) % 65536;
        if (original < 0) original += 65536;
        decoded.append(QChar(original));
    }

    result.output = decoded;
    result.gridLayout = buildGrid();
    result.gridCells = result.gridLayout.size();

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Reset ---- */

void MorbitCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
