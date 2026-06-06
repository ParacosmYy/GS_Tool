/**
 * @file CaesarCode.cpp
 * @brief CaesarCode 实现
 *
 * 实现Caesar/ROT密码编解码、频率分析暴力破解。
 */

#include "utils/code173/CaesarCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* Standard English letter frequencies (A-Z) */
const double CaesarCode::s_englishFreq[26] = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
};

/* ---- Construction / Destruction ---- */

CaesarCode::CaesarCode(QObject *parent)
    : QObject(parent)
{
}

CaesarCode::~CaesarCode() = default;

/* ---- Configuration ---- */

void CaesarCode::setShift(int shift) { m_shift = ((shift % 26) + 26) % 26; }
void CaesarCode::setMode(PresetMode mode) { m_mode = mode; }

/* ---- Single character shift ---- */

QChar CaesarCode::shiftChar(QChar c, int offset) const
{
    if (c >= 'A' && c <= 'Z')
        return static_cast<QChar>('A' + (c.toLatin1() - 'A' + offset + 26) % 26);
    if (c >= 'a' && c <= 'z')
        return static_cast<QChar>('a' + (c.toLatin1() - 'a' + offset + 26) % 26);
    return c;
}

/* ---- ROT47 shift ---- */

QChar CaesarCode::shiftRot47(QChar c, int offset) const
{
    int code = c.toLatin1();
    if (code >= 33 && code <= 126) {
        int shifted = code + offset;
        /* Wrap within printable ASCII range [33,126] */
        int range = 126 - 33 + 1; /* 94 */
        shifted = 33 + ((shifted - 33) % range + range) % range;
        return static_cast<QChar>(shifted);
    }
    return c;
}

/* ---- Encode ---- */

QString CaesarCode::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    result.reserve(plaintext.size());

    int shift = m_shift;
    if (m_mode == ROT13) shift = 13;
    else if (m_mode == ROT47) shift = 47;

    for (const QChar& c : plaintext) {
        if (m_mode == ROT47)
            result.append(shiftRot47(c, shift));
        else
            result.append(shiftChar(c, shift));
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes + m_stats.totalCracks);

    emit encodeCompleted(result.size());
    return result;
}

/* ---- Decode ---- */

QString CaesarCode::decode(const QString& ciphertext) const
{
    int shift = m_shift;
    if (m_mode == ROT13) shift = 13;
    else if (m_mode == ROT47) shift = 47;

    /* Decoding is encoding with negative shift */
    QString result;
    result.reserve(ciphertext.size());
    for (const QChar& c : ciphertext) {
        if (m_mode == ROT47)
            result.append(shiftRot47(c, -shift));
        else
            result.append(shiftChar(c, -shift));
    }

    m_stats.totalDecodes++;
    emit decodeCompleted(result.size());
    return result;
}

/* ---- Decrypt with known shift ---- */

QString CaesarCode::decryptWithShift(const QString& ciphertext, int shift) const
{
    QString result;
    result.reserve(ciphertext.size());
    for (const QChar& c : ciphertext)
        result.append(shiftChar(c, -shift));
    return result;
}

/* ---- Frequency distribution ---- */

QMap<QChar, double> CaesarCode::frequencyDistribution(const QString& text) const
{
    QMap<QChar, double> freq;
    int totalLetters = 0;

    for (const QChar& c : text) {
        if (c >= 'A' && c <= 'Z') {
            freq[c.toUpper()] += 1.0;
            totalLetters++;
        } else if (c >= 'a' && c <= 'z') {
            freq[c.toUpper()] += 1.0;
            totalLetters++;
        }
    }

    if (totalLetters > 0) {
        for (auto it = freq.begin(); it != freq.end(); ++it)
            it.value() /= totalLetters;
    }
    return freq;
}

/* ---- Chi-squared score ---- */

double CaesarCode::chiSquaredScore(const QMap<QChar, double>& observed) const
{
    double score = 0.0;
    for (int i = 0; i < 26; ++i) {
        QChar c = QChar('A' + i);
        double obs = observed.value(c, 0.0);
        double exp = s_englishFreq[i];
        if (exp > 0.0)
            score += (obs - exp) * (obs - exp) / exp;
    }
    return score;
}

/* ---- Brute force all 26 shifts ---- */

QVector<CaesarCode::CrackResult> CaesarCode::bruteForce(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<CrackResult> results;
    results.reserve(26);

    for (int shift = 0; shift < 26; ++shift) {
        CrackResult r;
        r.shift = shift;
        r.plaintext = decryptWithShift(ciphertext, shift);

        /* Score using frequency analysis */
        QMap<QChar, double> freq = frequencyDistribution(r.plaintext);
        r.score = chiSquaredScore(freq); /* Lower = better match */

        results.append(r);
    }

    /* Sort by score (ascending = best match first) */
    std::sort(results.begin(), results.end(),
              [](const CrackResult& a, const CrackResult& b) {
                  return a.score < b.score;
              });

    m_stats.totalCracks++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes + m_stats.totalCracks);

    emit crackCompleted(
        results.isEmpty() ? 0 : results[0].shift,
        results.isEmpty() ? 0.0 : results[0].score);
    return results;
}

/* ---- Frequency analysis crack ---- */

CaesarCode::CrackResult CaesarCode::crackFrequency(const QString& ciphertext) const
{
    QVector<CrackResult> all = bruteForce(ciphertext);
    if (all.isEmpty()) return CrackResult{};
    return all[0]; /* Already sorted by best score */
}

/* ---- Statistics ---- */

void CaesarCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
