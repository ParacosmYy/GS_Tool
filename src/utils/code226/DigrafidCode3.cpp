/**
 * @file DigrafidCode3.cpp
 * @brief DigrafidCode3 实现
 *
 * 实现Digrafid密码：三图替换与双关键字分数化网格加密解密。
 */

#include "utils/code226/DigrafidCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode3::DigrafidCode3(QObject *parent)
    : QObject(parent), m_alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
{
}

DigrafidCode3::~DigrafidCode3() = default;

/* ---- Configuration ---- */

void DigrafidCode3::setKeys(const QString& keyword1,
                              const QString& keyword2, int period)
{
    m_key1 = keyword1.toUpper();
    m_key2 = keyword2.toUpper();
    m_period = qMax(3, period);

    m_grid1 = buildGrid(m_key1);
    m_grid2 = buildGrid(m_key2);
}

/* ---- Process keyword ---- */

QString DigrafidCode3::processKeyword(const QString& key) const
{
    QString result;
    QString upper = key.toUpper();
    for (QChar ch : upper) {
        if (m_alphabet.contains(ch) && !result.contains(ch))
            result.append(ch);
    }
    return result;
}

/* ---- Build fractionation grid ---- */

QVector<QVector<QChar>> DigrafidCode3::buildGrid(
    const QString& keyword) const
{
    QString processed = processKeyword(keyword);
    QString remaining;
    for (QChar ch : m_alphabet) {
        if (!processed.contains(ch))
            remaining.append(ch);
    }
    QString full = processed + remaining;

    // Grid size: sqrt of alphabet size
    int gridSize = qCeil(qSqrt(static_cast<double>(full.size())));
    QVector<QVector<QChar>> grid(gridSize,
                                  QVector<QChar>(gridSize, QChar(' ')));

    int idx = 0;
    for (int r = 0; r < gridSize; ++r)
        for (int c = 0; c < gridSize; ++c)
            if (idx < full.size())
                grid[r][c] = full[idx++];

    const_cast<DigrafidCode3*>(this)->m_stats.gridRows = gridSize;
    const_cast<DigrafidCode3*>(this)->m_stats.gridCols = gridSize;
    return grid;
}

/* ---- Validate keyword ---- */

bool DigrafidCode3::validateKeyword(const QString& keyword) const
{
    if (keyword.isEmpty()) return false;
    QString upper = keyword.toUpper();
    QSet<QChar> seen;
    for (QChar ch : upper) {
        if (!m_alphabet.contains(ch)) return false;
        if (seen.contains(ch)) return false;
        seen.insert(ch);
    }
    return true;
}

/* ---- Find character in grid ---- */

QPair<int, int> DigrafidCode3::findInGrid(
    const QVector<QVector<QChar>>& grid, QChar ch) const
{
    for (int r = 0; r < grid.size(); ++r)
        for (int c = 0; c < grid[r].size(); ++c)
            if (grid[r][c] == ch) return {r, c};
    return {-1, -1};
}

/* ---- Position to digit ---- */

int DigrafidCode3::posToDigit(int row, int col, int gridSize) const
{
    return row * gridSize + col;
}

/* ---- Digit to position ---- */

QPair<int, int> DigrafidCode3::digitToPos(int digit, int gridSize) const
{
    return {digit / gridSize, digit % gridSize};
}

/* ---- Encrypt ---- */

