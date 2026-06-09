/**
 * @file FractionatedMorse5.cpp
 * @brief FractionatedMorse5 实现
 *
 * 实现分数摩尔斯密码：变长摩尔斯元素组与维特比网格概率解密。
 */

#include "utils/code251/FractionatedMorse5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FractionatedMorse5::FractionatedMorse5(QObject *parent) : QObject(parent)
{
    initMorseTable();
    buildTrigramMap();
}

FractionatedMorse5::~FractionatedMorse5() = default;

/* ---- Configuration ---- */

void FractionatedMorse5::setAlphabet(const QString& alphabet)
{
    m_alphabet = alphabet;
    buildTrigramMap();
}

void FractionatedMorse5::setTransitionProb(double p)
{
    m_transProb = qBound(0.1, p, 0.99);
}

/* ---- Initialize Morse code table ---- */

void FractionatedMorse5::initMorseTable()
{
    // Standard International Morse Code
    m_morseTable['A'] = ".-";    m_morseTable['B'] = "-...";
    m_morseTable['C'] = "-.-.";  m_morseTable['D'] = "-..";
    m_morseTable['E'] = ".";     m_morseTable['F'] = "..-.";
    m_morseTable['G'] = "--.";   m_morseTable['H'] = "....";
    m_morseTable['I'] = "..";    m_morseTable['J'] = ".---";
    m_morseTable['K'] = "-.-";   m_morseTable['L'] = ".-..";
    m_morseTable['M'] = "--";    m_morseTable['N'] = "-.";
    m_morseTable['O'] = "---";   m_morseTable['P'] = ".--.";
    m_morseTable['Q'] = "--.-";  m_morseTable['R'] = ".-.";
    m_morseTable['S'] = "...";   m_morseTable['T'] = "-";
    m_morseTable['U'] = "..-";   m_morseTable['V'] = "...-";
    m_morseTable['W'] = ".--";   m_morseTable['X'] = "-..-";
    m_morseTable['Y'] = "-.--";  m_morseTable['Z'] = "--..";
    m_morseTable['0'] = "-----"; m_morseTable['1'] = ".----";
    m_morseTable['2'] = "..---"; m_morseTable['3'] = "...--";
    m_morseTable['4'] = "....-"; m_morseTable['5'] = ".....";
    m_morseTable['6'] = "-...."; m_morseTable['7'] = "--...";
    m_morseTable['8'] = "---.."; m_morseTable['9'] = "----.";

    // Build reverse lookup
    for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it)
        m_reverseMorse[it.value()] = it.key();
}

/* ---- Build trigram-to-letter mapping ---- */

void FractionatedMorse5::buildTrigramMap()
{
    // Fractionated Morse maps each 3-element group (. - x) to a letter
    // 3^3 = 27 possible trigrams, map to 26 letters + space
    QStringList trigrams = {
        "...", "..-", "..x", ".-.", ".--", ".-x", ".x.", ".x-", ".xx",
        "-..", "-.-", "-.x", "--.", "---", "--x", "-x.", "-x-", "-xx",
        "x..", "x.-", "x.x", "x-.", "x--", "x-x", "xx.", "xx-", "xxx"
    };
    QString letters = m_alphabet.isEmpty()
        ? QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ ")
        : m_alphabet;

    m_trigramMap.clear();
    m_reverseTrigram.clear();
    int limit = qMin(trigrams.size(), letters.size());
    for (int i = 0; i < limit; ++i) {
        m_trigramMap[trigrams[i]] = letters[i];
        m_reverseTrigram[letters[i]] = trigrams[i];
    }
}

/* ---- Convert text to Morse ---- */

QString FractionatedMorse5::toMorse(const QString& text) const
{
    QString morse;
    for (const QChar& ch : text.toUpper()) {
        if (m_morseTable.contains(ch)) {
            if (!morse.isEmpty()) morse += 'x'; // Letter separator
            morse += m_morseTable[ch];
        }
    }
    return morse;
}

