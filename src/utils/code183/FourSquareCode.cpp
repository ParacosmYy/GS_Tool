/**
 * @file FourSquareCode.cpp
 * @brief FourSquareCode 实现
 *
 * 实现四方密码：双关键字配对Polybius方阵、2×2双字母编解码。
 */

#include "utils/code183/FourSquareCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FourSquareCode::FourSquareCode(QObject *parent) : QObject(parent)
    , m_alphabet(defaultAlphabet()) {}
FourSquareCode::~FourSquareCode() = default;

/* ---- Default alphabet ---- */

QString FourSquareCode::defaultAlphabet()
{
    return QStringLiteral("ABCDEFGHIKLMNOPQRSTUVWXYZ");
}

/* ---- Configuration ---- */

void FourSquareCode::setKey1(const QString& key) { m_key1 = key.toUpper(); }
void FourSquareCode::setKey2(const QString& key) { m_key2 = key.toUpper(); }
void FourSquareCode::setAlphabet(const QString& alpha) { m_alphabet = alpha.toUpper(); }

/* ---- Key processing ---- */

QString FourSquareCode::processKey(const QString& key) const
{
    QString processed;
    QSet<QChar> seen;
    // Add key characters first
    for (QChar c : key) {
        QChar ch = c.toUpper();
        if (ch == 'J') ch = 'I';
        if (!seen.contains(ch) && m_alphabet.contains(ch)) {
            processed.append(ch);
            seen.insert(ch);
        }
    }
    // Fill remaining alphabet
    for (QChar c : m_alphabet) {
        if (!seen.contains(c)) {
            processed.append(c);
            seen.insert(c);
        }
    }
    return processed;
}

/* ---- Build Polybius square ---- */

QVector<QVector<QChar>> FourSquareCode::buildSquare(const QString& key) const
{
    QString filled = processKey(key);
    QVector<QVector<QChar>> square(5, QVector<QChar>(5));
    int idx = 0;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            square[r][c] = filled[idx++];
    return square;
}

/* ---- Find character in square ---- */

QPair<int, int> FourSquareCode::findInSquare(
    const QVector<QVector<QChar>>& square, QChar ch) const
{
    if (ch == 'J') ch = 'I';
    for (int r = 0; r < square.size(); ++r)
        for (int c = 0; c < square[r].size(); ++c)
            if (square[r][c] == ch) return {r, c};
    return {0, 0};
}

/* ---- Prepare text ---- */

QString FourSquareCode::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c.isLetter()) {
            if (c == 'J') result.append('I');
            else result.append(c);
        }
    }
    // Pad with 'X' if odd length
    if (result.size() % 2 != 0) result.append('X');
    return result;
}

/* ---- Encrypt ---- */

QString FourSquareCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Build four squares: TL=plain, TR=key1, BL=key2, BR=plain
    auto tlSquare = buildSquare(QString());      // Plain alphabet
    auto trSquare = buildSquare(m_key1);          // Key1 square
    auto blSquare = buildSquare(m_key2);          // Key2 square
    auto brSquare = buildSquare(QString());       // Plain alphabet

    QString prepared = prepareText(plaintext);
    QString result;

    for (int i = 0; i < prepared.size() - 1; i += 2) {
        QChar a = prepared[i];
        QChar b = prepared[i + 1];

        // Find positions in plain squares
        auto posA = findInSquare(tlSquare, a);  // Row, Col in top-left
        auto posB = findInSquare(brSquare, b);  // Row, Col in bottom-right

        // Encrypted pair: read from TR (row of a, col of b) and BL (row of b, col of a)
        QChar enc1 = trSquare[posA.first][posB.second];
        QChar enc2 = blSquare[posB.first][posA.second];

        result.append(enc1);
        result.append(enc2);
    }

    m_stats.totalOperations++;
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", result.size());
    return result;
}

/* ---- Decrypt ---- */

QString FourSquareCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    auto tlSquare = buildSquare(QString());
    auto trSquare = buildSquare(m_key1);
    auto blSquare = buildSquare(m_key2);
    auto brSquare = buildSquare(QString());

    QString prepared = prepareText(ciphertext);
    QString result;

    for (int i = 0; i < prepared.size() - 1; i += 2) {
        QChar a = prepared[i];
        QChar b = prepared[i + 1];

        // Find positions in cipher squares
        auto posA = findInSquare(trSquare, a);   // Row, Col in TR
        auto posB = findInSquare(blSquare, b);   // Row, Col in BL

        // Decrypted pair: cross-read from plain squares
        QChar dec1 = tlSquare[posA.first][posB.second];
        QChar dec2 = brSquare[posB.first][posA.second];

        result.append(dec1);
        result.append(dec2);
    }

    m_stats.totalOperations++;
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decrypt", result.size());
    return result;
}

/* ---- Reset ---- */

void FourSquareCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
