/**
 * @file BifidCode.cpp
 * @brief BifidCode 实现
 *
 * 实现Bifid密码：Polybius方格构建、坐标分数化转置、周期密钥分块处理。
 */

#include "utils/code184/BifidCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BifidCode::BifidCode(QObject *parent) : QObject(parent)
{
    m_alphabet = "ABCDEFGHIKLMNOPQRSTUVWXYZ"; // Standard 25-letter (no J)
}

BifidCode::~BifidCode() = default;

/* ---- Configuration ---- */

void BifidCode::setPeriod(int period) { m_period = qMax(1, period); }
void BifidCode::setAlphabet(const QString& alpha) { m_alphabet = alpha; }

/* ---- Text preparation ---- */

QString BifidCode::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c == 'J') result += 'I';
        else if (m_alphabet.contains(c)) result += c;
    }
    return result;
}

/* ---- Polybius square construction ---- */

QVector<QVector<QChar>> BifidCode::buildPolybiusSquare(const QString& key) const
{
    QVector<QVector<QChar>> square(5, QVector<QChar>(5));
    QString usedKey;
    QString fullAlpha = key.toUpper();

    // Remove duplicates and non-alpha from key
    for (QChar c : fullAlpha) {
        QChar uc = (c == 'J') ? 'I' : c;
        if (m_alphabet.contains(uc) && !usedKey.contains(uc))
            usedKey += uc;
    }

    // Fill remaining alphabet
    for (QChar c : m_alphabet) {
        if (!usedKey.contains(c))
            usedKey += c;
    }

    // Fill 5x5 square
    int idx = 0;
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            square[r][c] = usedKey[idx++];

    return square;
}

/* ---- Coordinate lookup ---- */

QPair<int, int> BifidCode::charToCoord(QChar c, const QVector<QVector<QChar>>& square) const
{
    for (int r = 0; r < 5; ++r)
        for (int col = 0; col < 5; ++col)
            if (square[r][col] == c)
                return {r, col};
    return {0, 0};
}

QChar BifidCode::coordToChar(int row, int col, const QVector<QVector<QChar>>& square) const
{
    if (row < 0 || row >= 5 || col < 0 || col >= 5) return 'A';
    return square[row][col];
}

/* ---- Fractionate: interleave row and col coords ---- */

QVector<int> BifidCode::fractionate(const QVector<int>& coords, int period) const
{
    int n = coords.size() / 2; // Number of characters
    if (n == 0) return {};

    QVector<int> result;

    // Process in period-sized blocks
    for (int start = 0; start < n; start += period) {
        int end = qMin(start + period, n);

        // Collect rows then columns
        QVector<int> rows, cols;
        for (int i = start; i < end; ++i) {
            rows.append(coords[2 * i]);
            cols.append(coords[2 * i + 1]);
        }

        // Interleave: rows first, then cols
        for (int r : rows) result.append(r);
        for (int c : cols) result.append(c);
    }

    return result;
}

/* ---- Defractionate: reverse the fractionation ---- */

QVector<int> BifidCode::defractionate(const QVector<int>& coords, int period) const
{
    int n = coords.size() / 2; // Number of characters
    if (n == 0) return {};

    QVector<int> result(2 * n);

    for (int start = 0; start < n; start += period) {
        int end = qMin(start + period, n);
        int blockSize = end - start;

        // First blockSize entries are rows, next blockSize are cols
        for (int i = 0; i < blockSize; ++i) {
            result[2 * (start + i)] = coords[start + i];
            result[2 * (start + i) + 1] = coords[start + blockSize + i];
        }
    }

    return result;
}

/* ---- Encrypt ---- */

QString BifidCode::encrypt(const QString& plaintext, const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    auto square = buildPolybiusSquare(key);
    QString prepared = prepareText(plaintext);

    // Convert to row,col coordinates
    QVector<int> coords;
    for (QChar c : prepared) {
        auto coord = charToCoord(c, square);
        coords.append(coord.first);
        coords.append(coord.second);
    }

    // Fractionate with period
    auto frac = fractionate(coords, m_period);

    // Convert back to characters using paired coordinates
    QString ciphertext;
    for (int i = 0; i + 1 < frac.size(); i += 2)
        ciphertext += coordToChar(frac[i], frac[i + 1], square);

    m_stats.totalOperations++;
    m_stats.inputLength = prepared.size();
    m_stats.period = m_period;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", ciphertext.size());
    return ciphertext;
}

/* ---- Decrypt ---- */

QString BifidCode::decrypt(const QString& ciphertext, const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    auto square = buildPolybiusSquare(key);
    QString prepared = prepareText(ciphertext);

    // Convert to coordinates
    QVector<int> coords;
    for (QChar c : prepared) {
        auto coord = charToCoord(c, square);
        coords.append(coord.first);
        coords.append(coord.second);
    }

    // Defractionate with period
    auto defrac = defractionate(coords, m_period);

    // Convert back to characters
    QString plaintext;
    for (int i = 0; i + 1 < defrac.size(); i += 2)
        plaintext += coordToChar(defrac[i], defrac[i + 1], square);

    m_stats.totalOperations++;
    m_stats.inputLength = prepared.size();
    m_stats.period = m_period;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decrypt", plaintext.size());
    return plaintext;
}

/* ---- Reset ---- */

void BifidCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
