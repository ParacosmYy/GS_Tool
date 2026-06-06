/**
 * @file NihilistCode.cpp
 * @brief NihilistCode 实现
 *
 * 实现Nihilist密码：Polybius方阵、数字密钥加法、频率分析、已知明文攻击。
 */

#include "utils/code185/NihilistCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NihilistCode::NihilistCode(QObject *parent) : QObject(parent) {}
NihilistCode::~NihilistCode() = default;

/* ---- Configuration ---- */

void NihilistCode::setKeyPhrase(const QString& phrase) { m_keyPhrase = phrase.toUpper(); }

/* ---- Build 5x5 Polybius square (J merged with I) ---- */

void NihilistCode::buildSquare(QVector<QVector<QChar>>& square,
                                QVector<int>& charToNum) const
{
    square = QVector<QVector<QChar>>(5, QVector<QChar>(5));
    charToNum.fill(-1, 26);

    QString alphabet;
    // Key phrase first (dedup), then remaining alphabet
    QVector<bool> used(26, false);
    for (QChar c : m_keyPhrase) {
        if (c.isLetter()) {
            int idx = c.toLatin1() - 'A';
            if (idx == 9) idx = 8; // J -> I
            if (!used[idx]) { used[idx] = true; alphabet.append(QChar('A' + idx)); }
        }
    }
    for (int i = 0; i < 26; ++i) {
        int ci = (i == 9) ? 8 : i; // J -> I
        if (!used[ci]) { used[ci] = true; alphabet.append(QChar('A' + ci)); }
    }

    for (int i = 0; i < qMin(25, alphabet.size()); ++i) {
        int row = i / 5, col = i % 5;
        square[row][col] = alphabet[i];
        int letterIdx = alphabet[i].toLatin1() - 'A';
        charToNum[letterIdx] = 10 * (row + 1) + (col + 1);
    }
}

/* ---- Char to Polybius number ---- */

int NihilistCode::charToNum(QChar c, const QVector<QVector<QChar>>& square) const
{
    int ci = c.toUpper().toLatin1() - 'A';
    if (ci == 9) ci = 8; // J -> I
    for (int r = 0; r < 5; ++r)
        for (int c2 = 0; c2 < 5; ++c2)
            if (square[r][c2].toLatin1() - 'A' == ci)
                return 10 * (r + 1) + (c2 + 1);
    return 0;
}

/* ---- Polybius number to char ---- */

QChar NihilistCode::numToChar(int num, const QVector<QVector<QChar>>& square) const
{
    int row = num / 10 - 1, col = num % 10 - 1;
    if (row < 0 || row >= 5 || col < 0 || col >= 5) return '?';
    return square[row][col];
}

/* ---- Expand key to numeric sequence ---- */

QVector<int> NihilistCode::expandKey(int length, const QVector<QVector<QChar>>& square) const
{
    QVector<int> keyNums;
    for (QChar c : m_keyPhrase) {
        if (c.isLetter()) keyNums.append(charToNum(c, square));
    }
    if (keyNums.isEmpty()) keyNums.append(11);

    QVector<int> expanded;
    for (int i = 0; i < length; ++i)
        expanded.append(keyNums[i % keyNums.size()]);
    return expanded;
}

/* ---- Encrypt ---- */

QVector<int> NihilistCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<QChar>> square;
    QVector<int> charToNumMap;
    buildSquare(square, charToNumMap);

    QString clean;
    for (QChar c : plaintext.toUpper())
        if (c.isLetter()) clean.append(c == QLatin1Char('J') ? QLatin1Char('I') : c);

    auto keyNums = expandKey(clean.size(), square);

    QVector<int> result;
    for (int i = 0; i < clean.size(); ++i) {
        int pNum = charToNum(clean[i], square);
        result.append(pNum + keyNums[i]);
    }

    m_stats.totalOperations++;
    m_stats.textSize = clean.size();
    m_stats.keyLength = qMin(m_keyPhrase.size(), clean.size());
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", result.size());
    return result;
}

/* ---- Decrypt ---- */

