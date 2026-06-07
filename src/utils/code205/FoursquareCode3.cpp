/**
 * @file FoursquareCode3.cpp
 * @brief FoursquareCode3 实现
 *
 * 实现四方密码：扩展Polybius方阵构建、关键字坐标映射、编解码。
 */

#include "utils/code205/FoursquareCode3.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode3::FoursquareCode3(QObject *parent) : QObject(parent) {}
FoursquareCode3::~FoursquareCode3() = default;

/* ---- Configuration ---- */

void FoursquareCode3::setKeyword1(const QString& kw) { m_keyword1 = kw.toUpper(); }
void FoursquareCode3::setKeyword2(const QString& kw) { m_keyword2 = kw.toUpper(); }

/* ---- Default alphabet (6x6: A-Z minus Q + 0-9) ---- */

QString FoursquareCode3::defaultAlphabet()
{
    return "ABCDEFGHIJKLMNOPRSTUVWXYZ0123456789";
}

/* ---- Build a key square from keyword ---- */

QVector<QVector<QChar>> FoursquareCode3::buildKeySquare(const QString& keyword) const
{
    QString alphabet = defaultAlphabet();
    QString used;
    // Add keyword chars (unique, valid)
    for (QChar c : keyword) {
        c = c.toUpper();
        if (alphabet.contains(c) && !used.contains(c))
            used.append(c);
    }
    // Append remaining alphabet
    for (QChar c : alphabet) {
        if (!used.contains(c))
            used.append(c);
    }

    QVector<QVector<QChar>> square(SQUARE_SIZE, QVector<QChar>(SQUARE_SIZE));
    int idx = 0;
    for (int r = 0; r < SQUARE_SIZE; ++r)
        for (int c = 0; c < SQUARE_SIZE; ++c)
            square[r][c] = used[idx++];
    return square;
}

/* ---- Build square (public wrapper) ---- */

QVector<QVector<QChar>> FoursquareCode3::buildSquare(const QString& keyword) const
{
    return buildKeySquare(keyword);
}

/* ---- Find character position ---- */

QPair<int, int> FoursquareCode3::findPosition(const QVector<QVector<QChar>>& square, QChar ch) const
{
    ch = ch.toUpper();
    for (int r = 0; r < SQUARE_SIZE; ++r)
        for (int c = 0; c < SQUARE_SIZE; ++c)
            if (square[r][c] == ch)
                return {r, c};
    return {-1, -1};
}

/* ---- Get character at position ---- */

QChar FoursquareCode3::charAt(const QVector<QVector<QChar>>& square, int row, int col) const
{
    if (row < 0 || row >= SQUARE_SIZE || col < 0 || col >= SQUARE_SIZE)
        return QChar();
    return square[row][col];
}

/* ---- Prepare text ---- */

QString FoursquareCode3::prepareText(const QString& text) const
{
    QString result;
    QString alpha = defaultAlphabet();
    for (QChar c : text.toUpper()) {
        if (alpha.contains(c))
            result.append(c);
    }
    // Pad with 'X' if odd length
    if (result.size() % 2 != 0)
        result.append('X');
    return result;
}

/* ---- Process a digraph pair ---- */

QString FoursquareCode3::processPair(QChar a, QChar b,
                                      const QVector<QVector<QChar>>& sq1,
                                      const QVector<QVector<QChar>>& sq2,
                                      bool encrypt) const
{
    // Plain square (no keyword)
    QVector<QVector<QChar>> plainSq = buildKeySquare(QString());
    auto [r1, c1] = findPosition(plainSq, a);
    auto [r2, c2] = findPosition(plainSq, b);
    if (r1 < 0 || r2 < 0) return QString(a).append(b);

    QString result;
    if (encrypt) {
        // Encrypt: a uses sq1 row, b uses sq2 col
        result.append(charAt(sq1, r1, c2));
        result.append(charAt(sq2, r2, c1));
    } else {
        // Decrypt: reverse mapping
        result.append(charAt(sq1, r1, c2));
        result.append(charAt(sq2, r2, c1));
    }
    return result;
}

/* ---- Encrypt ---- */

QString FoursquareCode3::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();
    QString prepared = prepareText(plaintext);
    QVector<QVector<QChar>> sq1 = buildKeySquare(m_keyword1);
    QVector<QVector<QChar>> sq2 = buildKeySquare(m_keyword2);

    QString ciphertext;
    for (int i = 0; i < prepared.size(); i += 2) {
        ciphertext.append(processPair(prepared[i], prepared[i + 1], sq1, sq2, true));
    }

    const_cast<FoursquareCode3*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode3*>(this)->m_stats.inputLength = plaintext.size();
    const_cast<FoursquareCode3*>(this)->m_stats.outputLength = ciphertext.size();
    const_cast<FoursquareCode3*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    const_cast<FoursquareCode3*>(this)->emit cipherCompleted("encrypt", ciphertext.size(), timer.elapsed());
    return ciphertext;
}

/* ---- Decrypt ---- */

QString FoursquareCode3::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();
    QString prepared = ciphertext.toUpper();
    QVector<QVector<QChar>> sq1 = buildKeySquare(m_keyword1);
    QVector<QVector<QChar>> sq2 = buildKeySquare(m_keyword2);

    QString plaintext;
    for (int i = 0; i + 1 < prepared.size(); i += 2) {
        plaintext.append(processPair(prepared[i], prepared[i + 1], sq1, sq2, false));
    }

    const_cast<FoursquareCode3*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode3*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    const_cast<FoursquareCode3*>(this)->emit cipherCompleted("decrypt", plaintext.size(), timer.elapsed());
    return plaintext;
}

/* ---- Reset ---- */

void FoursquareCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
