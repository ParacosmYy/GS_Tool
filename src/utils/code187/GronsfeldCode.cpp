/**
 * @file GronsfeldCode.cpp
 * @brief GronsfeldCode 实现
 *
 * 实现Gronsfeld密码：数字密钥加密解密、Kasiski检验、叠合攻击。
 */

#include "utils/code187/GronsfeldCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- English letter frequencies (A-Z) ---- */

const QVector<double> GronsfeldCode::s_englishFreq = {
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074
};

/* ---- Construction / Destruction ---- */

GronsfeldCode::GronsfeldCode(QObject *parent) : QObject(parent) {}
GronsfeldCode::~GronsfeldCode() = default;

/* ---- Configuration ---- */

void GronsfeldCode::setKey(const QVector<int>& key)
{
    m_key.clear();
    for (int k : key) m_key.append(qBound(0, k, 9));
}

/* ---- Normalize text to uppercase letters ---- */

QString GronsfeldCode::normalize(const QString& text) const
{
    QString result;
    for (QChar c : text) {
        if (c.isLetter())
            result.append(c.toUpper());
    }
    return result;
}

/* ---- Shift a single character ---- */

QChar GronsfeldCode::shiftChar(QChar c, int shift, bool encrypt) const
{
    if (!c.isLetter()) return c;
    int base = c.toUpper().toLatin1() - 'A';
    if (encrypt)
        base = (base + shift) % 26;
    else
        base = (base - shift + 26) % 26;
    return QChar('A' + base);
}

/* ---- Encrypt ---- */

QString GronsfeldCode::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString norm = normalize(plaintext);
    if (m_key.isEmpty()) return norm;

    QString result;
    int keyLen = m_key.size();
    for (int i = 0; i < norm.size(); ++i)
        result.append(shiftChar(norm[i], m_key[i % keyLen], true));

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString GronsfeldCode::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString norm = normalize(ciphertext);
    if (m_key.isEmpty()) return norm;

    QString result;
    int keyLen = m_key.size();
    for (int i = 0; i < norm.size(); ++i)
        result.append(shiftChar(norm[i], m_key[i % keyLen], false));

    emit operationCompleted("decrypt", timer.elapsed());
    return result;
}

/* ---- Chi-squared test against English ---- */

double GronsfeldCode::chiSquared(const QVector<int>& counts) const
{
    int total = 0;
    for (int c : counts) total += c;
    if (total == 0) return 1e30;

    double chi = 0.0;
    for (int i = 0; i < 26; ++i) {
        double expected = s_englishFreq[i] * total;
        if (expected > 0)
            chi += (counts[i] - expected) * (counts[i] - expected) / expected;
    }
    return chi;
}

/* ---- Kasiski examination ---- */

QVector<QPair<int, int>> GronsfeldCode::kasiskiExamination(
    const QString& ciphertext) const
{
    QString text = normalize(ciphertext);
    int n = text.size();

    // Find repeated trigrams and their spacings
    QMap<QString, QVector<int>> trigramPositions;
    for (int i = 0; i <= n - 3; ++i)
        trigramPositions[text.mid(i, 3)].append(i);

    // Count GCD of spacings
    QMap<int, int> factorCounts;
    for (auto it = trigramPositions.begin(); it != trigramPositions.end(); ++it) {
        auto& positions = it.value();
        if (positions.size() < 2) continue;
        for (int i = 0; i < positions.size() - 1; ++i) {
            int spacing = positions[i + 1] - positions[i];
            for (int f = 2; f <= spacing; ++f) {
                if (spacing % f == 0) factorCounts[f]++;
            }
        }
    }

    // Sort by frequency
    QVector<QPair<int, int>> results;
    for (auto it = factorCounts.begin(); it != factorCounts.end(); ++it)
        results.append({it.key(), it.value()});

    std::sort(results.begin(), results.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.second > b.second;
              });

    if (results.size() > 20) results.resize(20);
    return results;
}

/* ---- Superimposition attack ---- */

QVector<int> GronsfeldCode::superimpositionAttack(
    const QString& ciphertext, int keyLen) const
{
    QString text = normalize(ciphertext);
    QVector<int> recoveredKey(keyLen, 0);

    for (int pos = 0; pos < keyLen; ++pos) {
        QVector<int> counts(26, 0);
        for (int i = pos; i < text.size(); i += keyLen)
            counts[text[i].toLatin1() - 'A']++;

        // Try all 10 possible shifts (0-9)
        double bestChi = 1e30;
        int bestShift = 0;
        for (int shift = 0; shift <= 9; ++shift) {
            QVector<int> shiftedCounts(26, 0);
            for (int c = 0; c < 26; ++c)
                shiftedCounts[(c - shift + 26) % 26] = counts[c];
            double chi = chiSquared(shiftedCounts);
            if (chi < bestChi) { bestChi = chi; bestShift = shift; }
        }
        recoveredKey[pos] = bestShift;
    }

    m_stats.keyLength = keyLen;
    return recoveredKey;
}

/* ---- Frequency analysis ---- */

QVector<QVector<double>> GronsfeldCode::frequencyAnalysis(
    const QString& text, int keyLen) const
{
    QString norm = normalize(text);
    QVector<QVector<double>> freqs(keyLen, QVector<double>(26, 0.0));

    for (int pos = 0; pos < keyLen; ++pos) {
        int count = 0;
        for (int i = pos; i < norm.size(); i += keyLen) {
            freqs[pos][norm[i].toLatin1() - 'A'] += 1.0;
            count++;
        }
        if (count > 0) {
            for (int c = 0; c < 26; ++c)
                freqs[pos][c] /= count;
        }
    }
    return freqs;
}

/* ---- Index of coincidence ---- */

double GronsfeldCode::indexOfCoincidence(const QString& text) const
{
    QString norm = normalize(text);
    int n = norm.size();
    if (n < 2) return 0.0;

    QVector<int> counts(26, 0);
    for (QChar c : norm)
        counts[c.toLatin1() - 'A']++;

    double ic = 0.0;
    for (int i = 0; i < 26; ++i)
        ic += counts[i] * (counts[i] - 1);

    return ic / (n * (n - 1.0));
}

/* ---- Reset ---- */

void GronsfeldCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
