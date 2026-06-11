/**
 * @file StraddlingCheckerboard8.cpp
 * @brief StraddlingCheckerboard8 实现
 *
 * 实现跨骑棋盘密码：变宽行分区与位置依赖单表混合实现不规则分数化。
 */

#include "utils/code289/StraddlingCheckerboard8.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtMath>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard8::StraddlingCheckerboard8(QObject *parent)
    : QObject(parent)
{
    // Default row widths: 10 columns for row 0, variable for others
    m_rowWidths = {10, 10, 10};
    m_totalCols = 10;
}

StraddlingCheckerboard8::~StraddlingCheckerboard8() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard8::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper();
    buildBoard();
}

void StraddlingCheckerboard8::setRowWidths(const QVector<int>& widths)
{
    m_rowWidths = widths;
    if (!widths.isEmpty()) m_totalCols = widths[0];
    buildBoard();
}

void StraddlingCheckerboard8::setShiftKeys(const QVector<QVector<int>>& keys)
{
    m_shiftKeys = keys;
}

/* ---- Expand keyword into full alphabet ---- */

QString StraddlingCheckerboard8::expandAlphabet(const QString& keyword) const
{
    QString alpha = keyword;
    // Standard alphabet + space + digits + extra chars to fill 30+ slots
    const QString rest = QStringLiteral(" ESTONIARBCDFGHKLMPQUVWXYZ./0123456789");
    QSet<QChar> seen;
    for (const QChar& c : alpha) seen.insert(c);
    for (const QChar& c : rest) {
        if (!seen.contains(c)) {
            alpha.append(c);
            seen.insert(c);
        }
    }
    return alpha;
}

/* ---- Build checkerboard ---- */

void StraddlingCheckerboard8::buildBoard()
{
    QString alpha = expandAlphabet(m_keyword);
    int numRows = m_rowWidths.size();
    if (numRows == 0) { numRows = 3; m_rowWidths = {10, 10, 10}; }

    // Determine blank positions in row 0 (where other rows start)
    m_blankCols.clear();
    if (numRows > 1) m_blankCols.append(2);
    if (numRows > 2) m_blankCols.append(6);
    // Extend blanks for additional rows
    for (int i = 3; i < numRows; ++i) {
        m_blankCols.append(qMin(i * 2 + 1, m_totalCols - 1));
    }

    // Fill board
    m_board.clear();
    m_board.resize(numRows);
    for (int r = 0; r < numRows; ++r)
        m_board[r].resize(m_rowWidths[r]);

    // Row 0: fill non-blank positions
    int alphaIdx = 0;
    for (int c = 0; c < m_totalCols && alphaIdx < alpha.size(); ++c) {
        if (m_blankCols.contains(c)) {
            m_board[0][c] = QChar::Null;   // Blank: starts a sub-row
        } else {
            m_board[0][c] = alpha[alphaIdx++];
        }
    }

    // Sub-rows fill with remaining alphabet
    for (int r = 1; r < numRows && alphaIdx < alpha.size(); ++r) {
        for (int c = 0; c < m_rowWidths[r] && alphaIdx < alpha.size(); ++c) {
            m_board[r][c] = alpha[alphaIdx++];
        }
    }
}

/* ---- Apply position-dependent shift ---- */

int StraddlingCheckerboard8::applyShift(int digit, int position) const
{
    if (m_shiftKeys.isEmpty()) return digit;
    int row = position % m_shiftKeys.size();
    int col = position % (m_shiftKeys[row].isEmpty() ? 1 : m_shiftKeys[row].size());
    if (row < m_shiftKeys.size() && col < m_shiftKeys[row].size())
        return (digit + m_shiftKeys[row][col]) % 10;
    return digit;
}

/* ---- Reverse position-dependent shift ---- */

