/**
 * @file AffineCode.cpp
 * @brief AffineCode 实现
 *
 * 实现仿射密码：ax+b mod 26加密/解密、模逆验证、穷举密钥分析。
 */

#include "utils/code180/AffineCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Static data ---- */

constexpr double AffineCode::s_enFreq[26];
constexpr int AffineCode::s_validA[];

/* ---- Construction / Destruction ---- */

AffineCode::AffineCode(QObject *parent) : QObject(parent) {}
AffineCode::~AffineCode() = default;

/* ---- Utility ---- */

int AffineCode::charToIdx(QChar c) const
{
    if (c >= 'A' && c <= 'Z') return c.unicode() - 'A';
    if (c >= 'a' && c <= 'z') return c.unicode() - 'a';
    return -1; // non-alpha
}

QChar AffineCode::idxToChar(int idx, bool upper) const
{
    if (idx < 0 || idx >= 26) return QChar();
    return upper ? QChar('A' + idx) : QChar('a' + idx);
}

/* ---- Number theory ---- */

int AffineCode::gcd(int a, int b) const
{
    a = qAbs(a); b = qAbs(b);
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}

bool AffineCode::validateKey(int a) const
{
    return gcd(a, 26) == 1;
}

int AffineCode::modInverse(int a) const
{
    // Extended Euclidean algorithm to find x: a*x ≡ 1 (mod 26)
    a = ((a % 26) + 26) % 26;
    if (a == 0) return -1;

    int t = 0, newT = 1;
    int r = 26, newR = a;

    while (newR != 0) {
        int q = r / newR;
        int tmp = t - q * newT; t = newT; newT = tmp;
        tmp = r - q * newR; r = newR; newR = tmp;
    }

    if (r > 1) return -1; // No inverse exists
    if (t < 0) t += 26;
    return t;
}

/* ---- Encrypt ---- */

QString AffineCode::encrypt(const QString& plaintext, const Key& key) const
{
    QElapsedTimer timer;
    timer.start();

    if (!validateKey(key.a)) return {};

    QString result;
    result.reserve(plaintext.size());

    for (const QChar& c : plaintext) {
        int idx = charToIdx(c);
        if (idx >= 0) {
            int enc = (key.a * idx + key.b) % 26;
            result.append(idxToChar(enc, c.isUpper()));
        } else {
            result.append(c); // Preserve non-alpha characters
        }
    }

    return result;
}

/* ---- Decrypt ---- */

QString AffineCode::decrypt(const QString& ciphertext, const Key& key) const
{
    QElapsedTimer timer;
    timer.start();

    if (!validateKey(key.a)) return {};

    int aInv = modInverse(key.a);
    if (aInv < 0) return {};

    QString result;
    result.reserve(ciphertext.size());

    for (const QChar& c : ciphertext) {
        int idx = charToIdx(c);
        if (idx >= 0) {
            int dec = (aInv * (idx - key.b + 26)) % 26;
            result.append(idxToChar(dec, c.isUpper()));
        } else {
            result.append(c);
        }
    }

    return result;
}

/* ---- Frequency scoring ---- */

double AffineCode::frequencyScore(const QString& text) const
{
    int counts[26] = {};
    int totalLetters = 0;

    for (const QChar& c : text) {
        int idx = charToIdx(c);
        if (idx >= 0) { counts[idx]++; totalLetters++; }
    }

    if (totalLetters == 0) return 0.0;

    // Chi-squared statistic (lower = better match to English)
    double chi2 = 0.0;
    for (int i = 0; i < 26; ++i) {
        double expected = s_enFreq[i] * totalLetters;
        if (expected > 0.0) {
            double diff = counts[i] - expected;
            chi2 += (diff * diff) / expected;
        }
    }

    // Convert to fitness score (inverse of chi-squared)
    return 1.0 / (1.0 + chi2);
}

/* ---- Exhaustive cryptanalysis ---- */

QVector<AffineCode::AnalysisResult> AffineCode::exhaustiveCryptanalysis(
    const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<AnalysisResult> results;

    // Try all valid a values (12 values coprime to 26)
    for (int a : s_validA) {
        for (int b = 0; b < 26; ++b) {
            Key key{a, b};
            QString plain = decrypt(ciphertext, key);
            double score = frequencyScore(plain);

            results.append({key, plain, score});
        }
    }

    // Sort by score descending (best match first)
    std::sort(results.begin(), results.end(),
              [](const AnalysisResult& a, const AnalysisResult& b) {
                  return a.score > b.score;
              });

    return results;
}

/* ---- Reset ---- */

void AffineCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
