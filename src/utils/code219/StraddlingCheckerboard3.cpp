/**
 * @file StraddlingCheckerboard3.cpp
 * @brief StraddlingCheckerboard3 实现
 *
 * 实现跨越棋盘格编码：变宽行布局、模拟退火优化、编码/解码。
 */

#include "utils/code219/StraddlingCheckerboard3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard3::StraddlingCheckerboard3(QObject *parent) : QObject(parent) {}
StraddlingCheckerboard3::~StraddlingCheckerboard3() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard3::setParameters(int numRows, int alphabetSize)
{
    m_numRows = qMax(2, numRows);
    m_alphabetSize = qMax(2, alphabetSize);
    m_cellsPerRow = (m_alphabetSize + m_numRows - 1) / m_numRows;
    buildDefaultLayout();
    buildDecodeTable();
}

/* ---- Layout building ---- */

void StraddlingCheckerboard3::buildDefaultLayout()
{
    m_rows.resize(m_numRows);
    int symbolsLeft = m_alphabetSize;
    int symbol = 0;

    for (int r = 0; r < m_numRows; ++r) {
        // Variable-width: first rows get more cells
        int width = qMin(m_cellsPerRow + (r == 0 ? 1 : 0), symbolsLeft);
        m_rows[r].offset = r * m_cellsPerRow;
        m_rows[r].width = width;
        m_rows[r].symbolMap.resize(width);
        for (int c = 0; c < width; ++c)
            m_rows[r].symbolMap[c] = symbol++;
        symbolsLeft -= width;
        if (symbolsLeft <= 0) break;
    }

    // Map each symbol to its row
    m_symbolToRow.resize(m_alphabetSize);
    for (int r = 0; r < m_rows.size(); ++r)
        for (int c = 0; c < m_rows[r].width; ++c)
            m_symbolToRow[m_rows[r].symbolMap[c]] = r;
}

void StraddlingCheckerboard3::buildDecodeTable()
{
    int total = totalCells();
    m_decodeTable.resize(total);
    for (int i = 0; i < total; ++i) m_decodeTable[i] = -1;

    for (int r = 0; r < m_rows.size(); ++r) {
        for (int c = 0; c < m_rows[r].width; ++c) {
            int idx = m_rows[r].offset + c;
            if (idx < total) m_decodeTable[idx] = m_rows[r].symbolMap[c];
        }
    }
}

int StraddlingCheckerboard3::totalCells() const
{
    int t = 0;
    for (auto& r : m_rows) t += r.width;
    return t;
}

bool StraddlingCheckerboard3::validateLayout() const
{
    QVector<bool> used(m_alphabetSize, false);
    for (auto& row : m_rows) {
        for (int s : row.symbolMap) {
            if (s < 0 || s >= m_alphabetSize || used[s]) return false;
            used[s] = true;
        }
    }
    for (bool u : used) if (!u) return false;
    return true;
}

/* ---- Cost function ---- */

double StraddlingCheckerboard3::computeCost(const QVector<RowConfig>& config) const
{
    // Cost = max run length + balance penalty
    int maxRun = 0;
    double balance = 0.0;
    double avgWidth = 0.0;
    for (auto& row : config) avgWidth += row.width;
    avgWidth /= config.size();

    for (auto& row : config) {
        balance += qAbs(row.width - avgWidth);
        // Run: consecutive symbols mapped to same row
        int run = 1;
        for (int i = 1; i < row.symbolMap.size(); ++i) {
            if (row.symbolMap[i] == row.symbolMap[i - 1] + 1) run++;
            else { maxRun = qMax(maxRun, run); run = 1; }
        }
        maxRun = qMax(maxRun, run);
    }
    return maxRun + 0.5 * balance;
}

/* ---- SA neighbor ---- */

QVector<StraddlingCheckerboard3::RowConfig>
StraddlingCheckerboard3::neighborSolution(const QVector<RowConfig>& config) const
{
    QVector<RowConfig> neighbor = config;
    if (neighbor.size() < 2) return neighbor;

    // Swap two random symbols between rows
    int r1 = qrand() % neighbor.size();
    int r2 = qrand() % neighbor.size();
    while (r2 == r1) r2 = qrand() % neighbor.size();

    if (neighbor[r1].symbolMap.isEmpty() || neighbor[r2].symbolMap.isEmpty())
        return neighbor;

    int c1 = qrand() % neighbor[r1].symbolMap.size();
    int c2 = qrand() % neighbor[r2].symbolMap.size();
    std::swap(neighbor[r1].symbolMap[c1], neighbor[r2].symbolMap[c2]);
    return neighbor;
}

/* ---- Simulated annealing ---- */

void StraddlingCheckerboard3::optimizeLayout(int maxIterations,
                                               double initTemp, double coolingRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<RowConfig> bestLayout = m_rows;
    double bestCost = computeCost(m_rows);
    double currentCost = bestCost;
    double temp = initTemp;

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector<RowConfig> candidate = neighborSolution(m_rows);
        double candCost = computeCost(candidate);
        double delta = candCost - currentCost;

        if (delta < 0 || (temp > 0 && qrand() / double(RAND_MAX) < qExp(-delta / temp))) {
            m_rows = candidate;
            currentCost = candCost;
            if (currentCost < bestCost) {
                bestLayout = candidate;
                bestCost = currentCost;
            }
        }
        temp *= coolingRate;
    }

    m_rows = bestLayout;
    buildDecodeTable();

    m_stats.saIterations = maxIterations;
    m_stats.saTemperature = temp;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit layoutOptimized(maxIterations, bestCost);
}

/* ---- Encode ---- */

QVector<int> StraddlingCheckerboard3::encode(const QVector<int>& symbols) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    result.reserve(symbols.size() * 2);

    for (int s : symbols) {
        if (s < 0 || s >= m_alphabetSize) continue;
        int row = m_symbolToRow[s];
        // Find column in row
        int col = 0;
        for (int c = 0; c < m_rows[row].width; ++c) {
            if (m_rows[row].symbolMap[c] == s) { col = c; break; }
        }
        // Output: row index + cell position
        result.append(row);
        result.append(col);
    }

    m_stats.inputLength = symbols.size();
    m_stats.outputLength = result.size();
    m_stats.numRows = m_rows.size();
    m_stats.compressionRatio = symbols.isEmpty() ? 0.0
        : double(result.size()) / symbols.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodingCompleted(symbols.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QVector<int> StraddlingCheckerboard3::decode(const QVector<int>& positions) const
{
    QVector<int> result;
    for (int i = 0; i + 1 < positions.size(); i += 2) {
        int row = positions[i];
        int col = positions[i + 1];
        if (row < 0 || row >= m_rows.size()) continue;
        if (col < 0 || col >= m_rows[row].width) continue;
        result.append(m_rows[row].symbolMap[col]);
    }
    return result;
}

/* ---- Accessors ---- */

QVector<StraddlingCheckerboard3::RowConfig> StraddlingCheckerboard3::layout() const
{
    return m_rows;
}

int StraddlingCheckerboard3::maxRunLength() const
{
    int maxRun = 0;
    for (auto& row : m_rows) {
        int run = 1;
        for (int i = 1; i < row.symbolMap.size(); ++i) {
            if (row.symbolMap[i] == row.symbolMap[i - 1] + 1) run++;
            else { maxRun = qMax(maxRun, run); run = 1; }
        }
        maxRun = qMax(maxRun, run);
    }
    return maxRun;
}

/* ---- Reset ---- */

void StraddlingCheckerboard3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_rows.clear();
    m_decodeTable.clear();
    m_symbolToRow.clear();
}