QString DigrafidCode3::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString text = plaintext.toUpper();
    QString filtered;
    for (QChar ch : text)
        if (m_alphabet.contains(ch)) filtered.append(ch);

    int gridSize = m_grid1.size();
    QString result;
    int n = filtered.size();
    int period = m_period;

    // Process in groups of 'period' trigraphs
    for (int g = 0; g < n; g += period * 3) {
        int groupLen = qMin(period * 3, n - g);
        QVector<int> digits;

        // Convert trigraphs to digit triples
        for (int i = g; i + 2 < g + groupLen; i += 3) {
            auto p1 = findInGrid(m_grid1, filtered[i]);
            auto p2 = findInGrid(m_grid2, filtered[i + 1]);
            auto p3 = findInGrid(m_grid1, filtered[i + 2]);

            digits.append(posToDigit(p1.first, p1.second, gridSize));
            digits.append(posToDigit(p2.first, p2.second, gridSize));
            digits.append(posToDigit(p3.first, p3.second, gridSize));
        }

        // Columnar transposition within period group
        int numDigits = digits.size();
        int rows = qCeil(static_cast<double>(numDigits) / 3);
        QVector<QVector<int>> cols(3);
        for (int i = 0; i < numDigits; ++i)
            cols[i % 3].append(digits[i]);

        QVector<int> transposed;
        for (int c = 0; c < 3; ++c)
            for (int v : cols[c])
                transposed.append(v);

        // Convert back via grid2
        for (int i = 0; i + 2 < transposed.size(); i += 3) {
            auto p1 = digitToPos(transposed[i], gridSize);
            auto p2 = digitToPos(transposed[i + 1], gridSize);
            auto p3 = digitToPos(transposed[i + 2], gridSize);

            if (p1.first >= 0 && p1.first < gridSize &&
                p2.first >= 0 && p2.first < gridSize &&
                p3.first >= 0 && p3.first < gridSize) {
                result.append(m_grid2[p1.first][p1.second]);
                result.append(m_grid1[p2.first][p2.second]);
                result.append(m_grid2[p3.first][p3.second]);
            }
        }
    }

    const_cast<DigrafidCode3*>(this)->m_stats.numEncryptions++;
    const_cast<DigrafidCode3*>(this)->m_stats.inputLength = filtered.size();
    const_cast<DigrafidCode3*>(this)->m_stats.totalOps++;
    const_cast<DigrafidCode3*>(this)->m_timeSum += timer.elapsed();
    const_cast<DigrafidCode3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<DigrafidCode3*>(this)->encryptionCompleted(
        filtered.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString DigrafidCode3::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString text = ciphertext.toUpper();
    QString filtered;
    for (QChar ch : text)
        if (m_alphabet.contains(ch)) filtered.append(ch);

    int gridSize = m_grid2.size();
    QString result;
    int n = filtered.size();
    int period = m_period;

    for (int g = 0; g < n; g += period * 3) {
        int groupLen = qMin(period * 3, n - g);
        QVector<int> digits;

        for (int i = g; i + 2 < g + groupLen; i += 3) {
            auto p1 = findInGrid(m_grid2, filtered[i]);
            auto p2 = findInGrid(m_grid1, filtered[i + 1]);
            auto p3 = findInGrid(m_grid2, filtered[i + 2]);

            digits.append(posToDigit(p1.first, p1.second, gridSize));
            digits.append(posToDigit(p2.first, p2.second, gridSize));
            digits.append(posToDigit(p3.first, p3.second, gridSize));
        }

        // Reverse columnar transposition
        int numDigits = digits.size();
        int rows = numDigits / 3;
        QVector<int> original;
        QVector<int> col0, col1, col2;
        for (int i = 0; i < rows && i < numDigits; ++i)
            col0.append(digits[i]);
        for (int i = rows; i < rows * 2 && i < numDigits; ++i)
            col1.append(digits[i]);
        for (int i = rows * 2; i < rows * 3 && i < numDigits; ++i)
            col2.append(digits[i]);

        for (int r = 0; r < rows; ++r) {
            if (r < col0.size()) original.append(col0[r]);
            if (r < col1.size()) original.append(col1[r]);
            if (r < col2.size()) original.append(col2[r]);
        }

        for (int i = 0; i + 2 < original.size(); i += 3) {
            auto p1 = digitToPos(original[i], gridSize);
            auto p2 = digitToPos(original[i + 1], gridSize);
            auto p3 = digitToPos(original[i + 2], gridSize);

            if (p1.first >= 0 && p1.first < gridSize &&
                p2.first >= 0 && p2.first < gridSize &&
                p3.first >= 0 && p3.first < gridSize) {
                result.append(m_grid1[p1.first][p1.second]);
                result.append(m_grid2[p2.first][p2.second]);
                result.append(m_grid1[p3.first][p3.second]);
            }
        }
    }

    const_cast<DigrafidCode3*>(this)->m_stats.numDecryptions++;
    const_cast<DigrafidCode3*>(this)->m_stats.totalOps++;
    const_cast<DigrafidCode3*>(this)->m_timeSum += timer.elapsed();
    const_cast<DigrafidCode3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<DigrafidCode3*>(this)->decryptionCompleted(
        filtered.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DigrafidCode3::resetStatistics()
{
    m_grid1.clear();
    m_grid2.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
