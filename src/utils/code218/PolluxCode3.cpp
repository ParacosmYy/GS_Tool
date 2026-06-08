/**
 * @file PolluxCode3.cpp
 * @brief PolluxCode3 实现
 *
 * 实现Pollux密码变体：扩展莫尔斯编码、密钥映射加解密、贝叶斯密钥推断。
 */

#include "utils/code218/PolluxCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PolluxCode3::PolluxCode3(QObject *parent) : QObject(parent)
{
    buildMorseTable();
}

PolluxCode3::~PolluxCode3() = default;

/* ---- Build Morse table ---- */

void PolluxCode3::buildMorseTable()
{
    m_morseTable.clear();
    // Letters
    m_morseTable['A'] = ".-";   m_morseTable['B'] = "-...";
    m_morseTable['C'] = "-.-."; m_morseTable['D'] = "-..";
    m_morseTable['E'] = ".";    m_morseTable['F'] = "..-.";
    m_morseTable['G'] = "--.";  m_morseTable['H'] = "....";
    m_morseTable['I'] = "..";   m_morseTable['J'] = ".---";
    m_morseTable['K'] = "-.-";  m_morseTable['L'] = ".-..";
    m_morseTable['M'] = "--";   m_morseTable['N'] = "-.";
    m_morseTable['O'] = "---";  m_morseTable['P'] = ".--.";
    m_morseTable['Q'] = "--.-"; m_morseTable['R'] = ".-.";
    m_morseTable['S'] = "...";  m_morseTable['T'] = "-";
    m_morseTable['U'] = "..-";  m_morseTable['V'] = "...-";
    m_morseTable['W'] = ".--";  m_morseTable['X'] = "-..-";
    m_morseTable['Y'] = "-.--"; m_morseTable['Z'] = "--..";
    // Digits
    m_morseTable['0'] = "-----"; m_morseTable['1'] = ".----";
    m_morseTable['2'] = "..---"; m_morseTable['3'] = "...--";
    m_morseTable['4'] = "....-"; m_morseTable['5'] = ".....";
    m_morseTable['6'] = "-...."; m_morseTable['7'] = "--...";
    m_morseTable['8'] = "---.."; m_morseTable['9'] = "----.";
}

/* ---- Standard Morse table accessor ---- */

QMap<QChar, QString> PolluxCode3::standardMorseTable() const
{
    return m_morseTable;
}

/* ---- Set key ---- */

void PolluxCode3::setKey(const QMap<int, Symbol>& keyMapping)
{
    m_key = keyMapping;
}

/* ---- Text to Morse sequence ---- */

QString PolluxCode3::textToMorse(const QString& text) const
{
    QString result;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar ch = upper[i];
        if (ch == ' ') {
            result.append('W');  // Word gap marker
        } else if (m_morseTable.contains(ch)) {
            result.append(m_morseTable[ch]);
            if (i + 1 < upper.size() && upper[i + 1] != ' ')
                result.append('L');  // Letter gap
        }
    }
    return result;
}

/* ---- Encrypt ---- */

QString PolluxCode3::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString morse = textToMorse(plaintext);
    QString cipher;

    // Map each Morse element to a digit via key
    for (int i = 0; i < morse.size(); ++i) {
        QChar ch = morse[i];
        Symbol sym;
        if (ch == '.') sym = Dot;
        else if (ch == '-') sym = Dash;
        else if (ch == 'L') sym = LetterGap;
        else sym = WordGap;

        // Find digits mapping to this symbol
        QVector<int> candidates;
        for (auto it = m_key.begin(); it != m_key.end(); ++it) {
            if (it.value() == sym) candidates.append(it.key());
        }
        if (!candidates.isEmpty()) {
            cipher.append(QString::number(
                candidates[i % candidates.size()]));
        }
    }

    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = cipher.size();
    m_stats.keyLength = m_key.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("encrypt", cipher.size(), timer.elapsed());
    return cipher;
}

/* ---- Decrypt ---- */

QString PolluxCode3::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Convert digits to Morse symbols
    QString morse;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int digit = ciphertext[i].digitValue();
        if (m_key.contains(digit)) {
            Symbol sym = m_key[digit];
            switch (sym) {
            case Dot:       morse.append('.'); break;
            case Dash:      morse.append('-'); break;
            case LetterGap: morse.append('L'); break;
            case WordGap:   morse.append('W'); break;
            }
        }
    }

    // Parse Morse back to text
    QString result;
    QString currentLetter;
    QString currentMorse;

    for (int i = 0; i < morse.size(); ++i) {
        QChar ch = morse[i];
        if (ch == 'L' || ch == 'W') {
            // Look up current morse code
            for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it) {
                if (it.value() == currentMorse) {
                    result.append(it.key());
                    break;
                }
            }
            currentMorse.clear();
            if (ch == 'W') result.append(' ');
        } else {
            currentMorse.append(ch);
        }
    }
    // Last letter
    if (!currentMorse.isEmpty()) {
        for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it) {
            if (it.value() == currentMorse) {
                result.append(it.key());
                break;
            }
        }
    }

    m_stats.inputLength = ciphertext.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Symbol frequency analysis ---- */

