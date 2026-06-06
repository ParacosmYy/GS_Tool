/**
 * @file BeaufortCode.cpp
 * @brief BeaufortCode 实现
 *
 * 实现博福特密码：互惠替代加密/解密、重合指数法密钥检测、自动密钥恢复。
 */

#include "utils/code181/BeaufortCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- English letter frequencies ---- */

const double BeaufortCode::s_englishFreq[26] = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
};

/* ---- Construction / Destruction ---- */

BeaufortCode::BeaufortCode(QObject *parent) : QObject(parent) {}
BeaufortCode::~BeaufortCode() = default;

/* ---- Helper: extract alpha upper ---- */

QString BeaufortCode::toAlphaUpper(const QString& text) const
{
    QString result;
    result.reserve(text.size());
    for (QChar ch : text) {
        if (ch.isLetter())
            result.append(ch.toUpper());
    }
    return result;
}

/* ---- Beaufort single-char transform ---- */

QChar BeaufortCode::beaufortChar(QChar plain, QChar key) const
{
    // Beaufort: C = (K - P) mod 26
    int p = plain.toUpper().toLatin1() - 'A';
    int k = key.toUpper().toLatin1() - 'A';
    if (p < 0 || p > 25 || k < 0 || k > 25) return plain;
    int c = (k - p + 26) % 26;
    return QChar('A' + c);
}

/* ---- Encrypt ---- */

QString BeaufortCode::encrypt(const QString& plaintext, const QString& key) const
{
    QElapsedTimer timer;
    timer.start();

    QString alphaPlain = toAlphaUpper(plaintext);
    QString alphaKey = toAlphaUpper(key);
    if (alphaKey.isEmpty()) return plaintext;

    QString result;
    result.reserve(alphaPlain.size());
    int keyLen = alphaKey.size();
    for (int i = 0; i < alphaPlain.size(); ++i)
        result.append(beaufortChar(alphaPlain[i], alphaKey[i % keyLen]));

    const_cast<BeaufortCode*>(this)->m_stats.totalOperations++;
    const_cast<BeaufortCode*>(this)->m_timeSum += timer.elapsed();
    const_cast<BeaufortCode*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOperations;

    emit const_cast<BeaufortCode*>(this)->operationCompleted("encrypt", result.size());
    return result;
}

/* ---- Decrypt (same as encrypt for Beaufort) ---- */

QString BeaufortCode::decrypt(const QString& ciphertext, const QString& key) const
{
    // Beaufort is reciprocal: decrypt = encrypt
    return encrypt(ciphertext, key);
}

/* ---- Index of Coincidence ---- */

double BeaufortCode::indexOfCoincidence(const QString& text) const
{
    QString alpha = toAlphaUpper(text);
    int n = alpha.size();
    if (n < 2) return 0.0;

    int counts[26] = {};
    for (QChar ch : alpha) {
        int idx = ch.toLatin1() - 'A';
        if (idx >= 0 && idx < 26) counts[idx]++;
    }

    double numerator = 0.0;
    for (int i = 0; i < 26; ++i)
        numerator += static_cast<double>(counts[i]) * (counts[i] - 1);
    double denominator = static_cast<double>(n) * (n - 1);

    return (denominator > 0.0) ? numerator / denominator : 0.0;
}

/* ---- Detect key length ---- */

QVector<QPair<int, double>> BeaufortCode::detectKeyLength(const QString& ciphertext, int maxKeyLen) const
{
    QString alpha = toAlphaUpper(ciphertext);
    QVector<QPair<int, double>> results;

    for (int len = 1; len <= maxKeyLen; ++len) {
        // Split text into len groups and average IC
        double totalIC = 0.0;
        for (int g = 0; g < len; ++g) {
            QString group;
            for (int i = g; i < alpha.size(); i += len)
                group.append(alpha[i]);
            totalIC += indexOfCoincidence(group);
        }
        double avgIC = totalIC / len;
        results.append(qMakePair(len, avgIC));
    }

    // Sort by IC descending (highest IC ~ correct key length)
    std::sort(results.begin(), results.end(),
              [](const QPair<int, double>& a, const QPair<int, double>& b) {
                  return a.second > b.second;
              });

    const_cast<BeaufortCode*>(this)->m_stats.detectedKeyLength = results.isEmpty() ? 0 : results.first().first;
    const_cast<BeaufortCode*>(this)->m_stats.indexOfCoincidence = results.isEmpty() ? 0.0 : results.first().second;

    return results;
}

/* ---- Chi-squared against English ---- */

double BeaufortCode::chiSquared(const QVector<int>& counts, int total) const
{
    if (total == 0) return 1e18;
    double chi = 0.0;
    for (int i = 0; i < 26; ++i) {
        double expected = s_englishFreq[i] * total;
        if (expected > 0.0) {
            double diff = counts[i] - expected;
            chi += diff * diff / expected;
        }
    }
    return chi;
}

/* ---- Auto analyze key ---- */

QString BeaufortCode::autoAnalyzeKey(const QString& ciphertext, int keyLength) const
{
    QString alpha = toAlphaUpper(ciphertext);
    QString key;

    for (int g = 0; g < keyLength; ++g) {
        QString group;
        for (int i = g; i < alpha.size(); i += keyLength)
            group.append(alpha[i]);

        // Try all 26 possible shifts, pick best by chi-squared
        double bestChi = 1e18;
        int bestShift = 0;
        int groupN = group.size();

        for (int shift = 0; shift < 26; ++shift) {
            QVector<int> counts(26, 0);
            for (QChar ch : group) {
                // Beaufort decrypt: P = (K - C) mod 26
                int c = ch.toLatin1() - 'A';
                int p = (shift - c + 26) % 26;
                counts[p]++;
            }
            double chi = chiSquared(counts, groupN);
            if (chi < bestChi) { bestChi = chi; bestShift = shift; }
        }
        key.append(QChar('A' + bestShift));
    }

    return key;
}

/* ---- Frequency analysis ---- */

QVector<QPair<QChar, double>> BeaufortCode::frequencyAnalysis(const QString& text) const
{
    QString alpha = toAlphaUpper(text);
    int n = alpha.size();
    int counts[26] = {};
    for (QChar ch : alpha) {
        int idx = ch.toLatin1() - 'A';
        if (idx >= 0 && idx < 26) counts[idx]++;
    }

    QVector<QPair<QChar, double>> freqs;
    for (int i = 0; i < 26; ++i)
        freqs.append(qMakePair(QChar('A' + i), n > 0 ? static_cast<double>(counts[i]) / n : 0.0));

    std::sort(freqs.begin(), freqs.end(),
              [](const QPair<QChar, double>& a, const QPair<QChar, double>& b) {
                  return a.second > b.second;
              });
    return freqs;
}

/* ---- Reset ---- */

void BeaufortCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
