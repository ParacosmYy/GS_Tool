/**
 * @file MorbitCode6.cpp
 * @brief MorbitCode6 实现
 *
 * 实现Morbit密码：3x3网格符号映射与单字母密钥驱动置换紧凑视觉编码。
 */

#include "utils/code269/MorbitCode6.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MorbitCode6::MorbitCode6(QObject *parent)
    : QObject(parent)
{
    setKey(QStringLiteral("987654321"));
}

MorbitCode6::~MorbitCode6() = default;

/* ---- Configuration ---- */

void MorbitCode6::setKey(const QString& key)
{
    if (isValidKey(key)) {
        m_key = key;
        buildPermutation();
    }
}

void MorbitCode6::setGridSymbols(const QString& symbols)
{
    if (symbols.length() == 9) {
        m_gridSymbols = symbols;
        if (!m_key.isEmpty()) buildPermutation();
    }
}

/* ---- Key validation ---- */

bool MorbitCode6::isValidKey(const QString& key) const
{
    if (key.length() != 9) return false;

    // Must contain digits 1-9 exactly once
    QVector<bool> seen(9, false);
    for (int i = 0; i < 9; ++i) {
        QChar ch = key[i];
        if (ch < '1' || ch > '9') return false;
        int idx = ch.digitValue() - 1;
        if (seen[idx]) return false;
        seen[idx] = true;
    }
    return true;
}

/* ---- Build permutation from key ---- */

void MorbitCode6::buildPermutation()
{
    // Forward permutation: key position i -> digit value at that position
    // The key defines how the 9 grid positions are rearranged
    m_forward.resize(9);
    m_inverse.resize(9);

    for (int i = 0; i < 9; ++i) {
        int val = m_key[i].digitValue() - 1;
        m_forward[i] = val;
        m_inverse[val] = i;
    }
}

/* ---- Map digit pair through grid ---- */

QString MorbitCode6::mapPair(int r, int c, bool enc) const
{
    // Convert 2D grid coords to 1D index
    int idx = r * 3 + c;
    if (idx < 0 || idx >= 9) return QString();

    const QVector<int>& perm = enc ? m_forward : m_inverse;
    int mapped = perm[idx];

    // Return the symbol at the mapped position
    return QString(m_gridSymbols[mapped]);
}

/* ---- Encode ---- */

QString MorbitCode6::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    // Morbit encoding: each character maps to a pair of grid coordinates
    // The key permutation rearranges the grid positions
    QString result;
    QString upper = plaintext.toUpper();

    for (int i = 0; i < upper.length(); ++i) {
        QChar ch = upper[i];

        // Map character to grid position (A-I -> grid 0-8)
        int charVal = -1;
        if (ch >= 'A' && ch <= 'I') {
            charVal = ch.unicode() - 'A'; // 0-8
        } else if (ch >= 'J' && ch <= 'Z') {
            // J=9 maps to grid via modulo
            charVal = (ch.unicode() - 'A') % 9;
        } else {
            // Non-alpha: pass through
            result.append(ch);
            continue;
        }

        // Apply forward permutation
        int mapped = m_forward[charVal];

        // Convert to row/col in 3x3 grid
        int row = mapped / 3;
        int col = mapped % 3;

        // Output as symbol pair
        result.append(m_gridSymbols[row * 3 + col]);
        result.append(m_gridSymbols[mapped]);
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    const_cast<MorbitCode6*>(this)->codingCompleted(
        m_stats.inputLength, m_stats.outputLength, elapsed);

    return result;
}

/* ---- Decode ---- */

QString MorbitCode6::decode(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    int len = ciphertext.length();
    int i = 0;

    while (i < len) {
        QChar ch1 = ciphertext[i];

        // Check if it's a grid symbol
        int symIdx = m_gridSymbols.indexOf(ch1);
        if (symIdx < 0 || i + 1 >= len) {
            result.append(ch1);
            ++i;
            continue;
        }

        QChar ch2 = ciphertext[i + 1];
        int symIdx2 = m_gridSymbols.indexOf(ch2);
        if (symIdx2 < 0) {
            result.append(ch1);
            ++i;
            continue;
        }

        // Use the second symbol as the mapped position
        int mappedPos = symIdx2;

        // Apply inverse permutation to recover original grid position
        int origPos = m_inverse[mappedPos];

        // Map grid position back to character
        QChar decoded = QChar('A' + origPos);
        result.append(decoded);
        i += 2;
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    const_cast<MorbitCode6*>(this)->codingCompleted(
        m_stats.inputLength, m_stats.outputLength, elapsed);

    return result;
}

/* ---- Accessors ---- */

QVector<int> MorbitCode6::permutationMap() const { return m_forward; }
QVector<int> MorbitCode6::inverseMap() const { return m_inverse; }

/* ---- Reset ---- */

void MorbitCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
