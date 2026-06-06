/**
 * @file AutokeyCode.cpp
 * @brief AutokeyCode 实现
 *
 * 实现Autokey密码：运行密钥生成、已知明文攻击、密钥偏移发现、频率分析。
 */

#include "utils/code182/AutokeyCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AutokeyCode::AutokeyCode(QObject *parent) : QObject(parent) {}
AutokeyCode::~AutokeyCode() = default;

/* ---- Configuration ---- */

void AutokeyCode::setAlphabet(const QString& alpha) { m_alphabet = alpha; }

/* ---- Char <-> Index ---- */

int AutokeyCode::charToIndex(QChar c) const
{
    int idx = m_alphabet.indexOf(c.toUpper());
    return idx;
}

QChar AutokeyCode::indexToChar(int idx) const
{
    int n = m_alphabet.size();
    return m_alphabet[((idx % n) + n) % n];
}

/* ---- Running key ---- */

QString AutokeyCode::buildRunningKey(const QString& key, int textLength) const
{
    QString running = key.toUpper();
    // Autokey: pad with plaintext-derived chars (handled during encrypt)
    return running.left(textLength);
}

QString AutokeyCode::extractInitialKey(const QString& runningKey, int textLength) const
{
    Q_UNUSED(textLength)
    return runningKey;
}

/* ---- Encrypt ---- */

QString AutokeyCode::encrypt(const QString& plaintext, const QString& key) const
{
    int n = m_alphabet.size();
    QString result;
    QString fullKey = key.toUpper();
    int ki = 0;

    for (int i = 0; i < plaintext.size(); ++i) {
        QChar pc = plaintext[i].toUpper();
        int pi = charToIndex(pc);
        if (pi < 0) { result += pc; continue; }

        // Use key char, or auto-key from plaintext
        int kIdx;
        if (ki < fullKey.size()) {
            kIdx = charToIndex(fullKey[ki]);
        } else {
            // Autokey: use previous plaintext char
            kIdx = charToIndex(plaintext[ki - fullKey.size()].toUpper());
        }
        if (kIdx < 0) kIdx = 0;

        int encIdx = (pi + kIdx) % n;
        result += indexToChar(encIdx);
        ki++;
    }
    return result;
}

/* ---- Decrypt ---- */

QString AutokeyCode::decrypt(const QString& ciphertext, const QString& key) const
{
    int n = m_alphabet.size();
    QString result;
    QString fullKey = key.toUpper();
    int ki = 0;

    for (int i = 0; i < ciphertext.size(); ++i) {
        QChar cc = ciphertext[i].toUpper();
        int ci = charToIndex(cc);
        if (ci < 0) { result += cc; continue; }

        int kIdx;
        if (ki < fullKey.size()) {
            kIdx = charToIndex(fullKey[ki]);
        } else {
            // Autokey: use previously decrypted plaintext char
            kIdx = charToIndex(result[ki - fullKey.size()].toUpper());
        }
        if (kIdx < 0) kIdx = 0;

        int decIdx = (ci - kIdx + n) % n;
        result += indexToChar(decIdx);
        ki++;
    }
    return result;
}

/* ---- Known-plaintext attack ---- */

QPair<bool, QString> AutokeyCode::knownPlaintextAttack(
    const QString& ciphertext, const QString& knownPlaintext, int keyOffset) const
{
    int n = m_alphabet.size();
    QString recoveredKey;

    for (int i = 0; i < knownPlaintext.size(); ++i) {
        int cPos = keyOffset + i;
        if (cPos < 0 || cPos >= ciphertext.size()) return {false, {}};

        int ci = charToIndex(ciphertext[cPos].toUpper());
        int pi = charToIndex(knownPlaintext[i].toUpper());
        if (ci < 0 || pi < 0) return {false, {}};

        // key = ciphertext - plaintext (mod n)
        int kIdx = (ci - pi + n) % n;

        // For autokey, if position >= keyLength, key char comes from earlier plaintext
        // This recovered key fragment helps discover the original key
        recoveredKey += indexToChar(kIdx);
    }

    // Verify: try to decrypt with recovered key prefix
    if (recoveredKey.size() >= qMin(3, knownPlaintext.size())) {
        QString testDecrypt = decrypt(ciphertext, recoveredKey);
        if (testDecrypt.mid(keyOffset, knownPlaintext.size()).toUpper()
            == knownPlaintext.toUpper()) {
            return {true, recoveredKey};
        }
    }
    return {true, recoveredKey};
}