int StraddlingCheckerboard8::reverseShift(int digit, int position) const
{
    if (m_shiftKeys.isEmpty()) return digit;
    int row = position % m_shiftKeys.size();
    int col = position % (m_shiftKeys[row].isEmpty() ? 1 : m_shiftKeys[row].size());
    if (row < m_shiftKeys.size() && col < m_shiftKeys[row].size())
        return (digit - m_shiftKeys[row][col] % 10 + 10) % 10;
    return digit;
}

/* ---- Encode ---- */

QString StraddlingCheckerboard8::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    QString upper = plaintext.toUpper();
    int pos = 0;    // Output position for shift tracking

    for (int i = 0; i < upper.size(); ++i) {
        QChar ch = upper[i];
        if (ch == QLatin1Char(' ')) ch = QLatin1Char('E'); // Map space

        bool found = false;
        // Search in row 0 first
        for (int c = 0; c < m_totalCols; ++c) {
            if (m_board.isEmpty() || m_board[0].size() <= c) continue;
            if (m_board[0][c] == ch) {
                int d = applyShift(c, pos);
                result.append(QChar(QLatin1Char('0') + d));
                pos++;
                found = true;
                break;
            }
        }
        if (found) continue;

        // Search in sub-rows
        for (int r = 1; r < m_board.size(); ++r) {
            for (int c = 0; c < m_board[r].size(); ++c) {
                if (m_board[r][c] == ch) {
                    // Emit blank column index as row selector, then column
                    int blankIdx = qMin(r - 1, m_blankCols.size() - 1);
                    int rowCode = m_blankCols[blankIdx];
                    int d1 = applyShift(rowCode, pos);
                    result.append(QChar(QLatin1Char('0') + d1));
                    pos++;
                    int d2 = applyShift(c, pos);
                    result.append(QChar(QLatin1Char('0') + d2));
                    pos++;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        // If not found, encode as literal digit
        if (!found) {
            result.append(QStringLiteral("90"));
            result.append(QString::number(ch.unicode()).rightJustified(3, QLatin1Char('0')));
            pos += 5;
        }
    }

    double elapsed = timer.elapsed();
    const_cast<StraddlingCheckerboard8*>(this)->m_stats.inputLength = upper.size();
    const_cast<StraddlingCheckerboard8*>(this)->m_stats.outputLength = result.size();
    const_cast<StraddlingCheckerboard8*>(this)->m_stats.totalOps++;
    const_cast<StraddlingCheckerboard8*>(this)->m_timeSum += elapsed;
    const_cast<StraddlingCheckerboard8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<StraddlingCheckerboard8*>(this)->encodeDone(upper.size(), result.size(), elapsed);
    return result;
}

/* ---- Decode ---- */

QString StraddlingCheckerboard8::decode(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    int pos = 0;
    int i = 0;

    while (i < ciphertext.size()) {
        int d = reverseShift(ciphertext[i].digitValue(), pos);

        // Check if d is a blank column (row selector)
        if (m_blankCols.contains(d) && i + 1 < ciphertext.size()) {
            int rowIdx = m_blankCols.indexOf(d) + 1;
            pos++;
            int col = reverseShift(ciphertext[i + 1].digitValue(), pos);
            pos++;
            if (rowIdx < m_board.size() && col < m_board[rowIdx].size()
                && m_board[rowIdx][col] != QChar::Null) {
                result.append(m_board[rowIdx][col]);
            }
            i += 2;
        } else {
            // Row 0 lookup
            if (d < m_totalCols && !m_board.isEmpty() && d < m_board[0].size()
                && m_board[0][d] != QChar::Null) {
                result.append(m_board[0][d]);
            }
            pos++;
            i++;
        }
    }

    double elapsed = timer.elapsed();
    const_cast<StraddlingCheckerboard8*>(this)->m_stats.totalOps++;
    const_cast<StraddlingCheckerboard8*>(this)->m_timeSum += elapsed;
    const_cast<StraddlingCheckerboard8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Reset ---- */

void StraddlingCheckerboard8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
