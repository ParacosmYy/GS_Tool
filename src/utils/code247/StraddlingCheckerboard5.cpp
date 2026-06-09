/**
 * @file StraddlingCheckerboard5.cpp
 * @brief StraddlingCheckerboard5 实现
 *
 * 实现跨界棋盘密码：莫尔斯码启发网格布局与频率优化空单元放置。
 */

#include "utils/code247/StraddlingCheckerboard5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard5::StraddlingCheckerboard5(QObject *parent) : QObject(parent)
{
    m_charRow.resize(256, -1);
    m_charCol.resize(256, -1);
    buildBoard();
}

StraddlingCheckerboard5::~StraddlingCheckerboard5() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard5::setKeyPhrase(const QString& phrase)
{
    m_keyPhrase = phrase;
    buildBoard();
}

void StraddlingCheckerboard5::setStraddleRows(int row1, int row2)
{
    m_straddleRow1 = qBound(0, row1, 9);
    m_straddleRow2 = qBound(0, row2, 9);
    if (m_straddleRow1 == m_straddleRow2)
        m_straddleRow2 = (m_straddleRow1 + 1) % 10;
    buildBoard();
}

/* ---- Frequency ordering (English letter frequency) ---- */

QVector<int> StraddlingCheckerboard5::frequencyOrder() const
{
    // English letter frequency: E T A O I N S H R...
    return {4, 19, 0, 14, 8, 13, 18, 7, 17, 11, 3, 2, 20, 15, 12, 5, 10,
            6, 23, 21, 25, 22, 9, 1, 16, 24};
}

/* ---- Generate permuted alphabet from key ---- */

QVector<QChar> StraddlingCheckerboard5::permuteAlphabet() const
{
    QVector<QChar> alpha;
    QChar used[26] = {};
    int usedCount = 0;

    // Add key phrase characters first
    for (QChar ch : m_keyPhrase.toUpper()) {
        if (ch >= 'A' && ch <= 'Z') {
            int idx = ch.toLatin1() - 'A';
            if (!used[idx]) {
                used[idx] = true;
                alpha.append(ch);
                usedCount++;
            }
        }
    }

    // Add remaining letters
    for (int i = 0; i < 26; ++i) {
        if (!used[i]) {
            alpha.append(QChar('A' + i));
        }
    }
    return alpha;
}

/* ---- Build the checkerboard ---- */

void StraddlingCheckerboard5::buildBoard()
{
    // Reset lookup tables
    m_charRow.fill(-1);
    m_charCol.fill(-1);
    m_board.clear();

    auto alpha = permuteAlphabet();
    auto freqOrder = frequencyOrder();

    // Board: row 0 = main row (8 single-digit cells, 2 empty for straddle)
    // Row straddle1 and straddle2 = double-digit cells (10 each)
    m_board.resize(3);
    for (int i = 0; i < 3; ++i)
        m_board[i].resize(10);

    // Place high-frequency letters in single-digit cells (row 0)
    // Skip positions at straddleRow1 and straddleRow2
    QVector<int> emptyCols = {m_straddleRow1, m_straddleRow2};
    int alphaIdx = 0;
    for (int col = 0; col < 10 && alphaIdx < alpha.size(); ++col) {
        if (col == m_straddleRow1 || col == m_straddleRow2) continue;
        QChar ch = alpha[alphaIdx++];
        m_board[0][col] = ch;
        int code = ch.toLatin1();
        if (code >= 0 && code < 256) {
            m_charRow[code] = 0;
            m_charCol[code] = col;
        }
    }

    // Place remaining letters in straddle rows
    int row = 1;
    int col = 0;
    while (alphaIdx < alpha.size()) {
        QChar ch = alpha[alphaIdx++];
        m_board[row][col] = ch;
        int straddleRow = (row == 1) ? m_straddleRow1 : m_straddleRow2;
        int code = ch.toLatin1();
        if (code >= 0 && code < 256) {
            m_charRow[code] = straddleRow;
            m_charCol[code] = col;
        }
        col++;
        if (col >= 10) { col = 0; row++; }
        if (row >= 3) break;
    }
}

/* ---- Encode ---- */

QString StraddlingCheckerboard5::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    QString upper = plaintext.toUpper();

    for (QChar ch : upper) {
        int code = ch.toLatin1();
        if (code < 0 || code >= 256 || m_charRow[code] < 0) {
            // Unknown character: use escape (99) + code
            result.append("99");
            result.append(QString::number(code).rightJustified(3, '0'));
            continue;
        }
        int row = m_charRow[code];
        int col = m_charCol[code];
        if (row == 0) {
            // Single digit
            result.append(QChar('0' + col));
        } else {
            // Double digit: straddle row index + column
            result.append(QChar('0' + row));
            result.append(QChar('0' + col));
        }
    }

    m_stats.numEncodes++;
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.compressionRatio = (plaintext.size() > 0)
        ? static_cast<double>(result.size()) / plaintext.size() : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodeCompleted(plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QString StraddlingCheckerboard5::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    int i = 0;
    while (i < ciphertext.size()) {
        int d = ciphertext[i].toLatin1() - '0';
        if (d < 0 || d > 9) { i++; continue; }

        if (d == m_straddleRow1 || d == m_straddleRow2) {
            // Double digit: row + col
            if (i + 1 >= ciphertext.size()) break;
            int col = ciphertext[i + 1].toLatin1() - '0';
            if (col < 0 || col > 9) { i++; continue; }
            int boardRow = (d == m_straddleRow1) ? 1 : 2;
            if (boardRow < m_board.size() && col < m_board[boardRow].size())
                result.append(m_board[boardRow][col]);
            i += 2;
        } else if (d == 9 && i + 2 < ciphertext.size()
                   && ciphertext[i + 1].toLatin1() == '9') {
            // Escape sequence: 99 + 3-digit code
            int charCode = ciphertext.mid(i + 2, 3).toInt();
            result.append(QChar(charCode));
            i += 5;
        } else {
            // Single digit from row 0
            if (d < m_board[0].size())
                result.append(m_board[0][d]);
            i++;
        }
    }

    m_stats.numDecodes++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Get board layout ---- */

QVector<QString> StraddlingCheckerboard5::boardLayout() const
{
    QVector<QString> layout;
    QString row0 = "Row0: ";
    for (int c = 0; c < 10; ++c) {
        if (c == m_straddleRow1 || c == m_straddleRow2)
            row0 += "[  ] ";
        else
            row0 += QString("[%1] ").arg(m_board[0][c]);
    }
    layout.append(row0);

    if (m_board.size() > 1) {
        QString row1 = QString("Row%1: ").arg(m_straddleRow1);
        for (int c = 0; c < 10 && c < m_board[1].size(); ++c)
            row1 += QString("[%1] ").arg(m_board[1][c]);
        layout.append(row1);
    }
    if (m_board.size() > 2) {
        QString row2 = QString("Row%1: ").arg(m_straddleRow2);
        for (int c = 0; c < 10 && c < m_board[2].size(); ++c)
            row2 += QString("[%1] ").arg(m_board[2][c]);
        layout.append(row2);
    }
    return layout;
}

/* ---- Reset ---- */

void StraddlingCheckerboard5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
