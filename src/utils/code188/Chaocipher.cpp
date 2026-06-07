/**
 * @file Chaocipher.cpp
 * @brief Chaocipher 实现
 *
 * 实现Chaocipher双旋转字母表密码：位置驱动置换、中间字母切分、加密解密。
 */

#include "utils/code188/Chaocipher.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chaocipher::Chaocipher(QObject *parent) : QObject(parent)
{
    // Default left and right alphabets (can be overridden)
    m_left = QStringLiteral("HXUCZVAMDSLKPEFJRIGTWOBNYQ");
    m_right = QStringLiteral("PTLNBQDEOYSFAVZKGJRIHWXUMC");
    m_initialLeft = m_left;
    m_initialRight = m_right;
}

Chaocipher::~Chaocipher() = default;

/* ---- Configuration ---- */

void Chaocipher::setLeftAlphabet(const QString& alpha)
{
    if (validateAlphabet(alpha)) {
        m_left = alpha.toUpper();
        m_initialLeft = m_left;
    }
}

void Chaocipher::setRightAlphabet(const QString& alpha)
{
    if (validateAlphabet(alpha)) {
        m_right = alpha.toUpper();
        m_initialRight = m_right;
    }
}

/* ---- Validate alphabet ---- */

bool Chaocipher::validateAlphabet(const QString& alpha) const
{
    if (alpha.length() != ALPHA_SIZE) return false;
    QVector<bool> seen(ALPHA_SIZE, false);
    for (int i = 0; i < ALPHA_SIZE; ++i) {
        QChar c = alpha[i].toUpper();
        if (c < 'A' || c > 'Z') return false;
        int idx = c.unicode() - 'A';
        if (seen[idx]) return false; // Duplicate
        seen[idx] = true;
    }
    return true;
}

/* ---- Find character position ---- */

int Chaocipher::findPos(const QString& alpha, QChar ch) const
{
    for (int i = 0; i < alpha.size(); ++i)
        if (alpha[i] == ch) return i;
    return -1;
}

/* ---- Permute left alphabet ---- */

void Chaocipher::permuteLeft(int pos)
{
    // Shift left from pos+1 to front, keeping first pos chars at end
    if (pos + 1 < ALPHA_SIZE) {
        QString part = m_left.mid(pos + 1);
        QString front = m_left.left(pos + 1);
        m_left = part + front;
    }
    // Then take element at position PVT_POS and move to end, shift rest left
    if (PVT_POS < ALPHA_SIZE) {
        QChar pvt = m_left[PVT_POS];
        m_left = m_left.mid(0, PVT_POS) + m_left.mid(PVT_POS + 1) + pvt;
    }
}

/* ---- Permute right alphabet ---- */

void Chaocipher::permuteRight(int pos)
{
    // Shift right from pos+1 to front
    if (pos + 1 < ALPHA_SIZE) {
        QString part = m_right.mid(pos + 1);
        QString front = m_right.left(pos + 1);
        m_right = part + front;
    }
    // Take element at position PVT_POS and move to end
    if (PVT_POS < ALPHA_SIZE) {
        QChar pvt = m_right[PVT_POS];
        m_right = m_right.mid(0, PVT_POS) + m_right.mid(PVT_POS + 1) + pvt;
    }
}

/* ---- Encrypt ---- */

QString Chaocipher::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (int i = 0; i < plaintext.size(); ++i) {
        QChar ch = plaintext[i].toUpper();
        if (ch < 'A' || ch > 'Z') continue; // Skip non-alpha

        // Find plaintext char in right alphabet
        int pos = findPos(m_right, ch);
        if (pos < 0) continue;

        // Ciphertext char is at same position in left alphabet
        QChar cipher = m_left[pos];
        result.append(cipher);

        // Permute both alphabets
        permuteLeft(pos);
        permuteRight(pos);
    }

    m_stats.totalOps++;
    m_stats.charsEncrypted += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("encrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString Chaocipher::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (int i = 0; i < ciphertext.size(); ++i) {
        QChar ch = ciphertext[i].toUpper();
        if (ch < 'A' || ch > 'Z') continue;

        // Find ciphertext char in left alphabet
        int pos = findPos(m_left, ch);
        if (pos < 0) continue;

        // Plaintext char is at same position in right alphabet
        QChar plain = m_right[pos];
        result.append(plain);

        // Permute both alphabets
        permuteLeft(pos);
        permuteRight(pos);
    }

    m_stats.totalOps++;
    m_stats.charsDecrypted += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Reset alphabets to initial state ---- */

void Chaocipher::resetAlphabets()
{
    m_left = m_initialLeft;
    m_right = m_initialRight;
}

/* ---- Reset statistics ---- */

void Chaocipher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    resetAlphabets();
}
