/**
 * @file PlayfairCode.cpp
 * @brief PlayfairCode 实现
 *
 * 实现Playfair密码：密钥方阵、二连字替换、频率分析破解。
 */

#include "utils/code177/PlayfairCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- English bigram log-frequencies (top 25 for fitness) ---- */
static const QPair<QString, double> s_bigrams[] = {
    {"TH", 3.56}, {"HE", 3.07}, {"IN", 2.43}, {"ER", 2.05}, {"AN", 1.99},
    {"RE", 1.85}, {"ON", 1.76}, {"AT", 1.49}, {"EN", 1.45}, {"ND", 1.35},
    {"TI", 1.34}, {"ES", 1.34}, {"OR", 1.28}, {"TE", 1.27}, {"OF", 1.17},
    {"ED", 1.17}, {"IS", 1.13}, {"IT", 1.12}, {"AL", 1.09}, {"AR", 1.07},
    {"ST", 1.05}, {"TO", 1.05}, {"NT", 1.04}, {"NG", 0.95}, {"SE", 0.93}
};

/* ---- Construction / Destruction ---- */

PlayfairCode::PlayfairCode(QObject *parent)
    : QObject(parent)
{
}

PlayfairCode::~PlayfairCode() = default;

/* ---- Key square generation ---- */

void PlayfairCode::setKey(const QString& key)
{
    /* Build 5x5 square: key first (unique letters), then remaining alphabet */
    bool used[26] = {};
    QString processed;
    for (QChar ch : key.toUpper()) {
        if (!ch.isLetter()) continue;
        if (ch == 'J') ch = 'I';
        int idx = ch.toLatin1() - 'A';
        if (!used[idx]) { used[idx] = true; processed += ch; }
    }
    /* Fill remaining alphabet */
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (c == 'J') continue;
        if (!used[c - 'A']) { used[c - 'A'] = true; processed += QChar(c); }
    }

    int k = 0;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            m_square[r][c] = processed[k++];

    m_keySet = true;
    m_stats.keyLength = processed.size();
}

/* ---- Preprocess: uppercase, remove non-alpha, J→I, insert X filler ---- */

QString PlayfairCode::preprocess(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (!ch.isLetter()) continue;
        if (ch == 'J') ch = 'I';
        result += ch;
    }
    /* Split into digraphs, insert X between repeated chars */
    QString out;
    int i = 0;
    while (i < result.size()) {
        QChar a = result[i++];
        QChar b = (i < result.size()) ? result[i] : 'X';
        if (a == b) { b = 'X'; } else { i++; }
        out += a;
        out += b;
    }
    return out;
}

/* ---- Find character position in square ---- */

QPair<int, int> PlayfairCode::findChar(QChar c) const
{
    for (int r = 0; r < 5; ++r)
        for (int col = 0; col < 5; ++col)
            if (m_square[r][col] == c) return {r, col};
    return {0, 0};
}

/* ---- Process a digraph pair (encrypt or decrypt) ---- */

QPair<QChar, QChar> PlayfairCode::processPair(QChar a, QChar b, bool enc) const
{
    auto [r1, c1] = findChar(a);
    auto [r2, c2] = findChar(b);
    int dir = enc ? 1 : 4; /* +1 for encrypt, -1 mod 5 = 4 for decrypt */

    if (r1 == r2) {
        /* Same row: shift right/left */
        return {m_square[r1][(c1 + dir) % 5], m_square[r2][(c2 + dir) % 5]};
    } else if (c1 == c2) {
        /* Same column: shift down/up */
        return {m_square[(r1 + dir) % 5][c1], m_square[(r2 + dir) % 5][c2]};
    } else {
        /* Rectangle: swap columns */
        return {m_square[r1][c2], m_square[r2][c1]};
    }
}

/* ---- Encrypt ---- */

QString PlayfairCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_keySet) return {};

    QString prepared = preprocess(plaintext);
    QString result;
    for (int i = 0; i < prepared.size(); i += 2) {
        auto [a, b] = processPair(prepared[i], prepared[i + 1], true);
        result += a;
        result += b;
    }

    m_stats.totalEncrypts++;
    m_stats.textSize = plaintext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncrypts + m_stats.totalDecrypts);

    emit encryptCompleted(result.size());
    return result;
}

/* ---- Decrypt ---- */

QString PlayfairCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_keySet) return {};

    QString prepared = preprocess(ciphertext);
    QString result;
    for (int i = 0; i < prepared.size(); i += 2) {
        auto [a, b] = processPair(prepared[i], prepared[i + 1], false);
        result += a;
        result += b;
    }

    m_stats.totalDecrypts++;
    m_stats.textSize = ciphertext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncrypts + m_stats.totalDecrypts);

    emit decryptCompleted(result.size());
    return result;
}

/* ---- Fitness: measure how "English-like" text is ---- */

double PlayfairCode::fitnessScore(const QString& text) const
{
    double score = 0.0;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size() - 1; ++i) {
        QString bg = upper.mid(i, 2);
        for (const auto& p : s_bigrams) {
            if (bg == p.first) { score += p.second; break; }
        }
    }
    return score / qMax(1, upper.size() - 1);
}

/* ---- Helper: swap two positions in square ---- */

void PlayfairCode::swapInSquare(int r1, int c1, int r2, int c2)
{
    QChar tmp = m_square[r1][c1];
    m_square[r1][c1] = m_square[r2][c2];
    m_square[r2][c2] = tmp;
}

/* ---- Generate random square for analysis ---- */

void PlayfairCode::generateRandomSquare()
{
    QString alpha;
    for (char c = 'A'; c <= 'Z'; ++c)
        if (c != 'J') alpha += QChar(c);

    /* Fisher-Yates shuffle */
    for (int i = alpha.size() - 1; i > 0; --i) {
        int j = qrand() % (i + 1);
        std::swap(alpha[i], alpha[j]);
    }

    int k = 0;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            m_square[r][c] = alpha[k++];
}

/* ---- Square to key string ---- */

QString PlayfairCode::squareToKey() const
{
    QString key;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            key += m_square[r][c];
    return key;
}

/* ---- Frequency-based cryptanalysis via simulated annealing ---- */

QString PlayfairCode::analyzeFrequency(const QString& ciphertext, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    generateRandomSquare();
    m_keySet = true;

    QString bestDecrypted = decrypt(ciphertext);
    double bestFitness = fitnessScore(bestDecrypted);
    double temp = 20.0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        /* Random swap in square */
        int r1 = qrand() % 5, c1 = qrand() % 5;
        int r2 = qrand() % 5, c2 = qrand() % 5;
        if (r1 == r2 && c1 == c2) continue;

        swapInSquare(r1, c1, r2, c2);
        QString trial = decrypt(ciphertext);
        double trialFit = fitnessScore(trial);

        double delta = trialFit - bestFitness;
        double prob = (delta > 0) ? 1.0 : qExp(delta / qMax(0.01, temp));

        if (delta > 0 || (qrand() % 10000) / 10000.0 < prob) {
            bestFitness = trialFit;
            bestDecrypted = trial;
        } else {
            /* Revert swap */
            swapInSquare(r1, c1, r2, c2);
        }

        temp *= 0.995; /* Cool down */
    }

    m_timeSum += timer.elapsed();
    return squareToKey();
}

/* ---- Reset ---- */

void PlayfairCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
