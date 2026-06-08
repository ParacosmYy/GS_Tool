/**
 * @file FractionatedMorse3.cpp
 * @brief FractionatedMorse3 实现
 *
 * 实现分数摩尔斯：扩展Trigraph映射、爬山随机重启密钥搜索。
 */

#include "utils/code223/FractionatedMorse3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

FractionatedMorse3::FractionatedMorse3(QObject *parent) : QObject(parent)
{
    buildMorseTable();
    setKey("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

FractionatedMorse3::~FractionatedMorse3() = default;

/* ---- Build Morse code table ---- */

void FractionatedMorse3::buildMorseTable()
{
    // Standard Morse code mapping (letter -> morse)
    m_letterToMorse['A'] = ".-";    m_letterToMorse['B'] = "-...";
    m_letterToMorse['C'] = "-.-.";  m_letterToMorse['D'] = "-..";
    m_letterToMorse['E'] = ".";     m_letterToMorse['F'] = "..-.";
    m_letterToMorse['G'] = "--.";   m_letterToMorse['H'] = "....";
    m_letterToMorse['I'] = "..";    m_letterToMorse['J'] = ".---";
    m_letterToMorse['K'] = "-.-";   m_letterToMorse['L'] = ".-..";
    m_letterToMorse['M'] = "--";    m_letterToMorse['N'] = "-.";
    m_letterToMorse['O'] = "---";   m_letterToMorse['P'] = ".--.";
    m_letterToMorse['Q'] = "--.-";  m_letterToMorse['R'] = ".-.";
    m_letterToMorse['S'] = "...";   m_letterToMorse['T'] = "-";
    m_letterToMorse['U'] = "..-";   m_letterToMorse['V'] = "...-";
    m_letterToMorse['W'] = ".--";   m_letterToMorse['X'] = "-..-";
    m_letterToMorse['Y'] = "-.--";  m_letterToMorse['Z'] = "--..";

    // Reverse mapping
    for (auto it = m_letterToMorse.begin(); it != m_letterToMorse.end(); ++it)
        m_morseToLetter[it.value()] = it.key();
}

/* ---- Build trigraph substitution table ---- */

void FractionatedMorse3::buildTrigraphTable()
{
    // Generate all 26 valid trigraphs from Morse: sequences of '.', '-', 'x'
    // Extended trigraph: all 3-char combos of '.', '-', 'x' that map to 26 letters
    // Standard fractionated Morse uses the 26 valid trigraphs
    Q_UNUSED(m_key);
}

/* ---- Set key ---- */

void FractionatedMorse3::setKey(const QString& key)
{
    QString clean;
    for (QChar c : key.toUpper())
        if (c >= 'A' && c <= 'Z') clean.append(c);
    if (clean.length() < 26) {
        // Pad missing letters
        for (char c = 'A'; c <= 'Z'; ++c)
            if (!clean.contains(c)) clean.append(c);
    }
    m_key = clean.left(26);
    buildTrigraphTable();
}

/* ---- Text to Morse ---- */

QString FractionatedMorse3::textToMorse(const QString& text) const
{
    QString morse;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar c = upper[i];
        if (m_letterToMorse.contains(c)) {
            if (!morse.isEmpty()) morse += 'x'; // Letter separator
            morse += m_letterToMorse[c];
        } else if (c == ' ') {
            morse += "xx"; // Word separator
        }
    }
    return morse;
}

/* ---- Morse to text ---- */

QString FractionatedMorse3::morseToText(const QString& morse) const
{
    QString result;
    QStringList letters = morse.split("xx", Qt::SkipEmptyParts);
    for (const QString& letterGroup : letters) {
        QStringList symbols = letterGroup.split('x', Qt::SkipEmptyParts);
        for (const QString& sym : symbols) {
            if (m_morseToLetter.contains(sym))
                result += m_morseToLetter[sym];
        }
        result += ' ';
    }
    return result.trimmed();
}

/* ---- Encrypt ---- */

FractionatedMorse3::CipherResult FractionatedMorse3::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Convert plaintext to Morse code with 'x' separator
    QString morse = textToMorse(plaintext);

    // Pad Morse to multiple of 3 for trigraph mapping
    while (morse.length() % 3 != 0) morse += 'x';

    // Map each trigraph to a letter from the key
    QString cipher;
    for (int i = 0; i < morse.length(); i += 3) {
        QString trigraph = morse.mid(i, 3);
        // Map trigraph to index: treat '.', '-', 'x' as trits (0,1,2)
        int index = 0;
        for (int j = 0; j < 3; ++j) {
            int trit = (trigraph[j] == '.') ? 0 : (trigraph[j] == '-') ? 1 : 2;
            index = index * 3 + trit;
        }
        index = index % 26;
        cipher += m_key[index];
    }

    CipherResult result;
    result.text = cipher;
    result.key = m_key;
    m_stats.encryptOps++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("encrypt", timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

FractionatedMorse3::CipherResult FractionatedMorse3::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Map each cipher letter back to trigraph via key position
    QString morse;
    for (QChar c : ciphertext.toUpper()) {
        int idx = m_key.indexOf(c);
        if (idx < 0) continue;
        // Convert index back to 3 trits
        for (int j = 2; j >= 0; --j) {
            int trit = idx % 3;
            idx /= 3;
            morse.prepend(trit == 0 ? '.' : (trit == 1 ? '-' : 'x'));
        }
    }

    QString plain = morseToText(morse);

    CipherResult result;
    result.text = plain;
    result.key = m_key;
    m_stats.decryptOps++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("decrypt", timer.elapsed());
    return result;
}

/* ---- English fitness (quadgram approximation) ---- */

double FractionatedMorse3::englishFitness(const QString& text) const
{
    double score = 0.0;
    QString clean;
    for (QChar c : text.toUpper())
        if (c >= 'A' && c <= 'Z') clean.append(c);

    // Log-frequency scoring using letter frequencies
    static const double freq[26] = {
        8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094, 6.966,
        0.153, 0.772, 4.025, 2.406, 6.749, 7.507, 1.929, 0.095, 5.987,
        6.327, 9.056, 2.758, 0.978, 2.361, 0.150, 1.974, 0.074
    };
    for (int i = 0; i < clean.size(); ++i) {
        int idx = clean[i].toLatin1() - 'A';
        if (idx >= 0 && idx < 26) score += qLn(qMax(freq[idx], 0.01));
    }
    // Bigram bonus for common pairs
    for (int i = 0; i + 1 < clean.size(); ++i) {
        QString pair = clean.mid(i, 2);
        if (pair == "TH" || pair == "HE" || pair == "IN" || pair == "ER")
            score += 2.0;
        if (pair == "AN" || pair == "RE" || pair == "ON" || pair == "AT")
            score += 1.5;
    }
    return score;
}

/* ---- Mutate key ---- */

QString FractionatedMorse3::mutateKey(const QString& key) const
{
    static thread_local std::mt19937 rng(7777);
    std::uniform_int_distribution<int> dist(0, 25);
    QString mutated = key;
    int a = dist(rng), b = dist(rng);
    if (a != b) {
        QChar tmp = mutated[a];
        mutated[a] = mutated[b];
        mutated[b] = tmp;
    }
    return mutated;
}

/* ---- Random key ---- */

QString FractionatedMorse3::randomKey() const
{
    static thread_local std::mt19937 rng(9999);
    QString alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 25; i > 0; --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        std::swap(alpha[i], alpha[j]);
    }
    return alpha;
}

