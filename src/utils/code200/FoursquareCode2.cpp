/**
 * @file FoursquareCode2.cpp
 * @brief FoursquareCode2 实现
 *
 * 实现四方密码变体：关键词依赖网格布局、互易二合字母分析、加解密操作。
 */

#include "utils/code200/FoursquareCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode2::FoursquareCode2(QObject *parent) : QObject(parent)
{
    m_squareTL = buildStandardSquare();
    m_squareBR = buildStandardSquare();
    m_squareTR = buildSquare(m_keyword1);
    m_squareBL = buildSquare(m_keyword2);
}

FoursquareCode2::~FoursquareCode2() = default;

/* ---- Configuration ---- */

void FoursquareCode2::setKeyword1(const QString& kw)
{
    m_keyword1 = kw.toUpper();
    m_squareTR = buildSquare(m_keyword1);
}

void FoursquareCode2::setKeyword2(const QString& kw)
{
    m_keyword2 = kw.toUpper();
    m_squareBL = buildSquare(m_keyword2);
}

/* ---- Char <-> Index mapping (A=0..Z=24, J omitted) ---- */

int FoursquareCode2::charToIdx(QChar c)
{
    c = c.toUpper();
    if (c == QChar('J')) c = QChar('I');
    if (c >= QChar('A') && c <= QChar('Z')) {
        int idx = c.toLatin1() - 'A';
        return (idx > 9) ? idx - 1 : idx;  // Skip J (index 9)
    }
    return -1;
}

QChar FoursquareCode2::idxToChar(int idx)
{
    if (idx < 0 || idx >= 25) return QChar('?');
    // Map 0-24 to A-Z skipping J
    int ch = (idx < 9) ? idx : idx + 1;
    return QChar('A' + ch);
}

/* ---- Build standard 5x5 square (A-Z, skip J) ---- */

QVector<QVector<int>> FoursquareCode2::buildStandardSquare() const
{
    QVector<QVector<int>> sq(5, QVector<int>(5));
    for (int i = 0; i < 25; ++i) sq[i / 5][i % 5] = i;
    return sq;
}

/* ---- Build 5x5 square from keyword ---- */

QVector<QVector<int>> FoursquareCode2::buildSquare(const QString& keyword) const
{
    QVector<bool> used(25, false);
    QVector<int> order;

    // Add keyword letters first
    for (QChar c : keyword) {
        int idx = charToIdx(c);
        if (idx >= 0 && !used[idx]) { used[idx] = true; order.append(idx); }
    }
    // Fill remaining
    for (int i = 0; i < 25; ++i)
        if (!used[i]) order.append(i);

    QVector<QVector<int>> sq(5, QVector<int>(5));
    for (int i = 0; i < 25; ++i) sq[i / 5][i % 5] = order[i];
    return sq;
}

/* ---- Find position of letter in square ---- */

QPair<int, int> FoursquareCode2::findPosition(const QVector<QVector<int>>& square, int letter) const
{
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            if (square[r][c] == letter) return {r, c};
    return {0, 0};
}

/* ---- Encode digraph pair ---- */

QPair<int, int> FoursquareCode2::encodePair(const QPair<int, int>& p1,
                                              const QPair<int, int>& p2,
                                              bool encrypt) const
{
    // Four-square: TL/TR for first letter, BL/BR for second
    int r1 = p1.first, c1 = p1.second;
    int r2 = p2.first, c2 = p2.second;

    if (encrypt) {
        // Encrypted: TR[r1][c2], BL[r2][c1]
        return {r1, c2};
    } else {
        // Decrypted: TL[r1][c2], BR[r2][c1]
        return {r1, c2};
    }
}

/* ---- Prepare text ---- */

QString FoursquareCode2::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c.isLetter()) {
            if (c == QChar('J')) c = QChar('I');
            result.append(c);
        }
    }
    // Pad for even length
    if (result.size() % 2 != 0) result.append(QChar('X'));
    return result;
}

/* ---- Encrypt ---- */

QString FoursquareCode2::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prep = prepareText(plaintext);
    QString result;

    for (int i = 0; i < prep.size() - 1; i += 2) {
        int a = charToIdx(prep[i]);
        int b = charToIdx(prep[i + 1]);
        if (a < 0 || b < 0) { result.append("??"); continue; }

        auto posA = findPosition(m_squareTL, a);  // Row from TL
        auto posB = findPosition(m_squareBR, b);  // Col from BR

        int enc1 = m_squareTR[posA.first][posB.second];
        int enc2 = m_squareBL[posB.first][posA.second];

        result.append(idxToChar(enc1));
        result.append(idxToChar(enc2));
    }

    const_cast<FoursquareCode2*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode2*>(this)->m_stats.textLength = plaintext.size();
    const_cast<FoursquareCode2*>(this)->m_stats.digraphCount = prep.size() / 2;
    const_cast<FoursquareCode2*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    const_cast<FoursquareCode2*>(this)->emit operationCompleted("encrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode2::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString prep = prepareText(ciphertext);
    QString result;

    for (int i = 0; i < prep.size() - 1; i += 2) {
        int a = charToIdx(prep[i]);
        int b = charToIdx(prep[i + 1]);
        if (a < 0 || b < 0) { result.append("??"); continue; }

        auto posA = findPosition(m_squareTR, a);
        auto posB = findPosition(m_squareBL, b);

        int dec1 = m_squareTL[posA.first][posB.second];
        int dec2 = m_squareBR[posB.first][posA.second];

        result.append(idxToChar(dec1));
        result.append(idxToChar(dec2));
    }

    const_cast<FoursquareCode2*>(this)->m_stats.totalOps++;
    const_cast<FoursquareCode2*>(this)->m_timeSum += timer.elapsed();
    const_cast<FoursquareCode2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    const_cast<FoursquareCode2*>(this)->emit operationCompleted("decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Analyze reciprocal digraphs ---- */

QVector<QPair<QChar, QChar>> FoursquareCode2::analyzeDigraphs(const QString& text) const
{
    QVector<QPair<QChar, QChar>> result;
    QString prep = prepareText(text);
    for (int i = 0; i < prep.size() - 1; i += 2)
        result.append({prep[i], prep[i + 1]});
    return result;
}

/* ---- Reset ---- */

void FoursquareCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
