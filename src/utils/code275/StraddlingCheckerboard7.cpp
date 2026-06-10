/**
 * @file StraddlingCheckerboard7.cpp
 * @brief StraddlingCheckerboard7 实现
 *
 * 实现跨越棋盘编码：扩展标签集与分数定位的增强型非标准编码。
 */

#include "utils/code275/StraddlingCheckerboard7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard7::StraddlingCheckerboard7(QObject *parent)
    : QObject(parent)
{
    // Default configuration: classic 2-row checkerboard
    m_config.rows = 2;
    m_config.cols = 10;
    m_config.rowSlots = {10, 10};
    m_config.labels.fill(-1, 20);
    setAlphabetSize(26);
}

StraddlingCheckerboard7::~StraddlingCheckerboard7() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard7::setBoardConfig(const BoardConfig& config)
{
    m_config = config;
    int total = 0;
    for (int s : m_config.rowSlots) total += s;
    m_config.labels.resize(total);
    buildTables();
}

void StraddlingCheckerboard7::setAlphabetSize(int size)
{
    m_alphabetSize = qBound(2, size, 256);
    buildTables();
}

/* ---- Build encode/decode tables ---- */

void StraddlingCheckerboard7::buildTables()
{
    // Initialize decode table from board config
    m_decodeTable.clear();
    m_decodeTable.resize(m_config.rows);
    for (int r = 0; r < m_config.rows; ++r) {
        m_decodeTable[r].resize(m_config.cols);
        m_decodeTable[r].fill(-1);
    }

    m_encodeTable.resize(m_alphabetSize);
    for (auto& entry : m_encodeTable) entry = {-1, -1};

    // Assign symbols to board cells (straddling pattern)
    int symbol = 0;
    int totalCells = 0;
    for (int s : m_config.rowSlots) totalCells += s;

    for (int r = 0; r < m_config.rows && symbol < m_alphabetSize; ++r) {
        int slots = (r < m_config.rowSlots.size()) ? m_config.rowSlots[r] : m_config.cols;
        for (int c = 0; c < slots && symbol < m_alphabetSize; ++c) {
            // Check if cell is a gap (straddling pattern)
            bool isGap = false;
            // Gaps at specific positions for straddling effect
            if (r == 0 && (c == 0)) isGap = true;  // Row 0, position 0 is gap
            if (r == 1 && (c >= 8)) isGap = true;   // Row 1, positions 8-9 are gaps

            if (!isGap) {
                m_config.labels[totalCells] = symbol;
                m_decodeTable[r][c] = symbol;
                if (symbol < m_encodeTable.size())
                    m_encodeTable[symbol] = {r, c};
                symbol++;
                totalCells++;
            }
        }
    }

    // Fill remaining symbols with extended label set (fractional positions)
    while (symbol < m_alphabetSize) {
        int r = symbol % m_config.rows;
        int c = (symbol / m_config.rows) % m_config.cols;
        m_encodeTable[symbol] = {r, c};
        symbol++;
    }
}

/* ---- Compute fractional position ---- */

StraddlingCheckerboard7::FractionalPos
StraddlingCheckerboard7::computeFractional(int symbol, int occurrence) const
{
    FractionalPos pos;
    if (symbol < 0 || symbol >= m_encodeTable.size()) return pos;

    pos.row = m_encodeTable[symbol][0];
    pos.col = m_encodeTable[symbol][1];

    // Fractional positioning: spread occurrences within the cell
    // using golden ratio for quasi-random distribution
    const double phi = 0.6180339887;
    pos.fraction = qFmod(occurrence * phi, 1.0);
    return pos;
}

/* ---- Resolve fractional position to symbol ---- */

int StraddlingCheckerboard7::resolveFractional(const FractionalPos& pos) const
{
    if (pos.row < 0 || pos.row >= m_decodeTable.size()) return -1;
    if (pos.col < 0 || pos.col >= m_decodeTable[pos.row].size()) return -1;
    return m_decodeTable[pos.row][pos.col];
}

/* ---- Count active cells ---- */

int StraddlingCheckerboard7::countActiveCells() const
{
    int count = 0;
    for (const auto& row : m_decodeTable)
        for (int s : row)
            if (s >= 0) count++;
    return qMax(1, count);
}

/* ---- Encode message ---- */

QVector<StraddlingCheckerboard7::FractionalPos>
StraddlingCheckerboard7::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    QVector<FractionalPos> result;
    QMap<int, int> occurrenceCount;  // Track occurrences per symbol

    for (int sym : message) {
        if (sym < 0 || sym >= m_alphabetSize) continue;
        int occ = occurrenceCount.value(sym, 0);
        result.append(computeFractional(sym, occ));
        occurrenceCount[sym] = occ + 1;
    }

    double elapsed = timer.elapsed();
    int activeCells = countActiveCells();
    double ratio = (message.size() > 0)
                       ? static_cast<double>(result.size() * 2) / (message.size() * qCeil(qLn(m_alphabetSize) / qLn(2)))
                       : 0.0;

    m_stats.numEncoded += message.size();
    m_stats.boardSize = activeCells;
    m_stats.compressionRatio = ratio;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodingDone(message.size(), activeCells, ratio, elapsed);

    return result;
}

/* ---- Decode positions ---- */

QVector<int> StraddlingCheckerboard7::decode(const QVector<FractionalPos>& positions)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    for (const auto& pos : positions) {
        int sym = resolveFractional(pos);
        if (sym >= 0) result.append(sym);
    }

    double elapsed = timer.elapsed();
    m_stats.numDecoded += result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Get board config ---- */

StraddlingCheckerboard7::BoardConfig StraddlingCheckerboard7::boardConfig() const
{
    return m_config;
}

/* ---- Optimize layout for frequency distribution ---- */

void StraddlingCheckerboard7::optimizeLayout(const QVector<double>& frequencies)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(frequencies.size(), m_alphabetSize);

    // Sort symbols by frequency (descending)
    QVector<QPair<double, int>> sorted;
    for (int i = 0; i < n; ++i) sorted.append({frequencies[i], i});
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Reassign: highest frequency symbols get smallest coordinate sums (row+col)
    QVector<QPair<int, int>> positions;
    for (int r = 0; r < m_config.rows; ++r)
        for (int c = 0; c < m_config.cols; ++c)
            positions.append({r, c});

    std::sort(positions.begin(), positions.end(),
              [](const auto& a, const auto& b) { return a.first + a.second < b.first + b.second; });

    // Rebuild tables with optimized assignment
    m_decodeTable.clear();
    m_decodeTable.resize(m_config.rows);
    for (int r = 0; r < m_config.rows; ++r) {
        m_decodeTable[r].resize(m_config.cols);
        m_decodeTable[r].fill(-1);
    }
    m_encodeTable.resize(m_alphabetSize);
    for (auto& e : m_encodeTable) e = {-1, -1};

    for (int i = 0; i < qMin(sorted.size(), positions.size()); ++i) {
        int sym = sorted[i].second;
        int r = positions[i].first;
        int c = positions[i].second;
        m_encodeTable[sym] = {r, c};
        m_decodeTable[r][c] = sym;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Reset ---- */

void StraddlingCheckerboard7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