QString NihilistCode::decrypt(const QVector<int>& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<QChar>> square;
    QVector<int> charToNumMap;
    buildSquare(square, charToNumMap);

    auto keyNums = expandKey(ciphertext.size(), square);

    QString result;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int pNum = ciphertext[i] - keyNums[i];
        result.append(numToChar(pNum, square));
    }

    m_stats.totalOperations++;
    m_stats.textSize = ciphertext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decrypt", ciphertext.size());
    return result;
}

/* ---- Polybius square export ---- */

QVector<QVector<int>> NihilistCode::polybiusSquare() const
{
    QVector<QVector<QChar>> square;
    QVector<int> charToNumMap;
    const_cast<NihilistCode*>(this)->buildSquare(square, charToNumMap);

    QVector<QVector<int>> result(5, QVector<int>(5));
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            result[r][c] = 10 * (r + 1) + (c + 1);
    return result;
}

/* ---- Index of coincidence ---- */

double NihilistCode::indexOfCoincidence(const QVector<int>& ct, int period) const
{
    int n = ct.size();
    if (period <= 0 || n < period * 2) return 0.0;

    double totalIC = 0.0;
    for (int p = 0; p < period; ++p) {
        QVector<int> group;
        for (int i = p; i < n; i += period) group.append(ct[i]);
        int g = group.size();
        if (g < 2) continue;

        // Count frequency of each value
        double ic = 0.0;
        for (int i = 0; i < g; ++i)
            for (int j = i + 1; j < g; ++j)
                if (group[i] == group[j]) ic += 2.0;
        totalIC += ic / (g * (g - 1.0));
    }
    return totalIC / period;
}

/* ---- Key length analysis ---- */

int NihilistCode::analyzeKeyLength(const QVector<int>& ciphertext) const
{
    int n = ciphertext.size();
    if (n < 4) return 1;

    int bestLen = 1;
    double bestIC = 0.0;
    for (int kl = 1; kl <= qMin(n / 2, 20); ++kl) {
        double ic = indexOfCoincidence(ciphertext, kl);
        if (ic > bestIC) { bestIC = ic; bestLen = kl; }
    }
    return bestLen;
}

/* ---- Frequency analysis ---- */

QVector<double> NihilistCode::frequencyAnalysis(const QVector<int>& ciphertext) const
{
    if (ciphertext.isEmpty()) return {};

    int minVal = *std::min_element(ciphertext.begin(), ciphertext.end());
    int maxVal = *std::max_element(ciphertext.begin(), ciphertext.end());
    int range = maxVal - minVal + 1;
    if (range > 1000) range = 1000;

    QVector<double> freq(range, 0.0);
    for (int v : ciphertext) {
        int idx = v - minVal;
        if (idx >= 0 && idx < range) freq[idx] += 1.0;
    }
    int n = ciphertext.size();
    for (int i = 0; i < range; ++i) freq[i] /= n;
    return freq;
}

/* ---- Known plaintext attack ---- */

QVector<int> NihilistCode::knownPlaintextAttack(const QVector<int>& ciphertext,
                                                  const QString& known) const
{
    QVector<QVector<QChar>> square;
    QVector<int> charToNumMap;
    const_cast<NihilistCode*>(this)->buildSquare(square, charToNumMap);

    QString cleanKnown;
    for (QChar c : known.toUpper())
        if (c.isLetter()) cleanKnown.append(c == QLatin1Char('J') ? QLatin1Char('I') : c);

    QVector<int> partialKey;
    int startMax = ciphertext.size() - cleanKnown.size();
    for (int start = 0; start <= qMax(0, startMax); ++start) {
        QVector<int> candidate;
        bool valid = true;
        for (int i = 0; i < cleanKnown.size() && start + i < ciphertext.size(); ++i) {
            int pNum = charToNum(cleanKnown[i], square);
            int kNum = ciphertext[start + i] - pNum;
            if (kNum < 11 || kNum > 55) { valid = false; break; }
            candidate.append(kNum);
        }
        if (valid && !candidate.isEmpty()) {
            if (partialKey.isEmpty() || candidate.size() > partialKey.size())
                partialKey = candidate;
        }
    }
    return partialKey;
}

/* ---- Reset ---- */

void NihilistCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
