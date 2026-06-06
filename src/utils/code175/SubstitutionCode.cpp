/**
 * @file SubstitutionCode.cpp
 * @brief SubstitutionCode 实现
 *
 * 实现单表替换密码：加密/解密、频率分析、爬山自动求解。
 */

#include "utils/code175/SubstitutionCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- English letter frequencies (A-Z) ---- */

const double SubstitutionCode::s_englishFreq[26] = {
    0.0817, 0.0150, 0.0278, 0.0425, 0.1270, 0.0223, 0.0202,
    0.0609, 0.0697, 0.0015, 0.0077, 0.0403, 0.0241, 0.0675,
    0.0751, 0.0193, 0.0010, 0.0599, 0.0633, 0.0906, 0.0276,
    0.0098, 0.0236, 0.0015, 0.0197, 0.0007
};

/* ---- Construction / Destruction ---- */

SubstitutionCode::SubstitutionCode(QObject *parent)
    : QObject(parent)
{
    /* Default identity key */
    m_encryptTable.resize(26);
    m_decryptTable.resize(26);
    for (int i = 0; i < 26; ++i) {
        m_encryptTable[i] = i;
        m_decryptTable[i] = i;
    }
}

SubstitutionCode::~SubstitutionCode() = default;

/* ---- Configuration ---- */

void SubstitutionCode::setKey(const QString& key)
{
    /* key is a 26-char permutation of A-Z */
    QString clean = key.toUpper().remove(QRegularExpression("[^A-Z]"));
    if (clean.length() != 26) return;

    m_encryptTable.resize(26);
    m_decryptTable.resize(26);

    for (int i = 0; i < 26; ++i) {
        int mapped = clean[i].toLatin1() - 'A';
        m_encryptTable[i] = mapped;
        m_decryptTable[mapped] = i;
    }
}

/* ---- Apply substitution ---- */

QString SubstitutionCode::applySubstitution(const QString& text,
                                             const QVector<int>& table,
                                             bool encrypt) const
{
    QByteArray result;
    result.reserve(text.size());

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];
        if (ch >= 'A' && ch <= 'Z') {
            int idx = ch.toLatin1() - 'A';
            char out = encrypt ? table[idx] + 'A' : table[idx] + 'A';
            result.append(out);
        } else if (ch >= 'a' && ch <= 'z') {
            int idx = ch.toLatin1() - 'a';
            char out = encrypt ? table[idx] + 'a' : table[idx] + 'a';
            result.append(out);
        } else {
            result.append(ch.toLatin1());
        }
    }
    return QString::fromLatin1(result);
}

/* ---- Encrypt / Decrypt ---- */

QString SubstitutionCode::encrypt(const QString& plaintext) const
{
    QString result = applySubstitution(plaintext, m_encryptTable, true);

    const_cast<SubstitutionCode*>(this)->m_stats.totalEncrypts++;
    emit const_cast<SubstitutionCode*>(this)->encryptCompleted(plaintext.size());
    return result;
}

QString SubstitutionCode::decrypt(const QString& ciphertext) const
{
    QString result = applySubstitution(ciphertext, m_decryptTable, false);

    const_cast<SubstitutionCode*>(this)->m_stats.totalDecrypts++;
    emit const_cast<SubstitutionCode*>(this)->decryptCompleted(ciphertext.size());
    return result;
}

/* ---- Frequency analysis ---- */

QMap<QChar, double> SubstitutionCode::frequencyAnalysis(const QString& text) const
{
    QMap<QChar, double> freq;
    int letterCount = 0;

    for (const QChar& ch : text) {
        if (ch.isLetter()) {
            QChar upper = ch.toUpper();
            freq[upper]++;
            letterCount++;
        }
    }

    if (letterCount > 0) {
        for (auto it = freq.begin(); it != freq.end(); ++it)
            it.value() /= letterCount;
    }
    return freq;
}

/* ---- Bigram score (simplified) ---- */