/* ---- Convert Morse to text ---- */

QString FractionatedMorse5::fromMorse(const QString& morse) const
{
    QString text;
    QStringList letters = morse.split('x', Qt::SkipEmptyParts);
    for (const QString& code : letters) {
        if (m_reverseMorse.contains(code))
            text += m_reverseMorse[code];
    }
    return text;
}

/* ---- Morse to trigram letters ---- */

QString FractionatedMorse5::morseToTrigramLetters(const QString& morse) const
{
    // Pad with 'x' to make length multiple of 3
    QString padded = morse;
    while (padded.length() % 3 != 0) padded += 'x';

    QString result;
    for (int i = 0; i < padded.length(); i += 3) {
        QString tri = padded.mid(i, 3);
        if (m_trigramMap.contains(tri))
            result += m_trigramMap[tri];
        else
            result += '?';
    }
    return result;
}

/* ---- Emission log-probability ---- */

double FractionatedMorse5::emissionLogProb(QChar observed, QChar expected) const
{
    if (observed == expected) return qLn(m_transProb);
    return qLn(1.0 - m_transProb);
}

/* ---- Viterbi decode ---- */

QString FractionatedMorse5::viterbiDecode(const QString& ciphertext) const
{
    int n = ciphertext.length();
    if (n == 0) return {};

    // For each position, track best letter and cumulative log-prob
    QVector<double> bestLogProb(26, -std::numeric_limits<double>::max());
    QVector<int> backptr(n * 26, 0);

    // Initialize first position
    QChar firstChar = ciphertext[0].toUpper();
    for (int c = 0; c < 26; ++c) {
        QChar letter('A' + c);
        bestLogProb[c] = emissionLogProb(firstChar, letter);
    }

    // Forward pass through trellis
    for (int pos = 1; pos < n; ++pos) {
        QVector<double> newProb(26, -std::numeric_limits<double>::max());
        QChar obs = ciphertext[pos].toUpper();

        for (int curr = 0; curr < 26; ++curr) {
            double emProb = emissionLogProb(obs, QChar('A' + curr));
            double bestPrev = -std::numeric_limits<double>::max();
            int bestIdx = 0;
            for (int prev = 0; prev < 26; ++prev) {
                double p = bestLogProb[prev] + emProb;
                if (p > bestPrev) { bestPrev = p; bestIdx = prev; }
            }
            newProb[curr] = bestPrev;
            backptr[pos * 26 + curr] = bestIdx;
        }
        bestLogProb = newProb;
    }

    // Backtrace
    int bestFinal = 0;
    double bestFinalProb = bestLogProb[0];
    for (int c = 1; c < 26; ++c) {
        if (bestLogProb[c] > bestFinalProb) {
            bestFinalProb = bestLogProb[c];
            bestFinal = c;
        }
    }

    QString result(n, 'A');
    result[n - 1] = QChar('A' + bestFinal);
    for (int pos = n - 2; pos >= 0; --pos) {
        bestFinal = backptr[(pos + 1) * 26 + bestFinal];
        result[pos] = QChar('A' + bestFinal);
    }
    return result;
}

/* ---- Encrypt ---- */

QString FractionatedMorse5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString morse = toMorse(plaintext);
    QString cipher = morseToTrigramLetters(morse);

    m_stats.numEncryptions++;
    m_stats.inputLength = plaintext.length();
    m_stats.outputLength = cipher.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptCompleted(plaintext.length(), cipher.length(), timer.elapsed());
    return cipher;
}

/* ---- Decrypt ---- */

QString FractionatedMorse5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Use Viterbi trellis for probabilistic decryption
    QString result = viterbiDecode(ciphertext);

    m_stats.numDecryptions++;
    m_stats.inputLength = ciphertext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptCompleted(ciphertext.length(), result.length(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void FractionatedMorse5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
