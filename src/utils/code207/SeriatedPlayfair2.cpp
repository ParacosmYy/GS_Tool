/**
 * @file SeriatedPlayfair2.cpp
 * @brief SeriatedPlayfair2 实现
 *
 * 实现序列化Playfair密码：矩阵构建、双字母处理、模拟退火密钥恢复。
 */

#include "utils/code207/SeriatedPlayfair2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Static digraph frequency table ---- */

QVector<QVector<double>> SeriatedPlayfair2::s_digraphFreq;

void SeriatedPlayfair2::initDigraphFreq()
{
    if (!s_digraphFreq.isEmpty()) return;
    s_digraphFreq.resize(26);
    // Top English digraphs as sparse log-probability approximation
    for (auto& row : s_digraphFreq)
        row.resize(26, -5.0);

    auto setFreq = [](int a, int b, double f) {
        s_digraphFreq[a][b] = f;
    };
    // Common English digraphs (log10 scaled)
    setFreq('T'-'A', 'H'-'A', 0.0); setFreq('H'-'A', 'E'-'A', -0.1);
    setFreq('I'-'A', 'N'-'A', -0.15); setFreq('E'-'A', 'R'-'A', -0.2);
    setFreq('A'-'A', 'N'-'A', -0.25); setFreq('R'-'A', 'E'-'A', -0.3);
    setFreq('N'-'A', 'D'-'A', -0.35); setFreq('T'-'A', 'I'-'A', -0.4);
    setFreq('O'-'A', 'N'-'A', -0.42); setFreq('S'-'A', 'T'-'A', -0.45);
    setFreq('E'-'A', 'S'-'A', -0.5); setFreq('E'-'A', 'N'-'A', -0.52);
    setFreq('O'-'A', 'R'-'A', -0.55); setFreq('A'-'A', 'T'-'A', -0.6);
    setFreq('T'-'A', 'O'-'A', -0.62); setFreq('I'-'A', 'T'-'A', -0.65);
    setFreq('N'-'A', 'G'-'A', -0.7); setFreq('A'-'A', 'R'-'A', -0.72);
    setFreq('O'-'A', 'U'-'A', -0.75); setFreq('T'-'A', 'E'-'A', -0.78);
    setFreq('S'-'A', 'E'-'A', -0.8); setFreq('I'-'A', 'S'-'A', -0.82);
    setFreq('H'-'A', 'A'-'A', -0.85); setFreq('L'-'A', 'E'-'A', -0.88);
    setFreq('O'-'A', 'I'-'A', -0.9); setFreq('N'-'A', 'T'-'A', -0.92);
    setFreq('A'-'A', 'L'-'A', -0.95);
}

/* ---- Construction / Destruction ---- */

SeriatedPlayfair2::SeriatedPlayfair2(QObject *parent) : QObject(parent) { initDigraphFreq(); }
SeriatedPlayfair2::~SeriatedPlayfair2() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair2::setPeriod(int period) { m_period = qMax(2, period); }
void SeriatedPlayfair2::setMaxSAIterations(int maxIter) { m_maxSAIter = qMax(100, maxIter); }
void SeriatedPlayfair2::setCoolingRate(double rate) { m_coolingRate = qBound(0.9, rate, 0.9999); }
void SeriatedPlayfair2::setInitialTemp(double temp) { m_initTemp = qMax(0.1, temp); }

/* ---- Preprocess text ---- */

QString SeriatedPlayfair2::preprocess(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (ch.isLetter()) {
            if (ch == 'J') ch = 'I';
            result.append(ch);
        }
    }
    return result;
}

/* ---- Build Playfair matrix ---- */

QVector<QVector<int>> SeriatedPlayfair2::buildMatrix(const QString& key) const
{
    QVector<QVector<int>> matrix(5, QVector<int>(5));
    QVector<bool> used(26, false);
    used['J' - 'A'] = true; // I/J merged

    QString k = preprocess(key);
    int row = 0, col = 0;

    for (QChar ch : k) {
        int idx = ch.unicode() - 'A';
        if (idx >= 0 && idx < 26 && !used[idx]) {
            matrix[row][col] = idx;
            used[idx] = true;
            if (++col == 5) { col = 0; ++row; }
        }
    }
    for (int i = 0; i < 26; ++i) {
        if (!used[i]) {
            matrix[row][col] = i;
            if (++col == 5) { col = 0; ++row; }
        }
    }
    return matrix;
}

/* ---- Find position in matrix ---- */

QPair<int, int> SeriatedPlayfair2::findInMatrix(int ch,
                                                  const QVector<QVector<int>>& matrix) const
{
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            if (matrix[r][c] == ch) return {r, c};
    return {0, 0};
}

/* ---- Process digraph pair ---- */