double SubstitutionCode::bigramScore(const QString& text)
{
    /* Common English bigrams scored by frequency */
    static const QVector<QPair<QString, double>> commonBigrams = {
        {"TH", 3.56}, {"HE", 3.07}, {"IN", 2.43}, {"ER", 2.05},
        {"AN", 1.99}, {"RE", 1.85}, {"ON", 1.76}, {"AT", 1.49},
        {"EN", 1.45}, {"ND", 1.35}, {"TI", 1.34}, {"ES", 1.34},
        {"OR", 1.28}, {"TE", 1.27}, {"OF", 1.17}, {"ED", 1.17},
        {"IS", 1.13}, {"IT", 1.12}, {"AL", 1.09}, {"AR", 1.07},
        {"ST", 1.05}, {"TO", 1.05}, {"NT", 1.04}, {"NG", 0.95},
        {"SE", 0.93}, {"HA", 0.93}, {"AS", 0.87}, {"OU", 0.87},
        {"IO", 0.83}, {"LE", 0.83}, {"VE", 0.83}, {"CO", 0.79},
        {"ME", 0.79}, {"DE", 0.76}, {"HI", 0.76}, {"RI", 0.73},
        {"RO", 0.73}, {"IC", 0.70}, {"NE", 0.69}, {"EA", 0.69},
        {"RA", 0.69}, {"CE", 0.65}
    };

    double score = 0.0;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size() - 1; ++i) {
        QString bi = upper.mid(i, 2);
        for (const auto& pair : commonBigrams) {
            if (pair.first == bi) {
                score += pair.second;
                break;
            }
        }
    }
    return score;
}

/* ---- Score text ---- */

double SubstitutionCode::scoreText(const QString& text) const
{
    return bigramScore(text);
}

/* ---- Frequency guess for initial key ---- */

QVector<int> SubstitutionCode::frequencyGuess(const QString& ciphertext) const
{
    QMap<QChar, double> freq = frequencyAnalysis(ciphertext);

    /* Sort ciphertext letters by frequency (descending) */
    QVector<QPair<QChar, double>> freqList;
    for (auto it = freq.begin(); it != freq.end(); ++it)
        freqList.append({it.key(), it.value()});
    std::sort(freqList.begin(), freqList.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    /* Map most frequent ciphertext letter to most frequent English letter */
    QVector<int> key(26);
    /* English letters sorted by frequency */
    static const int engOrder[] = {4,19,0,14,8,13,18,7,17,3,20,10,5,2,23,
                                    12,22,11,24,6,1,15,21,9,25,16};
    for (int i = 0; i < qMin(freqList.size(), 26); ++i) {
        int cipherLetter = freqList[i].first.toLatin1() - 'A';
        key[cipherLetter] = engOrder[i];
    }
    /* Fill unmapped with remaining */
    QVector<bool> used(26, false);
    for (int i = 0; i < 26; ++i) used[key[i]] = true;
    int nextUnused = 0;
    for (int i = 0; i < 26; ++i) {
        if (key[i] == 0 && !used[0]) { /* might be legitimately 0 */ }
        bool mapped = false;
        for (int j = 0; j < 26; ++j) if (key[j] == i) { mapped = true; break; }
        if (!mapped) {
            for (int j = 0; j < 26; ++j) {
                bool hasMapping = false;
                for (int k = 0; k < 26; ++k) if (key[k] == j) { hasMapping = true; break; }
                if (!hasMapping && key[j] == 0 && j != 0) { key[j] = i; break; }
            }
        }
    }
    return key;
}

/* ---- Swap key positions ---- */

void SubstitutionCode::swapKeyPositions(QVector<int>& key)
{
    int a = QRandomGenerator::global()->bounded(26);
    int b = QRandomGenerator::global()->bounded(26);
    std::swap(key[a], key[b]);
}

/* ---- Auto-solve via hill climbing ---- */

QString SubstitutionCode::autoSolve(const QString& ciphertext, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    /* Initial guess from frequency analysis */
    QVector<int> bestKey = frequencyGuess(ciphertext);
    QVector<int> currentKey = bestKey;

    QVector<int> decTable(26);
    for (int i = 0; i < 26; ++i) decTable[i] = currentKey[i];
    QString bestDecrypted = applySubstitution(ciphertext, decTable, false);
    double bestScore = scoreText(bestDecrypted);

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector<int> candidateKey = bestKey;
        swapKeyPositions(candidateKey);

        for (int i = 0; i < 26; ++i) decTable[i] = candidateKey[i];
        QString decrypted = applySubstitution(ciphertext, decTable, false);
        double score = scoreText(decrypted);

        if (score > bestScore) {
            bestScore = score;
            bestKey = candidateKey;
            bestDecrypted = decrypted;
        }

        if (iter % 500 == 0)
            emit autoSolveProgress(iter, bestScore);
    }

    m_stats.totalAutoSolves++;
    m_stats.lastScore = bestScore;
    m_stats.lastIterations = maxIterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAutoSolves;

    return bestDecrypted;
}

/* ---- Generate random key ---- */

QString SubstitutionCode::generateRandomKey() const
{
    QVector<int> key(26);
    for (int i = 0; i < 26; ++i) key[i] = i;
    std::shuffle(key.begin(), key.end(), std::mt19937(
        QRandomGenerator::global()->generate()));

    QString result;
    for (int i = 0; i < 26; ++i)
        result += QChar('A' + key[i]);
    return result;
}

/* ---- Reset ---- */

void SubstitutionCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
