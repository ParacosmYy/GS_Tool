/**
 * @file SeriatedPlayfair3.cpp
 * @brief SeriatedPlayfair3 实现
 *
 * 实现序列化Playfair密码：6x6扩展网格、模拟退火、双字母组频率评分。
 */

#include "utils/code220/SeriatedPlayfair3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair3::SeriatedPlayfair3(QObject *parent) : QObject(parent)
{
    m_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    m_bigramFreq.resize(36);
    for (int i = 0; i < 36; ++i)
        m_bigramFreq[i].resize(36, 0.0);
}

SeriatedPlayfair3::~SeriatedPlayfair3() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair3::setSAParameters(double initTemp, double coolingRate,
                                          int maxIter)
{
    m_initTemp = qMax(0.1, initTemp);
    m_coolingRate = qBound(0.9, coolingRate, 0.9999);
    m_maxIter = qMax(100, maxIter);
}

/* ---- Normalize key ---- */

QString SeriatedPlayfair3::normalizeKey(const QString& key) const
{
    QString norm;
    for (auto ch : key.toUpper()) {
        int idx = m_alphabet.indexOf(ch);
        if (idx >= 0 && !norm.contains(ch))
            norm.append(ch);
    }
    // Fill remaining alphabet chars
    for (auto ch : m_alphabet) {
        if (!norm.contains(ch))
            norm.append(ch);
    }
    return norm;
}

/* ---- Build 6x6 grid ---- */

QVector<QVector<int>> SeriatedPlayfair3::buildGrid(const QString& key) const
{
    QString norm = normalizeKey(key);
    QVector<QVector<int>> grid(6);
    for (int r = 0; r < 6; ++r) {
        grid[r].resize(6);
        for (int c = 0; c < 6; ++c)
            grid[r][c] = norm[r * 6 + c].toLatin1();
    }
    return grid;
}

/* ---- Find position ---- */

void SeriatedPlayfair3::findPosition(const QVector<QVector<int>>& grid,
                                       int ch, int& row, int& col) const
{
    char c = QChar(ch).toUpper().toLatin1();
    for (int r = 0; r < 6; ++r) {
        for (int cc = 0; cc < 6; ++cc) {
            if (grid[r][cc] == c) { row = r; col = cc; return; }
        }
    }
    row = 0; col = 0;
}

/* ---- Pair transform ---- */

QString SeriatedPlayfair3::transformPairs(
    const QString& text, const QVector<QVector<int>>& grid, bool enc) const
{
    QString result;
    QString prep;
    for (auto ch : text.toUpper()) {
        if (m_alphabet.contains(ch)) prep.append(ch);
    }
    // Ensure even length
    if (prep.size() % 2 != 0) prep.append('X');

    for (int i = 0; i < prep.size() - 1; i += 2) {
        int r1, c1, r2, c2;
        findPosition(grid, prep[i].toLatin1(), r1, c1);
        findPosition(grid, prep[i + 1].toLatin1(), r2, c2);

        if (r1 == r2) {
            // Same row: shift right (encrypt) or left (decrypt)
            int shift = enc ? 1 : 5;
            result.append(QChar(grid[r1][(c1 + shift) % 6]));
            result.append(QChar(grid[r2][(c2 + shift) % 6]));
        } else if (c1 == c2) {
            // Same column: shift down (encrypt) or up (decrypt)
            int shift = enc ? 1 : 5;
            result.append(QChar(grid[(r1 + shift) % 6][c1]));
            result.append(QChar(grid[(r2 + shift) % 6][c2]));
        } else {
            // Rectangle swap
            result.append(QChar(grid[r1][c2]));
            result.append(QChar(grid[r2][c1]));
        }
    }
    return result;
}

/* ---- Encrypt / Decrypt ---- */

QString SeriatedPlayfair3::encrypt(const QString& plaintext,
                                     const QString& key) const
{
    return transformPairs(plaintext, buildGrid(key), true);
}

QString SeriatedPlayfair3::decrypt(const QString& ciphertext,
                                     const QString& key) const
{
    return transformPairs(ciphertext, buildGrid(key), false);
}

/* ---- Bigram scoring ---- */

double SeriatedPlayfair3::scoreText(const QString& text) const
{
    double score = 0.0;
    for (int i = 0; i < text.size() - 1; ++i) {
        int a = m_alphabet.indexOf(text[i].toUpper());
        int b = m_alphabet.indexOf(text[i + 1].toUpper());
        if (a >= 0 && b >= 0 && a < 36 && b < 36)
            score += m_bigramFreq[a][b];
    }
    return score;
}

/* ---- Random key ---- */

QString SeriatedPlayfair3::randomKey() const
{
    QString key = m_alphabet;
    for (int i = key.size() - 1; i > 0; --i) {
        int j = qrand() % (i + 1);
        std::swap(key[i], key[j]);
    }
    return key;
}

/* ---- Mutate key ---- */

QString SeriatedPlayfair3::mutateKey(const QString& key) const
{
    QString mutated = key;
    int i = qrand() % mutated.size();
    int j = qrand() % mutated.size();
    std::swap(mutated[i], mutated[j]);
    return mutated;
}

/* ---- Load bigram frequencies ---- */

void SeriatedPlayfair3::loadBigramFrequencies(const QVector<QVector<double>>& freq)
{
    m_bigramFreq = freq;
}

/* ---- Crack using simulated annealing ---- */

SeriatedPlayfair3::DecryptResult SeriatedPlayfair3::crack(
    const QString& ciphertext, int seriationWidth)
{
    QElapsedTimer timer;
    timer.start();

    DecryptResult result;
    QString bestKey = randomKey();
    QString bestDecrypt = decrypt(ciphertext, bestKey);
    double bestScore = scoreText(bestDecrypt);

    QString currentKey = bestKey;
    double currentScore = bestScore;
    double temp = m_initTemp;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QString newKey = mutateKey(currentKey);
        QString newDecrypt = decrypt(ciphertext, newKey);
        double newScore = scoreText(newDecrypt);

        double delta = newScore - currentScore;
        if (delta > 0 || (temp > 0 && qExp(delta / temp) > (qrand() % 10000) / 10000.0)) {
            currentKey = newKey;
            currentScore = newScore;
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            bestKey = currentKey;
            bestDecrypt = newDecrypt;
        }

        temp *= m_coolingRate;

        if (iter % 500 == 0)
            emit decryptionProgress(iter, bestScore, temp);
    }

    result.plaintext = bestDecrypt;
    result.fitness = bestScore;
    result.iterations = m_maxIter;
    result.temperature = temp;

    m_stats.numDecryptions++;
    m_stats.bestFitness = qMax(m_stats.bestFitness, bestScore);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit crackingCompleted(bestScore, m_maxIter, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void SeriatedPlayfair3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
