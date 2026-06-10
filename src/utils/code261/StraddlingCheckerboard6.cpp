/**
 * @file StraddlingCheckerboard6.cpp
 * @brief StraddlingCheckerboard6 实现
 *
 * 实现跨越棋盘密码：Polybius坐标嵌入与行列移位周期转位置换。
 */

#include "utils/code261/StraddlingCheckerboard6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard6::StraddlingCheckerboard6(QObject *parent)
    : QObject(parent)
{
    m_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    m_blankRows = {0, 4};
}

StraddlingCheckerboard6::~StraddlingCheckerboard6() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard6::setKey(const QString& alphabetKey, const QVector<int>& blankRows)
{
    // Deduplicate and build 26-letter key
    QString key;
    for (QChar ch : alphabetKey.toUpper()) {
        if (ch.isLetter() && !key.contains(ch))
            key.append(ch);
    }
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (!key.contains(c))
            key.append(c);
    }
    m_alphabet = key.left(26);
    m_blankRows = blankRows;
    buildCheckerboard();
}

void StraddlingCheckerboard6::setPeriod(int period)
{
    m_period = qMax(1, period);
}

/* ---- Build checkerboard ---- */

void StraddlingCheckerboard6::buildCheckerboard()
{
    // 3 rows x 10 cols checkerboard
    // Blank rows have no single-digit encoding
    m_board.resize(3);
    for (auto& row : m_board)
        row.resize(10, QChar());

    m_rowMap.resize(26, -1);
    m_colMap.resize(26, -1);

    int alphaIdx = 0;
    for (int row = 0; row < 3; ++row) {
        bool isBlankRow = m_blankRows.contains(row);
        int startCol = isBlankRow ? 1 : 0;
        for (int col = startCol; col < 10 && alphaIdx < 26; ++col) {
            QChar ch = m_alphabet[alphaIdx];
            m_board[row][col] = ch;
            int letterIdx = ch.toLatin1() - 'A';
            m_rowMap[letterIdx] = row;
            m_colMap[letterIdx] = col;
            alphaIdx++;
        }
    }
}

/* ---- Encode single character ---- */

QString StraddlingCheckerboard6::encodeChar(QChar ch) const
{
    ch = ch.toUpper();
    if (!ch.isLetter()) return QString();
    int idx = ch.toLatin1() - 'A';
    int row = m_rowMap[idx];
    int col = m_colMap[idx];
    if (row < 0 || col < 0) return QString();

    // Polybius coordinate embedding:
    // Blank row chars encode as: row_digit + col_digit (2 digits)
    // Other chars encode as: col_digit (1 digit)
    if (m_blankRows.contains(row)) {
        return QString::number(row) + QString::number(col);
    }
    return QString::number(col);
}

/* ---- Periodic transposition (row/column shift) ---- */

QVector<int> StraddlingCheckerboard6::transpose(const QVector<int>& digits, bool encrypt) const
{
    if (digits.isEmpty()) return digits;

    int n = digits.size();
    QVector<int> result(n);

    // Row/column shift: cyclic shift by period
    for (int i = 0; i < n; ++i) {
        int shift = (i / m_period + 1) % 10;
        if (encrypt) {
            result[i] = (digits[i] + shift) % 10;
        } else {
            result[i] = (digits[i] - shift + 10) % 10;
        }
    }
    return result;
}

/* ---- Parse digits ---- */

QVector<int> StraddlingCheckerboard6::parseDigits(const QString& text) const
{
    QVector<int> digits;
    for (QChar ch : text) {
        if (ch.isDigit())
            digits.append(ch.digitValue());
    }
    return digits;
}

/* ---- Encode ---- */

QString StraddlingCheckerboard6::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Polybius coordinate embedding
    QString rawDigits;
    for (QChar ch : plaintext) {
        QString encoded = encodeChar(ch);
        if (!encoded.isEmpty())
            rawDigits.append(encoded);
    }

    // Step 2: Extract digit array
    QVector<int> digits = parseDigits(rawDigits);

    // Step 3: Periodic transposition layering
    QVector<int> transposed = transpose(digits, true);

    // Step 4: Convert back to string
    QString result;
    for (int d : transposed)
        result.append(QChar('0' + d));

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.numPeriods = (result.size() + m_period - 1) / m_period;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodingCompleted(plaintext.size(), result.size(), elapsed);
    return result;
}

/* ---- Decode ---- */

QString StraddlingCheckerboard6::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Parse digits
    QVector<int> digits = parseDigits(ciphertext);

    // Step 2: Reverse transposition
    QVector<int> untransposed = transpose(digits, false);

    // Step 3: Decode using checkerboard
    QString result;
    int i = 0;
    while (i < untransposed.size()) {
        int d = untransposed[i];
        bool matched = false;

        // Check if this digit is a blank row prefix
        for (int br : m_blankRows) {
            if (d == br && i + 1 < untransposed.size()) {
                int col = untransposed[i + 1];
                if (br < m_board.size() && col < m_board[br].size()) {
                    QChar ch = m_board[br][col];
                    if (!ch.isNull()) {
                        result.append(ch);
                        i += 2;
                        matched = true;
                        break;
                    }
                }
            }
        }

        if (!matched) {
            // Single-digit: non-blank row character (row 0 or non-blank)
            // Try non-blank rows
            for (int row = 0; row < 3; ++row) {
                if (m_blankRows.contains(row)) continue;
                if (d < m_board[row].size()) {
                    QChar ch = m_board[row][d];
                    if (!ch.isNull()) {
                        result.append(ch);
                        i++;
                        break;
                    }
                }
            }
            if (!matched && i < untransposed.size()) {
                // Fallback: skip unknown
                i++;
            }
        }
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodingCompleted(ciphertext.size(), result.size(), elapsed);
    return result;
}

/* ---- Get checkerboard layout ---- */

QVector<QVector<QChar>> StraddlingCheckerboard6::checkerboard() const
{
    return m_board;
}

/* ---- Reset ---- */

void StraddlingCheckerboard6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
