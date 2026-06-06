/**
 * @file VigenereCode.cpp
 * @brief VigenereCode 实现
 *
 * 实现维吉尼亚密码：加密解密、Kasiski检验、Friedman重合指数、频率分析破解。
 */

#include "utils/code174/VigenereCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* English letter frequency distribution */
const double VigenereCode::s_englishFreq[26] = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
};

/* ---- Construction / Destruction ---- */

VigenereCode::VigenereCode(QObject *parent)
    : QObject(parent)
{
}

VigenereCode::~VigenereCode() = default;

/* ---- Configuration ---- */

void VigenereCode::setKey(const QString& key)
{
    m_key = key.toUpper();
    QString cleaned;
    for (QChar c : m_key) {
        if (c >= 'A' && c <= 'Z') cleaned.append(c);
    }
    m_key = cleaned;
}

/* ---- Helpers ---- */

int VigenereCode::charToIndex(QChar c)
{
    if (c >= 'A' && c <= 'Z') return c.unicode() - 'A';
    if (c >= 'a' && c <= 'z') return c.unicode() - 'a';
    return -1;
}

QChar VigenereCode::indexToChar(int idx)
{
    return QChar('A' + qBound(0, idx % 26, 25));
}

QString VigenereCode::cleanText(const QString& text)
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c >= 'A' && c <= 'Z') result.append(c);
    }
    return result;
}

int VigenereCode::gcd(int a, int b)
{
    a = qAbs(a); b = qAbs(b);
    while (b) { int t = b; b = a % b; a = t; }
    return a;
}

QVector<int> VigenereCode::getFactors(int n)
{
    QVector<int> factors;
    if (n <= 0) return factors;
    for (int i = 2; i <= qSqrt(n); ++i) {
        if (n % i == 0) {
            factors.append(i);
            if (i != n / i) factors.append(n / i);
        }
    }
    std::sort(factors.begin(), factors.end());
    return factors;
}

/* ---- Encrypt ---- */

QString VigenereCode::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_key.isEmpty()) return plaintext;
    QString cleaned = cleanText(plaintext);
    QString result;
    int keyLen = m_key.size();
    for (int i = 0; i < cleaned.size(); ++i) {
        int p = charToIndex(cleaned[i]);
        int k = charToIndex(m_key[i % keyLen]);
        if (p >= 0 && k >= 0)
            result.append(indexToChar((p + k) % 26));
    }

    m_stats.totalEncryptions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions + m_stats.totalAnalyses);

    emit encryptionCompleted(result.size());
    return result;
}

/* ---- Decrypt ---- */

QString VigenereCode::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_key.isEmpty()) return ciphertext;
    QString cleaned = cleanText(ciphertext);
    QString result;
    int keyLen = m_key.size();
    for (int i = 0; i < cleaned.size(); ++i) {
        int c = charToIndex(cleaned[i]);
        int k = charToIndex(m_key[i % keyLen]);
        if (c >= 0 && k >= 0)
            result.append(indexToChar((c - k + 26) % 26));
    }

    m_stats.totalDecryptions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions + m_stats.totalAnalyses);

    emit decryptionCompleted(result.size());
    return result;
}

/* ---- Kasiski examination ---- */

QVector<QPair<int, int>> VigenereCode::findRepeats(const QString& text,
                                                     int seqLen) const
{
    QMap<QString, QVector<int>> positions;
    int n = text.size();

    for (int i = 0; i <= n - seqLen; ++i) {
        QString seq = text.mid(i, seqLen);
        positions[seq].append(i);
    }

    QVector<QPair<int, int>> spacings; /* (spacing, count) */
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        if (it.value().size() < 2) continue;
        for (int i = 0; i < it.value().size() - 1; ++i) {
            int spacing = it.value()[i + 1] - it.value()[i];
            spacings.append({spacing, 1});
        }
    }
    return spacings;
}

