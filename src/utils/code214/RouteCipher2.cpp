/**
 * @file RouteCipher2.cpp
 * @brief RouteCipher2 实现
 *
 * 实现路线密码：多路径网格遍历、遗传算法密钥搜索恢复。
 */

#include "utils/code214/RouteCipher2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

RouteCipher2::RouteCipher2(QObject *parent) : QObject(parent) {}
RouteCipher2::~RouteCipher2() = default;

/* ---- Configuration ---- */

void RouteCipher2::setParameters(int rows, int cols, int populationSize,
                                  int generations, double mutationRate)
{
    m_rows = qMax(2, rows);
    m_cols = qMax(2, cols);
    m_popSize = qMax(10, populationSize);
    m_gens = qMax(10, generations);
    m_mutRate = qBound(0.01, mutationRate, 0.5);
    m_stats.gridRows = m_rows;
    m_stats.gridCols = m_cols;
    m_stats.gridSize = m_rows * m_cols;
    m_stats.populationSize = m_popSize;
}

/* ---- Spiral inward traversal ---- */

QVector<int> RouteCipher2::spiralPath(int rows, int cols) const
{
    QVector<int> path;
    int top = 0, bottom = rows - 1, left = 0, right = cols - 1;
    while (top <= bottom && left <= right) {
        for (int c = left; c <= right; ++c) path.append(top * cols + c);
        ++top;
        for (int r = top; r <= bottom; ++r) path.append(r * cols + right);
        --right;
        if (top <= bottom) {
            for (int c = right; c >= left; --c) path.append(bottom * cols + c);
            --bottom;
        }
        if (left <= right) {
            for (int r = bottom; r >= top; --r) path.append(r * cols + left);
            ++left;
        }
    }
    return path;
}

/* ---- Zigzag row traversal ---- */

QVector<int> RouteCipher2::zigzagRowPath(int rows, int cols) const
{
    QVector<int> path;
    for (int r = 0; r < rows; ++r) {
        if (r % 2 == 0) {
            for (int c = 0; c < cols; ++c) path.append(r * cols + c);
        } else {
            for (int c = cols - 1; c >= 0; --c) path.append(r * cols + c);
        }
    }
    return path;
}

/* ---- Zigzag column traversal ---- */

QVector<int> RouteCipher2::zigzagColPath(int rows, int cols) const
{
    QVector<int> path;
    for (int c = 0; c < cols; ++c) {
        if (c % 2 == 0) {
            for (int r = 0; r < rows; ++r) path.append(r * cols + c);
        } else {
            for (int r = rows - 1; r >= 0; --r) path.append(r * cols + c);
        }
    }
    return path;
}

/* ---- Diagonal traversal ---- */

QVector<int> RouteCipher2::diagonalPath(int rows, int cols) const
{
    QVector<int> path;
    for (int d = 0; d < rows + cols - 1; ++d) {
        int r = qMin(d, rows - 1);
        int c = d - r;
        while (r >= 0 && c < cols) {
            path.append(r * cols + c);
            --r; ++c;
        }
    }
    return path;
}

/* ---- Generate path for type ---- */

QVector<int> RouteCipher2::generatePath(int rows, int cols, PathType path) const
{
    switch (path) {
    case SpiralInward: return spiralPath(rows, cols);
    case ZigzagRow:    return zigzagRowPath(rows, cols);
    case ZigzagCol:    return zigzagColPath(rows, cols);
    case Diagonal:     return diagonalPath(rows, cols);
    }
    return spiralPath(rows, cols);
}

/* ---- Encrypt ---- */

