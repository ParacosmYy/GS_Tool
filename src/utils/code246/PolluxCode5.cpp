/**
 * @file PolluxCode5.cpp
 * @brief PolluxCode5 实现
 *
 * 实现Pollux密码：扩展莫尔斯元素组合与n元频率概率密钥评分。
 */

#include "utils/code246/PolluxCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

PolluxCode5::PolluxCode5(QObject *parent) : QObject(parent)
{
    initMorseTable();
    initNgramFrequencies();
}

PolluxCode5::~PolluxCode5() = default;

/* ---- Configuration ---- */

void PolluxCode5::setNgramSize(int n) { m_ngramSize = qBound(2, n, 4); }
void PolluxCode5::setMaxCandidates(int count) { m_maxCandidates = qMax(10, count); }

/* ---- Initialize Morse code table ---- */

void PolluxCode5::initMorseTable()
{
    m_morseTable = {
        {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
        {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
        {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
        {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
        {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
        {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
        {'Y', "-.--"}, {'Z', "--.."}
    };
}

/* ---- Initialize English bigram frequencies (log10) ---- */

void PolluxCode5::initNgramFrequencies()
{
    // Common English bigram log-frequencies (approximate)
    m_ngramFreq = {
        {"TH", -1.08}, {"HE", -1.14}, {"IN", -1.30}, {"ER", -1.38},
        {"AN", -1.44}, {"RE", -1.50}, {"ON", -1.56}, {"AT", -1.62},
        {"EN", -1.68}, {"ND", -1.74}, {"TI", -1.78}, {"ES", -1.82},
        {"OR", -1.88}, {"TE", -1.92}, {"OF", -1.96}, {"ED", -2.00},
        {"IS", -2.04}, {"IT", -2.08}, {"AL", -2.12}, {"AR", -2.16},
        {"ST", -2.20}, {"TO", -2.24}, {"NT", -2.28}, {"NG", -2.32},
        {"SE", -2.36}, {"HA", -2.40}, {"AS", -2.44}, {"OU", -2.48},
        {"IO", -2.52}, {"LE", -2.56}, {"VE", -2.60}, {"CO", -2.64},
        {"ME", -2.68}, {"DE", -2.72}, {"HI", -2.76}, {"RI", -2.80},
        {"RO", -2.84}, {"IC", -2.88}, {"NE", -2.92}, {"EA", -2.96},
        {"RA", -3.00}, {"CE", -3.04}
    };
}

/* ---- Score plaintext using n-gram log-probability ---- */

double PolluxCode5::scoreText(const QString& text) const
{
    QString upper = text.toUpper();
    double score = 0.0;
    int n = m_ngramSize;
    int count = 0;
    for (int i = 0; i <= upper.size() - n; ++i) {
        QString gram = upper.mid(i, n);
        if (m_ngramFreq.contains(gram)) {
            score += m_ngramFreq[gram];
        } else {
            score -= 4.0;  // Penalty for unknown n-gram
        }
        count++;
    }
    return (count > 0) ? score / count : -1e9;
}

/* ---- Generate candidate key mappings ---- */

QVector<QVector<PolluxCode5::MorseElement>> PolluxCode5::generateCandidates(
    const QString& ciphertext) const
{
    // Count frequency of each digit in ciphertext
    QVector<int> digitFreq(10, 0);
    for (QChar c : ciphertext) {
        int d = c.digitValue();
        if (d >= 0 && d <= 9) digitFreq[d]++;
    }

    // Most frequent digits likely map to dot or separator (most common in Morse)
    // Generate random candidate mappings weighted by digit frequency
    QVector<QVector<MorseElement>> candidates;
    for (int trial = 0; trial < m_maxCandidates; ++trial) {
        QVector<MorseElement> mapping(10);
        // Assign elements with bias: ~40% dot, ~30% dash, ~30% separator
        for (int d = 0; d < 10; ++d) {
            int r = std::rand() % 10;
            if (r < 4) mapping[d] = Dot;
            else if (r < 7) mapping[d] = Dash;
            else mapping[d] = Separator;
        }
        // Ensure at least one of each element
        bool hasDot = false, hasDash = false, hasSep = false;
        for (auto e : mapping) {
            if (e == Dot) hasDot = true;
            if (e == Dash) hasDash = true;
            if (e == Separator) hasSep = true;
        }
        if (hasDot && hasDash && hasSep)
            candidates.append(mapping);
    }
    return candidates;
}

/* ---- Morse string to text ---- */

QString PolluxCode5::morseToText(const QString& morse) const
{
    // Build reverse lookup
    QMap<QString, QChar> reverseMorse;
    for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it)
        reverseMorse[it.value()] = it.key();

    // Split by separator spaces, then decode
    QVector<QString> letters;
    QString current;
    for (QChar c : morse) {
        if (c == ' ') {
            if (!current.isEmpty()) {
                letters.append(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.isEmpty()) letters.append(current);

    QString result;
    for (const QString& code : letters) {
        if (reverseMorse.contains(code))
            result += reverseMorse[code];
        else
            result += '?';
    }
    return result;
}

/* ---- Encrypt ---- */

QString PolluxCode5::encrypt(const QString& plaintext,
                              const QVector<MorseElement>& key) const
{
    if (key.size() < 10) return {};

    // Convert plaintext to Morse code
    QString morse;
    for (QChar c : plaintext.toUpper()) {
        if (m_morseTable.contains(c)) {
            if (!morse.isEmpty()) morse += ' ';  // Letter separator
            morse += m_morseTable[c];
        }
    }

    // Map each Morse symbol to digits
    QString result;
    for (QChar c : morse) {
        QVector<int> candidates;
        if (c == '.') {
            for (int d = 0; d < 10; ++d)
                if (key[d] == Dot) candidates.append(d);
        } else if (c == '-') {
            for (int d = 0; d < 10; ++d)
                if (key[d] == Dash) candidates.append(d);
        } else if (c == ' ') {
            for (int d = 0; d < 10; ++d)
                if (key[d] == Separator) candidates.append(d);
        }
        if (!candidates.isEmpty()) {
            int pick = candidates[std::rand() % candidates.size()];
            result += QString::number(pick);
        }
    }
    return result;
}

/* ---- Decrypt ---- */

QString PolluxCode5::decrypt(const QString& ciphertext,
                              const QVector<MorseElement>& key) const
{
    if (key.size() < 10) return {};

    // Convert digits to Morse symbols
    QString morse;
    for (QChar c : ciphertext) {
        int d = c.digitValue();
        if (d < 0 || d > 9) continue;
        switch (key[d]) {
        case Dot:       morse += '.'; break;
        case Dash:      morse += '-'; break;
        case Separator: morse += ' '; break;
        }
    }
    return morseToText(morse);
}

/* ---- Crack via n-gram scoring ---- */

PolluxCode5::KeyCandidate PolluxCode5::crack(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    auto candidates = generateCandidates(ciphertext);

    KeyCandidate best;
    best.score = -1e18;
    int evaluated = 0;

    for (const auto& mapping : candidates) {
        QString plaintext = decrypt(ciphertext, mapping);
        double sc = scoreText(plaintext);
        if (sc > best.score) {
            best.score = sc;
            best.mapping = mapping;
        }
        evaluated++;
        if (evaluated % 50 == 0)
            emit crackProgress(evaluated, best.score);
    }

    m_stats.cipherLength = ciphertext.size();
    m_stats.numCandidatesEvaluated = evaluated;
    m_stats.bestScore = best.score;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit crackCompleted(best.score, timer.elapsed());
    return best;
}

/* ---- Accessors ---- */

QMap<QString, double> PolluxCode5::ngramFrequencies() const
{
    return m_ngramFreq;
}

/* ---- Reset ---- */

void PolluxCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
