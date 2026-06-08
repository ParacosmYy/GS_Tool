/**
 * @file BazeleriesCode2.cpp
 * @brief BazeleriesCode2 实现
 *
 * 实现Bazeleries密码：组合分馏、交替列行置换、扩散加密。
 */

#include "utils/code215/BazeleriesCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BazeleriesCode2::BazeleriesCode2(QObject *parent) : QObject(parent)
{
    m_alphabet = buildAlphabet("BZELARI SCKDFGHMNOPTUWXY");
}

BazeleriesCode2::~BazeleriesCode2() = default;

/* ---- Configuration ---- */

void BazeleriesCode2::setParameters(int cols, int fracDepth)
{
    m_cols = qMax(2, cols);
    m_fracDepth = qMax(1, fracDepth);
}

void BazeleriesCode2::setKey(const QString& key)
{
    m_key = key.toUpper();
    m_alphabet = buildAlphabet(m_key);
}

/* ---- Build substitution alphabet from key ---- */

QString BazeleriesCode2::buildAlphabet(const QString& key) const
{
    QString alpha;
    QString upperKey = key.toUpper();
    // Remove duplicates from key, keep order
    for (const QChar& c : upperKey) {
        if (c.isLetter() && !alpha.contains(c))
            alpha.append(c);
    }
    // Fill remaining letters (standard 25-letter polybius, skip Q or J)
    const QString standard = "ABCDEFGHIKLMNOPQRSTUVWXYZ";
    for (const QChar& c : standard) {
        if (!alpha.contains(c))
            alpha.append(c);
    }
    return alpha;
}

/* ---- Substitute single character ---- */

QChar BazeleriesCode2::substitute(QChar c, bool invert) const
{
    if (!c.isLetter()) return c;
    QChar upper = c.toUpper();
    int idx = m_alphabet.indexOf(upper);
    if (idx < 0) return c;
    // Affine-like substitution: shift by key-derived offset
    int shift = (invert) ? -3 : 3;
    int newIdx = ((idx + shift) % 25 + 25) % 25;
    return m_alphabet[newIdx];
}

/* ---- Fractionate text into coordinate pairs ---- */

QVector<int> BazeleriesCode2::fractionate(const QString& text) const
{
    QVector<int> coords;
    int gridSize = qCeil(qSqrt(25.0));  // 5x5 grid
    for (int d = 0; d < m_fracDepth; ++d) {
        for (const QChar& c : text) {
            QChar upper = c.toUpper();
            int idx = m_alphabet.indexOf(upper);
            if (idx >= 0) {
                coords.append(idx / gridSize);  // Row
                coords.append(idx % gridSize);  // Column
            }
        }
    }
    return coords;
}

/* ---- Defractionate coordinates back to text ---- */

QString BazeleriesCode2::defractionate(const QVector<int>& coords) const
{
    QString result;
    int gridSize = qCeil(qSqrt(25.0));
    int totalPairs = coords.size() / (2 * m_fracDepth);
    for (int i = 0; i < totalPairs; ++i) {
        int row = 0, col = 0;
        for (int d = 0; d < m_fracDepth; ++d) {
            int base = (i * 2 * m_fracDepth) + d * 2 * totalPairs;
            if (base + 1 < coords.size()) {
                row += coords[base];
                col += coords[base + 1];
            }
        }
        row = (row / m_fracDepth) % gridSize;
        col = (col / m_fracDepth) % gridSize;
        int idx = row * gridSize + col;
        if (idx >= 0 && idx < m_alphabet.size())
            result.append(m_alphabet[idx]);
    }
    return result;
}

/* ---- Column transposition ---- */

QString BazeleriesCode2::columnTranspose(const QString& text, int rows, int cols) const
{
    QString result;
    // Read column by column
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            int idx = r * cols + c;
            if (idx < text.length())
                result.append(text[idx]);
        }
    }
    return result;
}

/* ---- Row transposition ---- */

QString BazeleriesCode2::rowTranspose(const QString& text, int rows, int cols) const
{
    QString result;
    // Reverse row order
    for (int r = rows - 1; r >= 0; --r) {
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            if (idx < text.length())
                result.append(text[idx]);
        }
    }
    return result;
}

/* ---- Inverse column transposition ---- */

QString BazeleriesCode2::inverseColumnTranspose(const QString& text, int rows, int cols) const
{
    QVector<QChar> result(text.length());
    int pos = 0;
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            int idx = r * cols + c;
            if (idx < result.size() && pos < text.length())
                result[idx] = text[pos++];
        }
    }
    QString out;
    for (const QChar& c : result) if (c.isNull()) break; else out.append(c);
    return out;
}

/* ---- Inverse row transposition ---- */

QString BazeleriesCode2::inverseRowTranspose(const QString& text, int rows, int cols) const
{
    QVector<QChar> result(text.length());
    int pos = 0;
    for (int r = rows - 1; r >= 0; --r) {
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            if (idx < result.size() && pos < text.length())
                result[idx] = text[pos++];
        }
    }
    QString out;
    for (const QChar& c : result) if (c.isNull()) break; else out.append(c);
    return out;
}

/* ---- Encrypt ---- */

QString BazeleriesCode2::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Substitute
    QString sub;
    for (const QChar& c : plaintext)
        sub.append(substitute(c, false));

    // Step 2: Pad to fill grid
    int rows = qCeil(static_cast<double>(sub.length()) / m_cols);
    while (sub.length() < rows * m_cols)
        sub.append('X');

    // Step 3: Column transposition (diffusion)
    QString col = columnTranspose(sub, rows, m_cols);

    // Step 4: Row transposition (diffusion)
    int rows2 = qCeil(static_cast<double>(col.length()) / m_cols);
    QString row = rowTranspose(col, rows2, m_cols);

    m_stats.inputLength = plaintext.length();
    m_stats.outputLength = row.length();
    m_stats.numRows = rows;
    m_stats.numCols = m_cols;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptionCompleted(m_stats.inputLength, m_stats.outputLength, timer.elapsed());
    return row;
}

/* ---- Decrypt ---- */

QString BazeleriesCode2::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    int rows2 = qCeil(static_cast<double>(ciphertext.length()) / m_cols);

    // Step 1: Inverse row transposition
    QString row = inverseRowTranspose(ciphertext, rows2, m_cols);

    // Step 2: Inverse column transposition
    int rows = qCeil(static_cast<double>(row.length()) / m_cols);
    QString col = inverseColumnTranspose(row, rows, m_cols);

    // Step 3: Inverse substitution
    QString result;
    for (const QChar& c : col)
        result.append(substitute(c, true));

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Reset ---- */

void BazeleriesCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
