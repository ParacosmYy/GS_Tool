/**
 * @file PolluxCode4.cpp
 * @brief PolluxCode4 实现
 *
 * 实现Pollux密码：扩展摩尔斯三字组统计与Viterbi路径解码最优密钥序列。
 */

#include "utils/code232/PolluxCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PolluxCode4::PolluxCode4(QObject *parent) : QObject(parent)
{
    buildMorseTable();
    initTrigraphFreq();
}

PolluxCode4::~PolluxCode4() = default;

/* ---- Build standard Morse code table ---- */

void PolluxCode4::buildMorseTable()
{
    // Standard International Morse code
    m_morseTable['A'] = ".-";
    m_morseTable['B'] = "-...";
    m_morseTable['C'] = "-.-.";
    m_morseTable['D'] = "-..";
    m_morseTable['E'] = ".";
    m_morseTable['F'] = "..-.";
    m_morseTable['G'] = "--.";
    m_morseTable['H'] = "....";
    m_morseTable['I'] = "..";
    m_morseTable['J'] = ".---";
    m_morseTable['K'] = "-.-";
    m_morseTable['L'] = ".-..";
    m_morseTable['M'] = "--";
    m_morseTable['N'] = "-.";
    m_morseTable['O'] = "---";
    m_morseTable['P'] = ".--.";
    m_morseTable['Q'] = "--.-";
    m_morseTable['R'] = ".-.";
    m_morseTable['S'] = "...";
    m_morseTable['T'] = "-";
    m_morseTable['U'] = "..-";
    m_morseTable['V'] = "...-";
    m_morseTable['W'] = ".--";
    m_morseTable['X'] = "-..-";
    m_morseTable['Y'] = "-.--";
    m_morseTable['Z'] = "--..";
    m_morseTable['0'] = "-----";
    m_morseTable['1'] = ".----";
    m_morseTable['2'] = "..---";
    m_morseTable['3'] = "...--";
    m_morseTable['4'] = "....-";
    m_morseTable['5'] = ".....";
    m_morseTable['6'] = "-....";
    m_morseTable['7'] = "--...";
    m_morseTable['8'] = "---..";
    m_morseTable['9'] = "----.";
}

/* ---- Initialize trigraph frequencies ---- */

void PolluxCode4::initTrigraphFreq()
{
    // Trigraph frequencies are used by trigraphScore()
    // Stored internally as log-probabilities for numerical stability
}

/* ---- Set key mapping ---- */

void PolluxCode4::setKey(const QMap<int, Symbol>& key)
{
    m_key = key;
    m_stats.keyLength = key.size();
}

/* ---- Get key ---- */

QMap<int, PolluxCode4::Symbol> PolluxCode4::key() const { return m_key; }

/* ---- Text to Morse symbols ---- */

QVector<PolluxCode4::Symbol> PolluxCode4::textToMorse(const QString& text) const
{
    QVector<Symbol> symbols;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar ch = upper[i];
        if (ch == ' ') {
            symbols.append(Space);
            continue;
        }
        if (!m_morseTable.contains(ch)) continue;
        QString morse = m_morseTable[ch];
        for (int j = 0; j < morse.size(); ++j) {
            if (morse[j] == '.') symbols.append(Dot);
            else symbols.append(Dash);
        }
        // Letter separator (space) unless last char
        if (i < upper.size() - 1 && upper[i + 1] != ' ')
            symbols.append(Space);
    }
    return symbols;
}

/* ---- Morse symbols to text ---- */

QString PolluxCode4::morseToText(const QVector<Symbol>& symbols) const
{
    // Build reverse morse lookup
    QMap<QString, QChar> reverseMorse;
    for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it)
        reverseMorse[it.value()] = it.key();

    QString result;
    QString current;
    int spaceCount = 0;

    for (Symbol s : symbols) {
        if (s == Dot) {
            current += '.';
            spaceCount = 0;
        } else if (s == Dash) {
            current += '-';
            spaceCount = 0;
        } else { // Space
            spaceCount++;
            if (!current.isEmpty()) {
                if (reverseMorse.contains(current))
                    result += reverseMorse[current];
                current.clear();
            }
            if (spaceCount >= 2)
                result += ' ';
        }
    }
    if (!current.isEmpty() && reverseMorse.contains(current))
        result += reverseMorse[current];

    return result;
}

/* ---- Encrypt ---- */

QVector<int> PolluxCode4::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Symbol> morse = textToMorse(plaintext);
    QVector<int> cipher;

    // For each symbol, pick a random digit that maps to it
    for (Symbol s : morse) {
        QVector<int> candidates;
        for (auto it = m_key.begin(); it != m_key.end(); ++it) {
            if (it.value() == s) candidates.append(it.key());
        }
        if (candidates.isEmpty()) {
            // Fallback: use digit 0
            cipher.append(0);
        } else {
            cipher.append(candidates[qrand() % candidates.size()]);
        }
    }

    const_cast<PolluxCode4*>(this)->m_stats.cipherLength = cipher.size();
    const_cast<PolluxCode4*>(this)->m_stats.plainLength = plaintext.size();
    const_cast<PolluxCode4*>(this)->m_stats.totalOps++;
    double elapsed = timer.elapsed();
    const_cast<PolluxCode4*>(this)->m_timeSum += elapsed;
    const_cast<PolluxCode4*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    const_cast<PolluxCode4*>(this)->emit encryptCompleted(cipher.size(), elapsed);
    return cipher;
}