VigenereCode::KeyLengthResult VigenereCode::kasiskiExamination(
    const QString& ciphertext) const
{
    KeyLengthResult result;
    QString cleaned = cleanText(ciphertext);
    int n = cleaned.size();
    if (n < 6) return result;

    /* Find repeats of length 3, 4, 5 */
    QMap<int, int> factorFreq;
    for (int seqLen = 3; seqLen <= qMin(5, n / 2); ++seqLen) {
        auto spacings = findRepeats(cleaned, seqLen);
        for (const auto& sp : spacings) {
            auto factors = getFactors(sp.first);
            for (int f : factors) {
                if (f >= 2 && f <= n / 2)
                    factorFreq[f]++;
            }
        }
    }

    /* Convert to sorted factor list */
    for (auto it = factorFreq.begin(); it != factorFreq.end(); ++it)
        result.kasiskiFactors.append({it.key(), it.value()});
    std::sort(result.kasiskiFactors.begin(), result.kasiskiFactors.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (!result.kasiskiFactors.isEmpty())
        result.keyLength = result.kasiskiFactors.first().first;
    result.friedmanIC = friedmanTest(cleaned);
    return result;
}

/* ---- Friedman test (Index of Coincidence) ---- */

double VigenereCode::friedmanTest(const QString& text) const
{
    QString cleaned = cleanText(text);
    int n = cleaned.size();
    if (n <= 1) return 0.0;

    int counts[26] = {};
    for (QChar c : cleaned) {
        int idx = charToIndex(c);
        if (idx >= 0) counts[idx]++;
    }

    double ic = 0.0;
    for (int i = 0; i < 26; ++i)
        ic += static_cast<double>(counts[i]) * (counts[i] - 1);

    return ic / (static_cast<double>(n) * (n - 1));
}

/* ---- Combined key length analysis ---- */

VigenereCode::KeyLengthResult VigenereCode::analyzeKeyLength(
    const QString& ciphertext) const
{
    auto result = kasiskiExamination(ciphertext);
    result.friedmanIC = friedmanTest(ciphertext);

    /* Use Friedman IC to estimate key length */
    double ic = result.friedmanIC;
    int n = cleanText(ciphertext).size();
    if (ic > 0.0 && n > 1) {
        double estimatedK = (0.0667 - 0.0385) / (ic - 0.0385);
        int friedmanKeyLen = qBound(1, static_cast<int>(qRound(estimatedK)), n);
        /* Cross-validate with Kasiski */
        if (result.keyLength == 0) result.keyLength = friedmanKeyLen;
    }
    return result;
}

/* ---- Frequency distribution ---- */

QMap<QChar, double> VigenereCode::frequencyDistribution(const QString& text)
{
    QMap<QChar, double> freq;
    int counts[26] = {};
    int total = 0;
    for (QChar c : text.toUpper()) {
        int idx = charToIndex(c);
        if (idx >= 0) { counts[idx]++; total++; }
    }
    if (total == 0) return freq;
    for (int i = 0; i < 26; ++i)
        freq[QChar('A' + i)] = static_cast<double>(counts[i]) / total;
    return freq;
}

/* ---- Crack key via frequency analysis ---- */

QString VigenereCode::crackKey(const QString& ciphertext, int keyLength) const
{
    QString cleaned = cleanText(ciphertext);
    int n = cleaned.size();
    if (keyLength <= 0 || n == 0) return {};

    QString key;
    for (int k = 0; k < keyLength; ++k) {
        /* Extract k-th column */
        QString column;
        for (int i = k; i < n; i += keyLength)
            column.append(cleaned[i]);

        /* Find best shift by correlating with English frequency */
        double bestCorr = -1e30;
        int bestShift = 0;
        for (int shift = 0; shift < 26; ++shift) {
            double corr = 0.0;
            int colLen = column.size();
            int counts[26] = {};
            for (QChar c : column) counts[charToIndex(c)]++;

            for (int i = 0; i < 26; ++i) {
                double observed = static_cast<double>(counts[(i + shift) % 26]) / colLen;
                corr += observed * s_englishFreq[i];
            }
            if (corr > bestCorr) {
                bestCorr = corr;
                bestShift = shift;
            }
        }
        key.append(indexToChar(bestShift));
    }
    return key;
}

/* ---- Auto crack ---- */

QString VigenereCode::autoCrack(const QString& ciphertext) const
{
    auto analysis = analyzeKeyLength(ciphertext);
    if (analysis.keyLength <= 0) return {};

    QString key = crackKey(ciphertext, analysis.keyLength);

    /* Temporarily set key and decrypt */
    VigenereCode temp;
    temp.setKey(key);
    QString result = temp.decrypt(ciphertext);

    m_stats.totalAnalyses++;
    emit analysisCompleted(analysis.keyLength, analysis.friedmanIC);
    return result;
}

/* ---- Statistics ---- */

void VigenereCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
