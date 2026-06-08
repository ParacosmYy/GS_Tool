/**
 * @file MorbitCode3.cpp
 * @brief MorbitCode3 实现
 *
 * 实现Morbit密码：扩展摩尔斯符号频率分析与迭代爬山随机扰动解码。
 */

#include "utils/code227/MorbitCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

MorbitCode3::MorbitCode3(QObject *parent) : QObject(parent)
{
    buildMorseTable();
}

MorbitCode3::~MorbitCode3() = default;

/* ---- Build Morse code table ---- */

void MorbitCode3::buildMorseTable()
{
    // Standard Morse code mapping
    static const QVector<QPair<QChar, QString>> table = {
        {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
        {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
        {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
        {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
        {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
        {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
        {'Y', "-.--"}, {'Z', "--.."}, {'0', "-----"},{'1', ".----"},
        {'2', "..---"},{'3', "...--"},{'4', "....-"},{'5', "....."},
        {'6', "-...."},{'7', "--..."},{'8', "---.."},{'9', "----."}
    };
    for (const auto& p : table) {
        m_morseTable[p.first] = p.second;
        m_morseReverse[p.second] = p.first;
    }
}

/* ---- Set key ---- */

bool MorbitCode3::setKey(const QString& key)
{
    if (key.length() != 9) return false;
    for (int i = 0; i < 9; ++i) {
        if (key[i] < '1' || key[i] > '9') return false;
        for (int j = i + 1; j < 9; ++j)
            if (key[i] == key[j]) return false;
    }
    m_key = key;

    // Build symbol map: digit -> morse pair index
    // Morbit uses 9 cipher symbols mapped to Morse pairs (.., .-, .|, -., --, -|, |., |-, ||)
    static const QString pairs[9] = {
        "..", ".-", ".|", "-.", "--", "-|", "|.", "|-", "||"
    };
    m_morseSymbolMap.clear();
    for (int i = 0; i < 9; ++i)
        m_morseSymbolMap[QChar('1' + i)] = i;
    return true;
}

/* ---- Text to Morse ---- */

QString MorbitCode3::textToMorse(const QString& text) const
{
    QString morse;
    for (int i = 0; i < text.length(); ++i) {
        QChar ch = text[i].toUpper();
        if (m_morseTable.contains(ch)) {
            morse += m_morseTable[ch];
            if (i < text.length() - 1) morse += '|';
        }
    }
    return morse;
}

/* ---- Encrypt ---- */

QString MorbitCode3::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_key.isEmpty()) return {};

    // Morse pair encoding table
    static const QString pairs[9] = {
        "..", ".-", ".|", "-.", "--", "-|", "|.", "|-", "||"
    };

    QString morse = textToMorse(plaintext);
    if (morse.length() % 2 != 0) morse += '|';

    QString cipher;
    for (int i = 0; i < morse.length(); i += 2) {
        QString pair = morse.mid(i, 2);
        int idx = -1;
        for (int j = 0; j < 9; ++j) {
            if (pairs[j] == pair) { idx = j; break; }
        }
        if (idx >= 0)
            cipher += m_key[idx];
    }

    m_stats.numEncryptions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptionCompleted(plaintext.length(), timer.elapsed());
    return cipher;
}

/* ---- Decrypt ---- */

QString MorbitCode3::decrypt(const QString& ciphertext) const
{
    if (m_key.isEmpty()) return {};

    static const QString pairs[9] = {
        "..", ".-", ".|", "-.", "--", "-|", "|.", "|-", "||"
    };

    // Reverse key mapping: digit -> pair index
    QMap<QChar, int> reverseMap;
    for (int i = 0; i < 9; ++i)
        reverseMap[m_key[i]] = i;

    QString morse;
    for (int i = 0; i < ciphertext.length(); ++i) {
        QChar ch = ciphertext[i];
        if (reverseMap.contains(ch)) {
            int idx = reverseMap[ch];
            morse += pairs[idx];
        }
    }

    // Decode Morse to text
    QString result;
    QString current;
    for (int i = 0; i < morse.length(); ++i) {
        if (morse[i] == '|') {
            if (m_morseReverse.contains(current))
                result += m_morseReverse[current];
            current.clear();
        } else {
            current += morse[i];
        }
    }
    if (!current.isEmpty() && m_morseReverse.contains(current))
        result += m_morseReverse[current];

    return result;
}

/* ---- Score plaintext (English frequency heuristic) ---- */

double MorbitCode3::scorePlaintext(const QString& text) const
{
    // Letter frequency scoring for English
    static const QMap<QChar, double> freq = {
        {'E',12.7}, {'T',9.1}, {'A',8.2}, {'O',7.5}, {'I',7.0},
        {'N',6.7},  {'S',6.3}, {'H',6.1}, {'R',6.0}, {'D',4.3},
        {'L',4.0},  {'C',2.8}, {'U',2.8}, {'M',2.4}, {'W',2.4},
        {'F',2.2},  {'G',2.0}, {'Y',2.0}, {'P',1.9}, {'B',1.5},
        {'V',1.0},  {'K',0.8}, {'J',0.2}, {'X',0.2}, {'Q',0.1},
        {'Z',0.1}
    };
    double score = 0.0;
    for (int i = 0; i < text.length(); ++i) {
        QChar ch = text[i].toUpper();
        if (freq.contains(ch))
            score += freq[ch];
    }
    // Bonus for common bigrams
    static const QStringList bigrams = {"TH","HE","IN","ER","AN","RE","ON","AT"};
    for (const auto& bg : bigrams) {
        int count = text.count(bg, Qt::CaseInsensitive);
        score += count * 5.0;
    }
    return score;
}

/* ---- Random key ---- */

QString MorbitCode3::randomKey() const
{
    QString digits = "123456789";
    // Fisher-Yates shuffle
    int seed = 42;
    for (int i = 8; i > 0; --i) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        int j = seed % (i + 1);
        QChar tmp = digits[i];
        digits[i] = digits[j];
        digits[j] = tmp;
    }
    return digits;
}

/* ---- Perturb key ---- */

QString MorbitCode3::perturbKey(const QString& key) const
{
    QString result = key;
    int seed = qrand();
    int i = seed % 9;
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    int j = seed % 9;
    QChar tmp = result[i];
    result[i] = result[j];
    result[j] = tmp;
    return result;
}

/* ---- Frequency analysis ---- */

QMap<QChar, int> MorbitCode3::frequencyAnalysis(const QString& ciphertext) const
{
    QMap<QChar, int> freq;
    for (int i = 0; i < ciphertext.length(); ++i) {
        QChar ch = ciphertext[i];
        if (ch >= '1' && ch <= '9')
            freq[ch]++;
    }
    return freq;
}

/* ---- Crack via hill-climbing ---- */

MorbitCode3::DecodeResult MorbitCode3::crack(const QString& ciphertext,
                                                int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult best;
    best.key = randomKey();
    setKey(best.key);
    best.plaintext = decrypt(ciphertext);
    best.score = scorePlaintext(best.plaintext);

    int noImprove = 0;
    for (int iter = 0; iter < maxIter; ++iter) {
        QString newKey = perturbKey(best.key);
        setKey(newKey);
        QString pt = decrypt(ciphertext);
        double sc = scorePlaintext(pt);

        if (sc > best.score) {
            best.score = sc;
            best.key = newKey;
            best.plaintext = pt;
            best.iterations = iter;
            noImprove = 0;
            emit crackProgress(iter, sc);
        } else {
            noImprove++;
        }

        // Random restart after stagnation
        if (noImprove > 200) {
            QString restartKey = randomKey();
            setKey(restartKey);
            QString rpt = decrypt(ciphertext);
            double rsc = scorePlaintext(rpt);
            if (rsc > best.score) {
                best.score = rsc;
                best.key = restartKey;
                best.plaintext = rpt;
                best.iterations = iter;
            }
            noImprove = 0;
        }
    }

    setKey(best.key);
    m_stats.numDecryptions++;
    m_stats.bestScoreIterations = best.iterations;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit crackCompleted(best.score, best.iterations);
    return best;
}

/* ---- Reset ---- */

void MorbitCode3::resetStatistics()
{
    m_key.clear();
    m_morseSymbolMap.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
