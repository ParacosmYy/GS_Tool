/**
 * @file StraddlingCheckerboard.cpp
 * @brief StraddlingCheckerboard 实现
 *
 * 实现跨棋盘密码：混合宽度数字编码、助记关键词布局优化、空位选择策略。
 */

#include "utils/code192/StraddlingCheckerboard.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QMap>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard::StraddlingCheckerboard(QObject *parent)
    : QObject(parent)
{
    buildBoard();
}

StraddlingCheckerboard::~StraddlingCheckerboard() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper();
    buildBoard();
}

void StraddlingCheckerboard::setBlankPositions(int blank1, int blank2)
{
    // Ensure valid distinct blank positions
    m_blank1 = qBound(0, blank1, 9);
    m_blank2 = qBound(0, blank2, 9);
    if (m_blank1 == m_blank2)
        m_blank2 = (m_blank1 + 1) % 10;
    buildBoard();
}

/* ---- Initialize default board ---- */

void StraddlingCheckerboard::initDefaultBoard()
{
    // Default alphabet: ESTONIARBCDFGHKLMPQUVWXYZ./#
    // Row 0: 10 chars (indexed 0-9, blanks at m_blank1, m_blank2)
    // Row 1: 10 chars (indexed m_blank1 + 0-9)
    // Row 2: 10 chars (indexed m_blank2 + 0-9)

    m_board.resize(3);
    for (int r = 0; r < 3; ++r)
        m_board[r].resize(10, QChar(' '));

    // Row 0 has blanks at m_blank1 and m_blank2
    QString row0Chars;
    QString allChars = "ESTONIARBCDFGHKLMPQUVWXYZ.#";
    int idx = 0;

    for (int c = 0; c < 10; ++c) {
        if (c == m_blank1 || c == m_blank2) {
            m_board[0][c] = QChar(' ');  // blank
        } else {
            m_board[0][c] = idx < allChars.size() ? allChars[idx++] : QChar(' ');
        }
    }

    // Rows 1 and 2 fill from remaining chars
    for (int c = 0; c < 10 && idx < allChars.size(); ++c)
        m_board[1][c] = allChars[idx++];
    for (int c = 0; c < 10 && idx < allChars.size(); ++c)
        m_board[2][c] = allChars[idx++];
}

/* ---- Build optimized board ---- */

void StraddlingCheckerboard::buildBoard()
{
    if (m_keyword.isEmpty()) {
        initDefaultBoard();
        rebuildLookup();
        return;
    }

    m_board.resize(3);
    for (int r = 0; r < 3; ++r)
        m_board[r].resize(10, QChar(' '));

    // Build alphabet from keyword (unique, preserving order)
    QString used;
    QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.#";
    QString keyChars;

    for (QChar ch : m_keyword) {
        if (ch.isLetter() && !used.contains(ch)) {
            used.append(ch);
            keyChars.append(ch);
        }
    }

    // Remaining chars in standard order
    for (QChar ch : alphabet) {
        if (ch == '.' || ch == '#' || ch.isLetter()) {
            if (!used.contains(ch))
                keyChars.append(ch);
        }
    }

    // Place into board: row 0 fills non-blank cols
    int idx = 0;
    for (int c = 0; c < 10; ++c) {
        if (c == m_blank1 || c == m_blank2) {
            m_board[0][c] = QChar(' ');
        } else {
            m_board[0][c] = idx < keyChars.size() ? keyChars[idx++] : QChar(' ');
        }
    }

    for (int c = 0; c < 10 && idx < keyChars.size(); ++c)
        m_board[1][c] = keyChars[idx++];
    for (int c = 0; c < 10 && idx < keyChars.size(); ++c)
        m_board[2][c] = keyChars[idx++];

    rebuildLookup();
}

/* ---- Rebuild reverse lookup ---- */

void StraddlingCheckerboard::rebuildLookup()
{
    m_lookup.clear();
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 10; ++c) {
            QChar ch = m_board[r][c];
            if (ch != ' ')
                m_lookup[ch.toUpper()] = {r, c};
        }
}

/* ---- Encode ---- */

QString StraddlingCheckerboard::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    QString upper = plaintext.toUpper();

    for (QChar ch : upper) {
        auto it = m_lookup.find(ch);
        if (it != m_lookup.end()) {
            int row = it.value().first;
            int col = it.value().second;

            if (row == 0) {
                // Direct: single digit
                result.append(QChar('0' + col));
            } else if (row == 1) {
                // Two digits: blank1 prefix + col
                result.append(QChar('0' + m_blank1));
                result.append(QChar('0' + col));
            } else {
                // Two digits: blank2 prefix + col
                result.append(QChar('0' + m_blank2));
                result.append(QChar('0' + col));
            }
        }
        // Unknown chars silently skipped
    }

    const_cast<StraddlingCheckerboard*>(this)->m_stats.totalOperations++;
    const_cast<StraddlingCheckerboard*>(this)->m_stats.lastInputLen = plaintext.size();
    const_cast<StraddlingCheckerboard*>(this)->m_stats.lastOutputLen = result.size();
    m_timeSum += timer.elapsed();
    const_cast<StraddlingCheckerboard*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOperations;

    const_cast<StraddlingCheckerboard*>(this)->operationCompleted(
        "encode", plaintext.size(), result.size());
    return result;
}

/* ---- Decode ---- */

QString StraddlingCheckerboard::decode(const QString& digits) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    int i = 0;
    int len = digits.size();

    while (i < len) {
        int d = digits[i].digitValue();
        if (d < 0) { i++; continue; }

        if (d == m_blank1) {
            // Two-digit: row 1
            if (i + 1 < len) {
                int col = digits[i + 1].digitValue();
                if (col >= 0 && col < 10)
                    result.append(m_board[1][col]);
                i += 2;
            } else {
                i++;
            }
        } else if (d == m_blank2) {
            // Two-digit: row 2
            if (i + 1 < len) {
                int col = digits[i + 1].digitValue();
                if (col >= 0 && col < 10)
                    result.append(m_board[2][col]);
                i += 2;
            } else {
                i++;
            }
        } else {
            // Direct: row 0
            if (d < 10)
                result.append(m_board[0][d]);
            i++;
        }
    }

    const_cast<StraddlingCheckerboard*>(this)->m_stats.totalOperations++;
    const_cast<StraddlingCheckerboard*>(this)->m_stats.lastInputLen = digits.size();
    const_cast<StraddlingCheckerboard*>(this)->m_stats.lastOutputLen = result.size();
    m_timeSum += timer.elapsed();
    const_cast<StraddlingCheckerboard*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOperations;

    const_cast<StraddlingCheckerboard*>(this)->operationCompleted(
        "decode", digits.size(), result.size());
    return result;
}

/* ---- Board layout access ---- */

QVector<QVector<QChar>> StraddlingCheckerboard::boardLayout() const
{
    return m_board;
}

/* ---- Reset ---- */

void StraddlingCheckerboard::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
