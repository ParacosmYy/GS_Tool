/**
 * @file GameOfLife.cpp
 * @brief 康威生命游戏实现
 */

#include "GameOfLife.h"
#include <QElapsedTimer>
#include <cstdlib>

GameOfLife::GameOfLife(int width, int height, QObject* parent)
    : QObject(parent)
    , m_width(qMax(1, width))
    , m_height(qMax(1, height))
    , m_generation(0)
    , m_grid(m_height, QVector<bool>(m_width, false))
    , m_nextGrid(m_height, QVector<bool>(m_width, false))
    , m_timeSum(0.0)
{
    m_birthRule = {3};
    m_survivalRule = {2, 3};
}

GameOfLife::PopulationStats GameOfLife::step()
{
    QElapsedTimer timer;
    timer.start();

    PopulationStats ps;
    int alive = 0, born = 0, died = 0;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int neighbors = countNeighbors(x, y);
            bool wasAlive = m_grid[y][x];

            if (wasAlive) {
                bool survives = m_survivalRule.contains(neighbors);
                m_nextGrid[y][x] = survives;
                if (!survives) died++;
                else alive++;
            } else {
                bool births = m_birthRule.contains(neighbors);
                m_nextGrid[y][x] = births;
                if (births) { born++; alive++; }
            }
        }
    }

    m_grid.swap(m_nextGrid);
    m_generation++;

    ps.alive = alive;
    ps.born = born;
    ps.died = died;
    ps.density = static_cast<double>(alive) / (m_width * m_height);
    m_population = ps;

    m_stats.totalSteps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSteps;

    emit stepped(m_generation, alive);
    return ps;
}

QVector<GameOfLife::PopulationStats> GameOfLife::stepN(int n)
{
    QVector<PopulationStats> stats;
    stats.reserve(n);
    for (int i = 0; i < n; ++i)
        stats.append(step());
    return stats;
}

void GameOfLife::setCell(int x, int y, bool alive)
{
    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
        m_grid[y][x] = alive;
}

bool GameOfLife::cell(int x, int y) const
{
    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
        return m_grid[y][x];
    return false;
}

void GameOfLife::loadPattern(Pattern pattern, int offsetX, int offsetY)
{
    QVector<QPoint> cells;
    switch (pattern) {
    case Blinker:
        cells = {{1,0},{1,1},{1,2}};
        break;
    case Glider:
        cells = {{1,0},{2,1},{0,2},{1,2},{2,2}};
        break;
    case LightweightShip:
        cells = {{1,0},{4,0},{0,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3}};
        break;
    case Pulsar:
        for (int dx : {2,3,4,8,9,10})
            for (int dy : {0}) cells.append({dx, dy});
        for (int dx : {0,5,7,12})
            for (int dy : {2,3,4}) cells.append({dx, dy});
        for (int dx : {2,3,4,8,9,10})
            for (int dy : {5}) cells.append({dx, dy});
        for (int dx : {2,3,4,8,9,10})
            for (int dy : {7}) cells.append({dx, dy});
        for (int dx : {0,5,7,12})
            for (int dy : {8,9,10}) cells.append({dx, dy});
        for (int dx : {2,3,4,8,9,10})
            for (int dy : {12}) cells.append({dx, dy});
        break;
    case GosperGun:
        cells = {{24,0},{22,1},{24,1},{12,2},{13,2},{20,2},{21,2},{34,2},{35,2},
                 {11,3},{15,3},{20,3},{21,3},{34,3},{35,3},{0,4},{1,4},{10,4},
                 {16,4},{20,4},{21,4},{0,5},{1,5},{10,5},{14,5},{16,5},{17,5},
                 {22,5},{24,5},{10,6},{16,6},{24,6},{11,7},{15,7},{12,8},{13,8}};
        break;
    case Rpentomino:
        cells = {{1,0},{2,0},{0,1},{1,1},{1,2}};
        break;
    }

    for (const auto& c : cells)
        setCell(c.x() + offsetX, c.y() + offsetY, true);
}

void GameOfLife::randomize(double density)
{
    for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x)
            m_grid[y][x] = (static_cast<double>(std::rand()) / RAND_MAX) < density;
    m_generation = 0;
}

void GameOfLife::clear()
{
    for (int y = 0; y < m_height; ++y)
        for (int x = 0; x < m_width; ++x)
            m_grid[y][x] = false;
    m_generation = 0;
    m_stats.totalReset++;
}

void GameOfLife::setRules(const QVector<int>& birth, const QVector<int>& survival)
{
    m_birthRule = birth;
    m_survivalRule = survival;
}

int GameOfLife::width() const { return m_width; }
int GameOfLife::height() const { return m_height; }
int GameOfLife::generation() const { return m_generation; }
GameOfLife::PopulationStats GameOfLife::population() const { return m_population; }

int GameOfLife::countNeighbors(int x, int y) const
{
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + m_width) % m_width;
            int ny = (y + dy + m_height) % m_height;
            if (m_grid[ny][nx]) count++;
        }
    }
    return count;
}

GameOfLife::Stats GameOfLife::stats() const { return m_stats; }

void GameOfLife::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