QPair<QChar, QChar> SeriatedPlayfair2::processPair(QChar a, QChar b,
                                                      const QVector<QVector<int>>& matrix,
                                                      bool encrypt) const
{
    int chA = a.unicode() - 'A', chB = b.unicode() - 'A';
    auto [r1, c1] = findInMatrix(chA, matrix);
    auto [r2, c2] = findInMatrix(chB, matrix);
    int dir = encrypt ? 1 : -1;

    int newA, newB;
    if (r1 == r2) {
        // Same row: shift columns
        newA = matrix[r1][(c1 + dir + 5) % 5];
        newB = matrix[r2][(c2 + dir + 5) % 5];
    } else if (c1 == c2) {
        // Same column: shift rows
        newA = matrix[(r1 + dir + 5) % 5][c1];
        newB = matrix[(r2 + dir + 5) % 5][c2];
    } else {
        // Rectangle: swap columns
        newA = matrix[r1][c2];
        newB = matrix[r2][c1];
    }
    return {QChar('A' + newA), QChar('A' + newB)};
}

/* ---- Encrypt ---- */

QString SeriatedPlayfair2::encrypt(const QString& plaintext, const QString& key) const
{
    QString text = preprocess(plaintext);
    auto matrix = buildMatrix(key);
    QString result;

    // Seriated: process in groups of m_period characters
    for (int g = 0; g < text.size(); g += m_period) {
        QString group = text.mid(g, m_period);
        // Pad if odd
        if (group.size() % 2 == 1) group.append('X');
        for (int i = 0; i < group.size(); i += 2) {
            auto [a, b] = processPair(group[i], group[i+1], matrix, true);
            result.append(a);
            result.append(b);
        }
    }
    return result;
}

/* ---- Decrypt ---- */

QString SeriatedPlayfair2::decrypt(const QString& ciphertext, const QString& key) const
{
    QString text = preprocess(ciphertext);
    auto matrix = buildMatrix(key);
    QString result;

    for (int g = 0; g < text.size(); g += m_period) {
        QString group = text.mid(g, m_period);
        for (int i = 0; i + 1 < group.size(); i += 2) {
            auto [a, b] = processPair(group[i], group[i+1], matrix, false);
            result.append(a);
            result.append(b);
        }
    }
    return result;
}

/* ---- Digraph score ---- */

double SeriatedPlayfair2::digraphScore(const QString& text) const
{
    double score = 0.0;
    QString clean = preprocess(text);
    for (int i = 0; i + 1 < clean.size(); ++i) {
        int a = clean[i].unicode() - 'A';
        int b = clean[i+1].unicode() - 'A';
        if (a >= 0 && a < 26 && b >= 0 && b < 26)
            score += s_digraphFreq[a][b];
    }
    return score;
}

/* ---- Random key ---- */

QString SeriatedPlayfair2::randomKey() const
{
    QVector<int> letters;
    for (int i = 0; i < 26; ++i)
        if (i != 'J' - 'A') letters.append(i);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(letters.begin(), letters.end(), g);
    QString key;
    for (int l : letters) key.append(QChar('A' + l));
    return key;
}

/* ---- Perturb key ---- */

QString SeriatedPlayfair2::perturbKey(const QString& key) const
{
    QString newKey = key;
    int i = qrand() % newKey.size();
    int j = qrand() % newKey.size();
    std::swap(newKey[i], newKey[j]);
    return newKey;
}

/* ---- Simulated annealing crack ---- */

QString SeriatedPlayfair2::crackSA(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    m_bestKey = randomKey();
    double bestScore = digraphScore(decrypt(ciphertext, m_bestKey));
    QString currentKey = m_bestKey;
    double currentScore = bestScore;

    double temp = m_initTemp;
    m_stats.saIterations = 0;

    for (int iter = 0; iter < m_maxSAIter; ++iter) {
        QString newKey = perturbKey(currentKey);
        double newScore = digraphScore(decrypt(ciphertext, newKey));
        double delta = newScore - currentScore;

        if (delta > 0 || qExp(delta / qMax(temp, 1e-10))
                         > static_cast<double>(qrand()) / RAND_MAX) {
            currentKey = newKey;
            currentScore = newScore;
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            m_bestKey = currentKey;
        }

        temp *= m_coolingRate;
        m_stats.saIterations = iter;

        if (iter % 500 == 0)
            emit crackProgress(iter, bestScore, temp);
    }

    m_stats.bestScore = bestScore;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("crackSA", timer.elapsed());
    return m_bestKey;
}

/* ---- Get best key ---- */

QString SeriatedPlayfair2::getBestKey() const { return m_bestKey; }

/* ---- Reset ---- */

void SeriatedPlayfair2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bestKey.clear();
}
