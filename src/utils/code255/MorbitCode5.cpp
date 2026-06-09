/**
 * @file MorbitCode5.cpp
 * @brief MorbitCode5 实现
 *
 * 实现Morbit密码：穷举密钥搜索剪枝与摩尔斯三元组频率评分自动解密。
 */

#include "utils/code255/MorbitCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

MorbitCode5::MorbitCode5(QObject *parent)
    : QObject(parent)
{
    buildMorseTable();
    buildTrigramFreq();
}
MorbitCode5::~MorbitCode5() = default;

/* ---- Configuration ---- */

void MorbitCode5::setCiphertext(const QString& cipher) { m_cipher = cipher; }
void MorbitCode5::setMaxCandidates(int max) { m_maxCandidates = qMax(100, max); }
void MorbitCode5::setScoreThreshold(double threshold) { m_scoreThreshold = threshold; }

/* ---- Build Morse code lookup ---- */

void MorbitCode5::buildMorseTable()
{
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
}

/* ---- Build English Morse trigram frequency ---- */

void MorbitCode5::buildTrigramFreq()
{
    // Common English letter trigram frequencies (relative)
    m_trigramFreq["THE"] = 3.5;
    m_trigramFreq["AND"] = 1.5;
    m_trigramFreq["ING"] = 1.4;
    m_trigramFreq["HER"] = 1.0;
    m_trigramFreq["ERE"] = 0.9;
    m_trigramFreq["ENT"] = 0.9;
    m_trigramFreq["THA"] = 0.8;
    m_trigramFreq["NTH"] = 0.8;
    m_trigramFreq["WAS"] = 0.7;
    m_trigramFreq["ETH"] = 0.7;
    m_trigramFreq["FOR"] = 0.6;
    m_trigramFreq["DTH"] = 0.5;
    m_trigramFreq["HAT"] = 0.5;
    m_trigramFreq["HIS"] = 0.5;
    m_trigramFreq["STH"] = 0.4;
    m_trigramFreq["ITH"] = 0.4;
    m_trigramFreq["FTH"] = 0.4;
    m_trigramFreq["ALL"] = 0.4;
    m_trigramFreq["NOT"] = 0.4;
    m_trigramFreq["BUT"] = 0.3;
}

/* ---- Generate key permutations ---- */

QVector<QString> MorbitCode5::generateKeys() const
{
    // Morbit uses digits 1-9 mapped to Morse pairs (., -, .-, -., .-, etc.)
    // Generate permutations of 9 digits for keys
    QVector<QString> keys;
    QString digits = "123456789";

    // For practicality, generate subset permutations
    std::mt19937 rng(42);
    QVector<QString> allPerms;

    // Generate limited set of key permutations
    int keyLen = qMin(9, static_cast<int>(m_cipher.length() / 2 + 1));
    keyLen = qMax(3, keyLen);

    QString base;
    for (int i = 1; i <= keyLen; ++i)
        base.append(QChar('0' + i));

    // Generate permutations via Fisher-Yates shuffling
    for (int trial = 0; trial < m_maxCandidates; ++trial) {
        QString perm = base;
        for (int i = perm.length() - 1; i > 0; --i) {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(rng);
            std::swap(perm[i], perm[j]);
        }
        allPerms.append(perm);
    }

    // Remove duplicates
    std::sort(allPerms.begin(), allPerms.end());
    allPerms.erase(std::unique(allPerms.begin(), allPerms.end()), allPerms.end());
    return allPerms;
}

/* ---- Digit pair to Morse substring ---- */

QString MorbitCode5::digitPairToMorse(const QString& pair) const
{
    // Morbit maps digit pairs to Morse elements:
    // 1=.., 2=.-, 3=-., 4=--, 5=., 6=-, 7=..., 8=..-, 9=.-.
    static const QMap<QChar, QString> mapping = {
        {'1', ".."}, {'2', ".-"}, {'3', "-."}, {'4', "--"},
        {'5', "."},  {'6', "-"},  {'7', "..."},{'8', "..-"},
        {'9', ".-."}
    };
    QString result;
    for (const QChar& c : pair) {
        if (mapping.contains(c))
            result += mapping[c];
    }
    return result;
}

/* ---- Morse string to text ---- */

