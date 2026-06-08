/**
 * @file TapirCode2.cpp
 * @brief TapirCode2 实现
 *
 * 实现袋鼠码：扩展字母表替换、频率偏向爬山密钥优化、N-gram评分。
 */

#include "utils/code211/TapirCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>
#include <random>

/* ---- Construction / Destruction ---- */

TapirCode2::TapirCode2(QObject *parent) : QObject(parent)
{
    m_alphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

TapirCode2::~TapirCode2() = default;

/* ---- Configuration ---- */

void TapirCode2::setAlphabet(const QString& alphabet) { m_alphabet = alphabet; }

void TapirCode2::setExpectedFrequencies(const QVector<QPair<QChar, double>>& freqs)
{
    m_expectedFreqs = freqs;
}

/* ---- Build substitution map ---- */

QMap<QChar, QChar> TapirCode2::buildSubstitutionMap(const QByteArray& key,
                                                      bool invert) const
{
    QMap<QChar, QChar> map;
    QString alpha = m_alphabet;
    int klen = key.size();

    for (int i = 0; i < alpha.size() && i < klen; ++i) {
        if (invert)
            map[QChar(key[i])] = alpha[i];
        else
            map[alpha[i]] = QChar(key[i]);
    }
    return map;
}

/* ---- Encode ---- */

QByteArray TapirCode2::encode(const QByteArray& input, const QByteArray& key) const
{
    QElapsedTimer timer;
    timer.start();

    auto subMap = buildSubstitutionMap(key, false);
    QByteArray result;
    result.reserve(input.size());

    for (char c : input) {
        QChar ch(c);
        if (subMap.contains(ch))
            result.append(subMap[ch].toLatin1());
        else
            result.append(c);
    }

    auto self = const_cast<TapirCode2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.inputLength = input.size();
    self->m_stats.outputLength = result.size();
    self->m_stats.alphabetSize = m_alphabet.size();
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit encodingCompleted(input.size(), result.size(), timer.elapsed());

    return result;
}

/* ---- Decode ---- */

QByteArray TapirCode2::decode(const QByteArray& input, const QByteArray& key) const
{
    QElapsedTimer timer;
    timer.start();

    auto subMap = buildSubstitutionMap(key, true);
    QByteArray result;
    result.reserve(input.size());

    for (char c : input) {
        QChar ch(c);
        if (subMap.contains(ch))
            result.append(subMap[ch].toLatin1());
        else
            result.append(c);
    }

    auto self = const_cast<TapirCode2*>(this);
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Swap key positions ---- */

QByteArray TapirCode2::swapKeyPositions(const QByteArray& key, int i, int j)
{
    QByteArray result = key;
    if (i >= 0 && i < result.size() && j >= 0 && j < result.size())
        std::swap(result[i], result[j]);
    return result;
}

/* ---- Compute frequencies ---- */

QVector<QPair<QChar, double>> TapirCode2::computeFrequencies(const QByteArray& text) const
{
    QMap<QChar, int> counts;
    int total = 0;
    for (char c : text) {
        QChar ch(c);
        if (m_alphabet.contains(ch, Qt::CaseSensitive)) {
            counts[ch]++;
            total++;
        }
    }
    QVector<QPair<QChar, double>> freqs;
    if (total == 0) return freqs;
    for (auto it = counts.begin(); it != counts.end(); ++it)
        freqs.append({it.key(), static_cast<double>(it.value()) / total});
    return freqs;
}

/* ---- Frequency fitness ---- */

double TapirCode2::frequencyFitness(const QByteArray& text) const
{
    if (m_expectedFreqs.isEmpty()) return 0.0;
    auto observed = computeFrequencies(text);
    if (observed.isEmpty()) return 0.0;

    // Chi-squared-like correlation with expected frequencies
    double score = 0.0;
    for (const auto& exp : m_expectedFreqs) {
        double obs = 0.0;
        for (const auto& o : observed) {
            if (o.first == exp.first) { obs = o.second; break; }
        }
        double diff = obs - exp.second;
        score -= diff * diff;
    }
    return score;
}

/* ---- N-gram score ---- */

double TapirCode2::ngramScore(const QByteArray& text, int n) const
{
    if (text.size() < n) return 0.0;

    // Simple digram/trigram frequency score
    QMap<QByteArray, int> counts;
    int total = 0;
    for (int i = 0; i <= text.size() - n; ++i) {
        QByteArray gram = text.mid(i, n);
        counts[gram]++;
        total++;
    }
    if (total == 0) return 0.0;

    // Score based on entropy (lower = more structured = better)
    double entropy = 0.0;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        double p = static_cast<double>(it.value()) / total;
        if (p > 0) entropy -= p * qLn(p);
    }
    return -entropy;  // Higher score for lower entropy
}

/* ---- Generate random key ---- */

QByteArray TapirCode2::generateRandomKey() const
{
    QByteArray key = m_alphabet.toUtf8();
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(key.begin(), key.end(), g);
    return key;
}

/* ---- Crack with hill climbing ---- */

QByteArray TapirCode2::crackWithHillClimb(const QByteArray& cipher,
                                            int maxIterations) const
{
    QElapsedTimer timer;
    timer.start();

    QByteArray bestKey = generateRandomKey();
    QByteArray plaintext = decode(cipher, bestKey);
    double bestScore = frequencyFitness(plaintext) + ngramScore(plaintext);

    int alphaLen = m_alphabet.size();
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> dist(0, alphaLen - 1);

    for (int iter = 0; iter < maxIterations; ++iter) {
        // Frequency-biased swap: swap positions weighted toward low-frequency chars
        int i = dist(g);
        int j = dist(g);
        while (j == i) j = dist(g);

        QByteArray newKey = swapKeyPositions(bestKey, i, j);
        QByteArray newPlain = decode(cipher, newKey);
        double newScore = frequencyFitness(newPlain) + ngramScore(newPlain);

        if (newScore > bestScore) {
            bestScore = newScore;
            bestKey = newKey;
        }

        // Random restart if stuck
        if (iter > 0 && iter % 200 == 0) {
            QByteArray restart = generateRandomKey();
            QByteArray restartPlain = decode(cipher, restart);
            double restartScore = frequencyFitness(restartPlain) + ngramScore(restartPlain);
            if (restartScore > bestScore * 0.8) {
                bestKey = restart;
                bestScore = restartScore;
            }
        }

        const_cast<TapirCode2*>(this)->emit crackingProgress(iter, bestScore);
    }

    auto self = const_cast<TapirCode2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.bestScore = bestScore;
    self->m_stats.hillClimbIterations = maxIterations;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return bestKey;
}

/* ---- Reset ---- */

void TapirCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