QString RouteCipher2::encrypt(const QString& plaintext, PathType path) const
{
    QElapsedTimer timer;
    timer.start();

    int size = m_rows * m_cols;
    QString padded = plaintext;
    while (padded.length() < size) padded += 'X';

    QVector<int> route = generatePath(m_rows, m_cols, path);
    QString result;
    result.reserve(size);
    for (int idx : route)
        result += (idx < padded.length()) ? padded[idx] : 'X';

    const_cast<RouteCipher2*>(this)->m_stats.totalOps++;
    const_cast<RouteCipher2*>(this)->m_timeSum += timer.elapsed();
    const_cast<RouteCipher2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<RouteCipher2*>(this)->encryptionCompleted(
        static_cast<int>(path), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString RouteCipher2::decrypt(const QString& ciphertext, PathType path) const
{
    QElapsedTimer timer;
    timer.start();

    int size = m_rows * m_cols;
    QVector<int> route = generatePath(m_rows, m_cols, path);
    QString result;
    result.resize(size, ' ');
    for (int i = 0; i < route.size() && i < ciphertext.length(); ++i)
        result[route[i]] = ciphertext[i];

    const_cast<RouteCipher2*>(this)->m_stats.totalOps++;
    const_cast<RouteCipher2*>(this)->m_timeSum += timer.elapsed();
    const_cast<RouteCipher2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- English bigram score ---- */

double RouteCipher2::englishScore(const QString& text) const
{
    // Simple monogram frequency scoring
    static const QString common = "ETAOINSHRDLCUMWFGYPBVKJXQZ";
    double score = 0.0;
    int len = text.length();
    if (len == 0) return 0.0;
    for (int i = 0; i < len; ++i) {
        QChar ch = text[i].toUpper();
        int idx = common.indexOf(ch);
        if (idx >= 0) score += (26.0 - idx) / 26.0;
        if (ch == ' ') score += 0.5;
    }
    return score / len;
}

/* ---- Fitness ---- */

double RouteCipher2::fitness(const QString& text, const QString& lang) const
{
    Q_UNUSED(lang)
    return englishScore(text);
}

/* ---- Tournament selection ---- */

int RouteCipher2::tournamentSelect(const QVector<double>& fitnesses) const
{
    int best = std::rand() % fitnesses.size();
    for (int t = 0; t < 3; ++t) {
        int challenger = std::rand() % fitnesses.size();
        if (fitnesses[challenger] > fitnesses[best]) best = challenger;
    }
    return best;
}

/* ---- Crossover ---- */

RouteCipher2::PathType RouteCipher2::crossover(PathType a, PathType b) const
{
    Q_UNUSED(b)
    // Simple: pick the better one (path space is only 4 values)
    return (std::rand() % 2 == 0) ? a : b;
}

/* ---- Recover key via GA ---- */

RouteCipher2::PathType RouteCipher2::recoverKey(const QString& ciphertext,
                                                  const QString& languageModel) const
{
    QElapsedTimer timer;
    timer.start();

    // Population: 4 path types, each with multiple individuals
    QVector<PathType> population(m_popSize);
    QVector<double> popFit(m_popSize, 0.0);

    for (int i = 0; i < m_popSize; ++i)
        population[i] = static_cast<PathType>(std::rand() % 4);

    PathType bestPath = SpiralInward;
    double bestFit = -1e30;

    for (int gen = 0; gen < m_gens; ++gen) {
        // Evaluate fitness
        for (int i = 0; i < m_popSize; ++i) {
            QString decrypted = decrypt(ciphertext, population[i]);
            popFit[i] = fitness(decrypted, languageModel);
            if (popFit[i] > bestFit) {
                bestFit = popFit[i];
                bestPath = population[i];
            }
        }

        // Selection and reproduction
        QVector<PathType> newPop(m_popSize);
        for (int i = 0; i < m_popSize; ++i) {
            int p1 = tournamentSelect(popFit);
            int p2 = tournamentSelect(popFit);
            newPop[i] = crossover(population[p1], population[p2]);
            // Mutation
            if ((std::rand() / static_cast<double>(RAND_MAX)) < m_mutRate)
                newPop[i] = static_cast<PathType>(std::rand() % 4);
        }
        population = newPop;
    }

    const_cast<RouteCipher2*>(this)->m_stats.generations = m_gens;
    const_cast<RouteCipher2*>(this)->m_stats.bestFitness = bestFit;
    const_cast<RouteCipher2*>(this)->m_stats.totalOps++;
    const_cast<RouteCipher2*>(this)->m_timeSum += timer.elapsed();
    const_cast<RouteCipher2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<RouteCipher2*>(this)->keyRecovered(
        static_cast<int>(bestPath), bestFit, timer.elapsed());
    return bestPath;
}

/* ---- Reset ---- */

void RouteCipher2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