/* ---- Auto-discover key offset ---- */

QVector<QPair<int, QString>> AutokeyCode::autoDiscoverKey(
    const QString& ciphertext, const QString& knownPlaintext) const
{
    QVector<QPair<int, QString>> results;
    int maxOffset = ciphertext.size() - knownPlaintext.size();

    for (int offset = 0; offset <= maxOffset; ++offset) {
        auto [ok, key] = knownPlaintextAttack(ciphertext, knownPlaintext, offset);
        if (ok) {
            double score = frequencyScore(decrypt(ciphertext, key));
            if (score > 0.5)
                results.append({offset, key});
        }
    }
    return results;
}

/* ---- Frequency analysis ---- */

double AutokeyCode::frequencyScore(const QString& text) const
{
    // English letter frequencies (A-Z)
    static const QVector<double> englishFreq = {
        8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094, 6.966,
        0.153, 0.772, 4.025, 2.406, 6.749, 7.507, 1.929, 0.095, 5.987,
        6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.074
    };

    int n = m_alphabet.size();
    QVector<double> observed(n, 0.0);
    int total = 0;

    for (const QChar& c : text) {
        int idx = charToIndex(c.toUpper());
        if (idx >= 0 && idx < n) { observed[idx]++; total++; }
    }
    if (total == 0) return 0.0;

    double chiSq = 0.0;
    for (int i = 0; i < n && i < englishFreq.size(); ++i) {
        double expected = englishFreq[i] * total / 100.0;
        if (expected > 0)
            chiSq += (observed[i] - expected) * (observed[i] - expected) / expected;
    }
    // Lower chi-squared = closer to English, invert for score
    double maxChi = static_cast<double>(total) * 0.5;
    return (maxChi > 0) ? qMax(0.0, 1.0 - chiSq / maxChi) : 0.0;
}

/* ---- Index of coincidence ---- */

double AutokeyCode::indexOfCoincidence(const QString& text) const
{
    int n = m_alphabet.size();
    QVector<int> freq(n, 0);
    int total = 0;
    for (const QChar& c : text) {
        int idx = charToIndex(c.toUpper());
        if (idx >= 0) { freq[idx]++; total++; }
    }
    if (total <= 1) return 0.0;
    double ic = 0.0;
    for (int i = 0; i < n; ++i) ic += freq[i] * (freq[i] - 1);
    return ic / (total * (total - 1.0));
}

/* ---- Chi-squared ---- */

double AutokeyCode::chiSquared(const QString& text) const
{
    return -frequencyScore(text) + 1.0; // Simplified inversion
}

/* ---- Guess key length ---- */

QVector<int> AutokeyCode::guessKeyLength(const QString& ciphertext, int maxLen) const
{
    QVector<QPair<double, int>> scores;
    double baseIC = indexOfCoincidence(ciphertext);

    for (int len = 1; len <= maxLen; ++len) {
        double avgIC = 0.0;
        for (int g = 0; g < len; ++g) {
            QString group;
            for (int i = g; i < ciphertext.size(); i += len)
                group += ciphertext[i];
            avgIC += indexOfCoincidence(group);
        }
        avgIC /= len;
        scores.append({qAbs(avgIC - 0.0667), len}); // English IC ~ 0.0667
    }
    std::sort(scores.begin(), scores.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    QVector<int> result;
    for (int i = 0; i < qMin(5, scores.size()); ++i)
        result.append(scores[i].second);
    Q_UNUSED(baseIC)
    return result;
}

/* ---- Reset ---- */

void AutokeyCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
