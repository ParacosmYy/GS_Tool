/**
 * @file TapirCode4.cpp
 * @brief TapirCode4 实现
 *
 * 实现Tapir编码：扩展多表替换与关键字周期密钥调度。
 */

#include "utils/code239/TapirCode4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TapirCode4::TapirCode4(QObject *parent) : QObject(parent) {}
TapirCode4::~TapirCode4() = default;

/* ---- Configuration ---- */

void TapirCode4::setKeyword(const QString& keyword)
{
    m_keyword = keyword;
    buildKeySchedule();
}

/* ---- Build periodic key schedule ---- */

void TapirCode4::buildKeySchedule()
{
    m_keySchedule.clear();
    if (m_keyword.isEmpty()) return;

    // Derive key schedule: each keyword char maps to a shift value
    // Extended: use cumulative position-weighted hash for richer scheduling
    int running = 0;
    for (int i = 0; i < m_keyword.size(); ++i) {
        int c = m_keyword[i].unicode();
        running = (running * 31 + c) & 0xFFFF;
        m_keySchedule.append(((running % ALPHABET_SIZE) + ALPHABET_SIZE) % ALPHABET_SIZE);
    }

    // Extend schedule to at least 256 entries via keyword-periodic extension
    int kwLen = m_keySchedule.size();
    for (int i = kwLen; i < 256; ++i) {
        int val = (m_keySchedule[i % kwLen] + m_keySchedule[(i + 1) % kwLen]) % ALPHABET_SIZE;
        m_keySchedule.append(val);
    }
}

/* ---- Forward substitution ---- */

int TapirCode4::substitute(int charVal, int pos) const
{
    int key = m_keySchedule.isEmpty() ? 0 : m_keySchedule[pos % m_keySchedule.size()];
    // Polyalphabetic: shift + position-dependent rotation
    int rotated = (key * 7 + pos * 13) % ALPHABET_SIZE;
    return ((charVal + key + rotated) % ALPHABET_SIZE + ALPHABET_SIZE) % ALPHABET_SIZE;
}

/* ---- Inverse substitution ---- */

int TapirCode4::invSubstitute(int cipherVal, int pos) const
{
    int key = m_keySchedule.isEmpty() ? 0 : m_keySchedule[pos % m_keySchedule.size()];
    int rotated = (key * 7 + pos * 13) % ALPHABET_SIZE;
    return ((cipherVal - key - rotated) % ALPHABET_SIZE + ALPHABET_SIZE) % ALPHABET_SIZE;
}

/* ---- Encode string ---- */

QString TapirCode4::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keySchedule.isEmpty()) buildKeySchedule();

    QString result;
    result.reserve(plaintext.size());

    for (int i = 0; i < plaintext.size(); ++i) {
        int ch = plaintext[i].unicode();
        if (ch >= ASCII_OFFSET && ch < ASCII_OFFSET + ALPHABET_SIZE) {
            int val = ch - ASCII_OFFSET;
            int enc = substitute(val, i);
            result.append(QChar(enc + ASCII_OFFSET));
        } else {
            // Non-printable range: pass through with simple XOR
            int key = m_keySchedule.isEmpty() ? 0 : m_keySchedule[i % m_keySchedule.size()];
            result.append(QChar(ch ^ (key & 0xFF)));
        }
    }

    m_stats.numEncode++;
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodeCompleted(plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decode string ---- */

QString TapirCode4::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keySchedule.isEmpty()) buildKeySchedule();

    QString result;
    result.reserve(ciphertext.size());

    for (int i = 0; i < ciphertext.size(); ++i) {
        int ch = ciphertext[i].unicode();
        if (ch >= ASCII_OFFSET && ch < ASCII_OFFSET + ALPHABET_SIZE) {
            int val = ch - ASCII_OFFSET;
            int dec = invSubstitute(val, i);
            result.append(QChar(dec + ASCII_OFFSET));
        } else {
            int key = m_keySchedule.isEmpty() ? 0 : m_keySchedule[i % m_keySchedule.size()];
            result.append(QChar(ch ^ (key & 0xFF)));
        }
    }

    m_stats.numDecode++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decodeCompleted(ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Encode bytes ---- */

QByteArray TapirCode4::encodeBytes(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keySchedule.isEmpty()) buildKeySchedule();

    QByteArray result;
    result.resize(data.size());

    for (int i = 0; i < data.size(); ++i) {
        int key = m_keySchedule[i % m_keySchedule.size()];
        int posKey = (key * 7 + i * 13) & 0xFF;
        result[i] = static_cast<char>((static_cast<quint8>(data[i]) + key + posKey) & 0xFF);
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Decode bytes ---- */

QByteArray TapirCode4::decodeBytes(const QByteArray& encoded)
{
    QElapsedTimer timer;
    timer.start();

    if (m_keySchedule.isEmpty()) buildKeySchedule();

    QByteArray result;
    result.resize(encoded.size());

    for (int i = 0; i < encoded.size(); ++i) {
        int key = m_keySchedule[i % m_keySchedule.size()];
        int posKey = (key * 7 + i * 13) & 0xFF;
        result[i] = static_cast<char>((static_cast<quint8>(encoded[i]) - key - posKey) & 0xFF);
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Reset ---- */

void TapirCode4::resetStatistics()
{
    m_keyword.clear(); m_keySchedule.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
