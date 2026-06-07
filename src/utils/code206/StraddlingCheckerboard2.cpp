/**
 * @file StraddlingCheckerboard2.cpp
 * @brief StraddlingCheckerboard2 实现
 *
 * 实现跨棋盘密码：优化棋盘布局生成、助记密钥调度、加密/解密。
 */

#include "utils/code206/StraddlingCheckerboard2.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Constants ---- */

const QString StraddlingCheckerboard2::ALPHABET =
    QStringLiteral("ES TO-NR IAMBDCKLGHFPUVWXYZ/.QJ0123456789");

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard2::StraddlingCheckerboard2(QObject *parent) : QObject(parent)
{
    buildLayout();
}

StraddlingCheckerboard2::~StraddlingCheckerboard2() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard2::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper();
    buildLayout();
}

void StraddlingCheckerboard2::setBlankRows(int row1, int row2)
{
    m_blankRow1 = qBound(0, row1, 9);
    m_blankRow2 = qBound(0, row2, 9);
    buildLayout();
}

/* ---- Sanitize input ---- */

QString StraddlingCheckerboard2::sanitize(const QString& text)
{
    QString result;
    for (const QChar& ch : text.toUpper()) {
        if (ch.isLetterOrNumber() || ch == QLatin1Char('.') || ch == QLatin1Char('/')
            || ch == QLatin1Char('-'))
            result.append(ch);
    }
    return result;
}

/* ---- Mnemonic key scheduling ---- */

QVector<int> StraddlingCheckerboard2::mnemonicSchedule() const
{
    // Derive column permutation from keyword using alphabetical ordering
    QString key = m_keyword.isEmpty() ? QStringLiteral("KEYWORD") : m_keyword;
    int n = key.size();

    QVector<QPair<QChar, int>> indexed;
    for (int i = 0; i < n; ++i)
        indexed.append({key[i], i});

    // Sort by character value to create permutation
    std::stable_sort(indexed.begin(), indexed.end(),
        [](const QPair<QChar, int>& a, const QPair<QChar, int>& b) {
            return a.first < b.first;
        });

    QVector<int> perm(n);
    for (int i = 0; i < n; ++i)
        perm[indexed[i].second] = i;
    return perm;
}

/* ---- Build layout ---- */

void StraddlingCheckerboard2::buildLayout()
{
    m_layout.resize(3);
    for (int r = 0; r < 3; ++r)
        m_layout[r].resize(10, QChar());

    m_reverseMap.clear();

    // First row: characters at non-blank columns
    QVector<int> blankCols = {m_blankRow1, m_blankRow2};
    QVector<int> normalCols;
    for (int c = 0; c < 10; ++c) {
        if (!blankCols.contains(c))
            normalCols.append(c);
    }

    // Use mnemonic scheduling to reorder alphabet
    QString alphabet = ALPHABET;
    QVector<int> schedule = mnemonicSchedule();

    // Apply schedule to extend key effect
    QString orderedAlpha;
    QSet<QChar> used;
    // First add keyword chars
    for (const QChar& ch : m_keyword) {
        if (!used.contains(ch) && alphabet.contains(ch)) {
            orderedAlpha.append(ch);
            used.insert(ch);
        }
    }
    // Then remaining alphabet
    for (const QChar& ch : alphabet) {
        if (!used.contains(ch)) {
            orderedAlpha.append(ch);
            used.insert(ch);
        }
    }

    int idx = 0;

    // Row 0: fill 8 normal columns
    for (int c : normalCols) {
        if (idx < orderedAlpha.size()) {
            m_layout[0][c] = orderedAlpha[idx];
            m_reverseMap[orderedAlpha[idx]] = {0, c};
            idx++;
        }
    }

    // Row 1: indexed by m_blankRow1
    for (int c = 0; c < 10 && idx < orderedAlpha.size(); ++c) {
        m_layout[1][c] = orderedAlpha[idx];
        m_reverseMap[orderedAlpha[idx]] = {1, c};
        idx++;
    }

    // Row 2: indexed by m_blankRow2
    for (int c = 0; c < 10 && idx < orderedAlpha.size(); ++c) {
        m_layout[2][c] = orderedAlpha[idx];
        m_reverseMap[orderedAlpha[idx]] = {2, c};
        idx++;
    }
}

/* ---- Generate layout ---- */

QVector<QVector<QChar>> StraddlingCheckerboard2::generateLayout() const
{
    return m_layout;
}

/* ---- Get layout ---- */

QVector<QVector<QChar>> StraddlingCheckerboard2::getLayout() const
{
    return m_layout;
}

/* ---- Encrypt ---- */

QString StraddlingCheckerboard2::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString clean = sanitize(plaintext);
    QString result;

    for (const QChar& ch : clean) {
        QChar upper = ch.toUpper();
        if (m_reverseMap.contains(upper)) {
            auto pos = m_reverseMap[upper];
            if (pos.first == 0) {
                // Single digit encoding
                result.append(QChar('0' + pos.second));
            } else {
                // Two digit encoding: blankRow digit + column digit
                int rowDigit = (pos.first == 1) ? m_blankRow1 : m_blankRow2;
                result.append(QChar('0' + rowDigit));
                result.append(QChar('0' + pos.second));
            }
        }
    }

    const_cast<StraddlingCheckerboard2*>(this)->m_stats.totalOps++;
    const_cast<StraddlingCheckerboard2*>(this)->m_stats.inputLength = clean.size();
    const_cast<StraddlingCheckerboard2*>(this)->m_stats.outputLength = result.size();
    const_cast<StraddlingCheckerboard2*>(this)->m_timeSum += timer.elapsed();
    const_cast<StraddlingCheckerboard2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    const_cast<StraddlingCheckerboard2*>(this)->emit
        cipherCompleted("encrypt", clean.size(), result.size(), timer.elapsed());

    return result;
}

/* ---- Decrypt ---- */

QString StraddlingCheckerboard2::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    int i = 0;
    int len = ciphertext.size();

    while (i < len) {
        int digit = ciphertext[i].digitValue();
        if (digit < 0) { i++; continue; }

        if (digit == m_blankRow1) {
            // Two-digit from row 1
            if (i + 1 < len) {
                int col = ciphertext[i + 1].digitValue();
                if (col >= 0 && col < 10 && m_layout[1][col].isPrint())
                    result.append(m_layout[1][col]);
                i += 2;
            } else { i++; }
        } else if (digit == m_blankRow2) {
            // Two-digit from row 2
            if (i + 1 < len) {
                int col = ciphertext[i + 1].digitValue();
                if (col >= 0 && col < 10 && m_layout[2][col].isPrint())
                    result.append(m_layout[2][col]);
                i += 2;
            } else { i++; }
        } else {
            // Single digit from row 0
            if (m_layout[0][digit].isPrint())
                result.append(m_layout[0][digit]);
            i++;
        }
    }

    const_cast<StraddlingCheckerboard2*>(this)->m_stats.totalOps++;
    const_cast<StraddlingCheckerboard2*>(this)->m_stats.inputLength = len;
    const_cast<StraddlingCheckerboard2*>(this)->m_stats.outputLength = result.size();
    const_cast<StraddlingCheckerboard2*>(this)->m_timeSum += timer.elapsed();
    const_cast<StraddlingCheckerboard2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    const_cast<StraddlingCheckerboard2*>(this)->emit
        cipherCompleted("decrypt", len, result.size(), timer.elapsed());

    return result;
}

/* ---- Reset ---- */

void StraddlingCheckerboard2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