QString MorbitCode5::morseToText(const QString& morse) const
{
    // Build reverse lookup
    QMap<QString, QChar> reverseMap;
    for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it)
        reverseMap[it.value()] = it.key();

    QString result;
    // Split by space groups (3 spaces = letter boundary, 1 space = element)
    QStringList letters = morse.split("   ", Qt::SkipEmptyParts);
    for (const QString& letter : letters) {
        QString trimmed = letter.trimmed();
        if (reverseMap.contains(trimmed))
            result += reverseMap[trimmed];
        else
            result += '?';
    }
    return result;
}

/* ---- Prune keys by partial evaluation ---- */

QVector<QString> MorbitCode5::pruneKeys(const QVector<QString>& keys) const
{
    QVector<QString> pruned;
    for (const QString& key : keys) {
        // Quick partial decrypt: first 20% of ciphertext
        int partialLen = qMax(4, m_cipher.length() / 5);
        QString partial = m_cipher.left(partialLen);

        // Convert using key
        QString morse;
        for (int i = 0; i + 1 < partial.length(); i += 2) {
            QChar d1 = partial[i];
            QChar d2 = partial[i + 1];
            // Map digit through key permutation
            int idx1 = d1.digitValue() - 1;
            int idx2 = d2.digitValue() - 1;
            if (idx1 >= 0 && idx1 < key.length() && idx2 >= 0 && idx2 < key.length()) {
                QString pair;
                pair += key[idx1];
                pair += key[idx2];
                morse += digitPairToMorse(pair);
                morse += " ";
            }
        }

        // Partial score: check for common letter patterns
        double partialScore = 0.0;
        QString partialText = morseToText(morse);
        for (int i = 0; i + 2 < partialText.length(); ++i) {
            QString trig = partialText.mid(i, 3).toUpper();
            if (m_trigramFreq.contains(trig))
                partialScore += m_trigramFreq[trig];
        }

        if (partialScore >= m_scoreThreshold || pruned.size() < 100)
            pruned.append(key);
    }
    return pruned;
}

/* ---- Score plaintext with trigram frequencies ---- */

double MorbitCode5::scorePlaintext(const QString& text) const
{
    double score = 0.0;
    QString upper = text.toUpper();
    for (int i = 0; i + 2 < upper.length(); ++i) {
        QString trig = upper.mid(i, 3);
        if (m_trigramFreq.contains(trig))
            score += m_trigramFreq[trig];
    }
    // Bonus for common single letters
    for (const QChar& c : upper) {
        if (c == 'E') score += 0.13;
        else if (c == 'T') score += 0.09;
        else if (c == 'A') score += 0.08;
        else if (c == 'O') score += 0.07;
        else if (c == 'I') score += 0.07;
    }
    return score;
}

/* ---- Decrypt with known key ---- */

QString MorbitCode5::decryptWithKey(const QString& key) const
{
    QString morse;
    for (int i = 0; i + 1 < m_cipher.length(); i += 2) {
        QChar d1 = m_cipher[i];
        QChar d2 = m_cipher[i + 1];
        int idx1 = d1.digitValue() - 1;
        int idx2 = d2.digitValue() - 1;
        if (idx1 >= 0 && idx1 < key.length() && idx2 >= 0 && idx2 < key.length()) {
            QString pair;
            pair += key[idx1];
            pair += key[idx2];
            morse += digitPairToMorse(pair);
            morse += " ";
        }
    }
    return morseToText(morse);
}

/* ---- Main decrypt: exhaustive search with pruning ---- */

MorbitCode5::Result MorbitCode5::decrypt()
{
    QElapsedTimer timer;
    timer.start();

    Result best;
    best.score = -1e30;

    QVector<QString> keys = generateKeys();
    keys = pruneKeys(keys);

    m_stats.numCandidates = keys.size();

    for (const QString& key : keys) {
        QString plaintext = decryptWithKey(key);
        double score = scorePlaintext(plaintext);

        if (score > best.score) {
            best.score = score;
            best.plaintext = plaintext;
            best.key = key;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.bestScore = best.score;
    m_stats.keyLength = best.key.length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(best.score, elapsed);
    return best;
}

/* ---- Reset ---- */

void MorbitCode5::resetStatistics()
{
    m_cipher.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
