/**
 * @file TapirCode5.cpp
 * @brief TapirCode5 实现
 *
 * 实现Tapir密码：多字母周期密钥与扩展数字标点符号集加密解密。
 */

#include "utils/code253/TapirCode5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TapirCode5::TapirCode5(QObject *parent)
    : QObject(parent) { buildAlphabet(); }
TapirCode5::~TapirCode5() = default;

/* ---- Build extended alphabet ---- */

void TapirCode5::buildAlphabet()
{
    m_alphabet.clear();
    // Uppercase A-Z
    for (ushort c = 'A'; c <= 'Z'; ++c)
        m_alphabet.append(QChar(c));
    // Lowercase a-z
    for (ushort c = 'a'; c <= 'z'; ++c)
        m_alphabet.append(QChar(c));
    // Digits 0-9
    for (ushort c = '0'; c <= '9'; ++c)
        m_alphabet.append(QChar(c));
    // Punctuation: common ASCII punctuation
    const QString punct = QStringLiteral(".,!?;:'\"()-[]{}<>@#$%^&*+_=/\\|~` ");
    for (const QChar& ch : punct)
        m_alphabet.append(ch);

    m_stats.alphabetSize = m_alphabet.size();
}

/* ---- Compute key offsets ---- */

void TapirCode5::computeKeyOffsets()
{
    m_keyOffsets.clear();
    if (m_key.isEmpty()) return;
    for (const QChar& ch : m_key) {
        int idx = charIndex(ch);
        // If char not in alphabet, use its code modulo alphabet size
        m_keyOffsets.append(idx >= 0 ? idx
                           : (ch.unicode() % m_alphabet.size()));
    }
}

/* ---- Find character index in alphabet ---- */

int TapirCode5::charIndex(QChar c) const
{
    for (int i = 0; i < m_alphabet.size(); ++i) {
        if (m_alphabet[i] == c) return i;
    }
    return -1;
}

/* ---- Shift character by offset ---- */

QChar TapirCode5::shiftChar(QChar c, int offset) const
{
    int idx = charIndex(c);
    if (idx < 0) return c;  // Leave unmapped chars unchanged
    int size = m_alphabet.size();
    int shifted = ((idx + offset) % size + size) % size;
    return m_alphabet[shifted];
}

/* ---- Set key ---- */

void TapirCode5::setKey(const QString& key)
{
    m_key = key;
    m_stats.keyLength = key.size();
    computeKeyOffsets();
}

/* ---- Encrypt ---- */

QString TapirCode5::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keyOffsets.isEmpty()) return plaintext;

    QString result;
    result.reserve(plaintext.size());
    int keyLen = m_keyOffsets.size();

    for (int i = 0; i < plaintext.size(); ++i) {
        int offset = m_keyOffsets[i % keyLen];
        result.append(shiftChar(plaintext[i], offset));
    }

    m_stats.inputLength = plaintext.size();
    m_stats.numEncryptions++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptionCompleted(plaintext.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString TapirCode5::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keyOffsets.isEmpty()) return ciphertext;

    QString result;
    result.reserve(ciphertext.size());
    int keyLen = m_keyOffsets.size();

    for (int i = 0; i < ciphertext.size(); ++i) {
        int offset = m_keyOffsets[i % keyLen];
        result.append(shiftChar(ciphertext[i], -offset));
    }

    m_stats.inputLength = ciphertext.size();
    m_stats.numDecryptions++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(ciphertext.size(), elapsed);
    return result;
}

/* ---- Get alphabet ---- */

QString TapirCode5::alphabet() const
{
    return m_alphabet;
}

/* ---- Reset ---- */

void TapirCode5::resetStatistics()
{
    m_key.clear();
    m_keyOffsets.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
    buildAlphabet();
}