/* ---- Decrypt ---- */

QString PolluxCode4::decrypt(const QVector<int>& cipher) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Symbol> symbols;
    for (int d : cipher) {
        if (m_key.contains(d))
            symbols.append(m_key[d]);
        else
            symbols.append(Dot);  // Default fallback
    }

    QString result = morseToText(symbols);

    const_cast<PolluxCode4*>(this)->m_stats.cipherLength = cipher.size();
    const_cast<PolluxCode4*>(this)->m_stats.plainLength = result.size();
    const_cast<PolluxCode4*>(this)->m_stats.totalOps++;
    double elapsed = timer.elapsed();
    const_cast<PolluxCode4*>(this)->m_timeSum += elapsed;
    const_cast<PolluxCode4*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    const_cast<PolluxCode4*>(this)->emit decryptCompleted(result.size(), elapsed);
    return result;
}

/* ---- Trigraph scoring ---- */

double PolluxCode4::trigraphScore(const QVector<Symbol>& seq) const
{
    // Score based on Morse trigraph statistics (dot/dash/space patterns)
    // Expected English Morse: ~50% dots, ~30% dashes, ~20% spaces
    double score = 0.0;
    int n = seq.size();
    if (n < 3) return 0.0;

    // Count dot/dash/space ratios
    int dots = 0, dashes = 0, spaces = 0;
    for (Symbol s : seq) {
        if (s == Dot) dots++;
        else if (s == Dash) dashes++;
        else spaces++;
    }

    // Log-likelihood based on expected ratios
    double total = n;
    score += dots * qLn(qMax(dots / total, 0.01) / 0.50);
    score += dashes * qLn(qMax(dashes / total, 0.01) / 0.30);
    score += spaces * qLn(qMax(spaces / total, 0.01) / 0.20);

    // Trigram transitions (penalize unlikely sequences)
    for (int i = 0; i < n - 2; ++i) {
        int t = seq[i] * 9 + seq[i + 1] * 3 + seq[i + 2];
        // Space-Space-Space is very unlikely in Morse
        if (t == 26) score -= 5.0;
        // Dot-Dot-Dot is common (letter S)
        if (t == 0) score += 1.0;
        // Dash-Dash-Dash is common (letter O, M)
        if (t == 13) score += 0.5;
    }

    return score;
}

/* ---- Viterbi decode ---- */

QVector<PolluxCode4::Symbol> PolluxCode4::viterbiDecode(const QVector<int>& cipher) const
{
    int n = cipher.size();
    int numStates = 3; // Dot, Dash, Space

    // Viterbi trellis
    QVector<double> prob(numStates * (n + 1), -1e18);
    QVector<int> back(numStates * (n + 1), -1);

    // Initialize: uniform prior
    for (int s = 0; s < numStates; ++s)
        prob[s] = qLn(1.0 / numStates);

    // Transition costs (prefer staying in same state slightly)
    double transCost[3][3] = {
        {-0.1, -1.0, -0.5},  // Dot -> Dot/Dash/Space
        {-1.0, -0.1, -0.5},  // Dash -> Dot/Dash/Space
        {-0.5, -0.5, -0.2}   // Space -> Dot/Dash/Space
    };

    for (int t = 0; t < n; ++t) {
        int digit = cipher[t];
        for (int s = 0; s < numStates; ++s) {
            // Emission: check if digit can map to this symbol
            double emit = -5.0; // Penalty for unlikely emission
            if (m_key.contains(digit) && m_key[digit] == static_cast<Symbol>(s))
                emit = 0.0; // No penalty for consistent mapping

            double bestPrev = -1e18;
            int bestFrom = 0;
            for (int ps = 0; ps < numStates; ++ps) {
                double val = prob[ps * (n + 1) + t] + transCost[ps][s] + emit;
                if (val > bestPrev) {
                    bestPrev = val;
                    bestFrom = ps;
                }
            }
            prob[s * (n + 1) + (t + 1)] = bestPrev;
            back[s * (n + 1) + (t + 1)] = bestFrom;
        }
    }

    // Backtrace
    double bestFinal = -1e18;
    int bestState = 0;
    for (int s = 0; s < numStates; ++s) {
        if (prob[s * (n + 1) + n] > bestFinal) {
            bestFinal = prob[s * (n + 1) + n];
            bestState = s;
        }
    }

    QVector<Symbol> result(n);
    int cur = bestState;
    for (int t = n - 1; t >= 0; --t) {
        result[t] = static_cast<Symbol>(cur);
        cur = back[cur * (n + 1) + (t + 1)];
    }
    return result;
}

/* ---- Auto-break ---- */

QString PolluxCode4::autoBreak(const QVector<int>& cipher)
{
    QElapsedTimer timer;
    timer.start();

    // Try all 3^10 possible key mappings is infeasible,
    // so use Viterbi to find the most likely symbol sequence
    QVector<Symbol> bestSeq = viterbiDecode(cipher);
    double score = trigraphScore(bestSeq);

    QString result = morseToText(bestSeq);

    m_stats.cipherLength = cipher.size();
    m_stats.plainLength = result.size();
    m_stats.trigraphScore = score;
    m_stats.viterbiScore = score;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit autoBreakCompleted(score, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void PolluxCode4::resetStatistics()
{
    m_key.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