/* ---- Crack ---- */

FractionatedMorse3::CipherResult FractionatedMorse3::crack(const QString& ciphertext,
                                                              int maxRestarts,
                                                              int hillClimbSteps)
{
    QElapsedTimer timer;
    timer.start();

    QString bestKey;
    double bestScore = -std::numeric_limits<double>::max();

    for (int restart = 0; restart < maxRestarts; ++restart) {
        QString currentKey = (restart == 0) ? "ABCDEFGHIJKLMNOPQRSTUVWXYZ" : randomKey();
        double currentScore;

        // Hill climbing
        for (int step = 0; step < hillClimbSteps; ++step) {
            QString candidate = mutateKey(currentKey);
            // Evaluate candidate key
            QString savedKey = m_key;
            m_key = candidate;
            CipherResult decResult = decrypt(ciphertext);
            m_key = savedKey;

            double candidateScore = englishFitness(decResult.text);
            if (candidateScore > currentScore || step == 0) {
                currentScore = candidateScore;
                currentKey = candidate;
            }
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            bestKey = currentKey;
        }
    }

    // Final decrypt with best key
    QString savedKey = m_key;
    m_key = bestKey;
    CipherResult finalResult = decrypt(ciphertext);
    m_key = savedKey;

    finalResult.key = bestKey;
    finalResult.score = bestScore;
    finalResult.restarts = maxRestarts;

    m_stats.crackOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("crack", timer.elapsed());
    return finalResult;
}

/* ---- Reset ---- */

void FractionatedMorse3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