QVector<double> PolluxCode3::symbolFrequencies(const QString& ciphertext) const
{
    QVector<double> freq(10, 0.0);
    int total = ciphertext.size();
    if (total == 0) return freq;
    for (int i = 0; i < total; ++i) {
        int d = ciphertext[i].digitValue();
        if (d >= 0 && d < 10) freq[d]++;
    }
    for (int i = 0; i < 10; ++i) freq[i] /= total;
    return freq;
}

/* ---- Bayesian score ---- */

double PolluxCode3::bayesianScore(const QMap<int, Symbol>& hypothesis,
                                   const QString& ciphertext) const
{
    // Expected Morse ratios: dots ~45%, dashes ~35%, gaps ~20%
    double expectedDot = 0.40;
    double expectedDash = 0.35;
    double expectedLetterGap = 0.15;
    double expectedWordGap = 0.10;

    QVector<double> freq = symbolFrequencies(ciphertext);
    double score = 0.0;

    for (auto it = hypothesis.begin(); it != hypothesis.end(); ++it) {
        double f = freq[it.key()];
        double expected = 0.0;
        switch (it.value()) {
        case Dot:       expected = expectedDot; break;
        case Dash:      expected = expectedDash; break;
        case LetterGap: expected = expectedLetterGap; break;
        case WordGap:   expected = expectedWordGap; break;
        }
        // Log-likelihood with Dirichlet prior
        if (f > 0.0 && expected > 0.0)
            score += f * qLn(expected);
    }
    return score;
}

/* ---- Bayesian key inference ---- */

QMap<int, PolluxCode3::Symbol> PolluxCode3::inferKey(
    const QString& ciphertext, const QMap<QChar, QString>& morseTable)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(morseTable)

    // Enumerate possible key assignments (greedy with 10 digits -> 4 symbols)
    QMap<int, Symbol> bestKey;
    double bestScore = -1e18;

    QVector<double> freq = symbolFrequencies(ciphertext);

    // Sort digits by frequency (descending)
    QVector<QPair<double, int>> freqOrder;
    for (int i = 0; i < 10; ++i)
        freqOrder.append({freq[i], i});
    std::sort(freqOrder.begin(), freqOrder.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Assign most frequent digits to most common symbols (dot > dash > gap)
    QVector<Symbol> symbolOrder = {Dot, Dash, LetterGap, WordGap};
    QVector<double> expectedRatio = {0.40, 0.35, 0.15, 0.10};

    int symIdx = 0;
    double accumulated = 0.0;
    for (int i = 0; i < freqOrder.size(); ++i) {
        accumulated += freqOrder[i].first;
        bestKey[freqOrder[i].second] = symbolOrder[symIdx];
        // Move to next symbol when accumulated ratio exceeds threshold
        if (symIdx < 3 && accumulated >= expectedRatio[symIdx]) {
            accumulated = 0.0;
            symIdx++;
        }
    }

    // Refine via iterative Bayesian scoring
    for (int iter = 0; iter < 20; ++iter) {
        bool improved = false;
        for (int d = 0; d < 10; ++d) {
            Symbol origSym = bestKey[d];
            Symbol bestSym = origSym;
            double localBest = bayesianScore(bestKey, ciphertext);

            for (int s = 0; s < 4; ++s) {
                if (s == static_cast<int>(origSym)) continue;
                bestKey[d] = static_cast<Symbol>(s);
                double sc = bayesianScore(bestKey, ciphertext);
                if (sc > localBest) {
                    localBest = sc;
                    bestSym = static_cast<Symbol>(s);
                    improved = true;
                }
            }
            bestKey[d] = bestSym;
        }
        if (!improved) break;
    }

    double finalScore = bayesianScore(bestKey, ciphertext);
    m_stats.keyLength = bestKey.size();
    m_stats.inferenceScore = finalScore;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit keyInferred(bestKey.size(), finalScore, timer.elapsed());
    return bestKey;
}

/* ---- Reset ---- */

void PolluxCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_key.clear();
}
