/**
 * @file DigrafidCode7.cpp
 * @brief DigrafidCode7 实现
 *
 * 实现二图字密码：扩展二图矩阵与列键分数编码的多表替代。
 */

#include "utils/code282/DigrafidCode7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

DigrafidCode7::DigrafidCode7(QObject *parent)
    : QObject(parent)
{
    m_charToIdx.resize(128, -1);
    rebuildMatrix();
}

DigrafidCode7::~DigrafidCode7() = default;

/* ---- Configuration ---- */

void DigrafidCode7::setKey(const QString& key) { m_key = key.toUpper(); rebuildMatrix(); }
void DigrafidCode7::setPeriod(int period) { m_period = qBound(1, period, 50); }

/* ---- Character mapping ---- */

int DigrafidCode7::charIndex(QChar c) const
{
    char ch = c.toUpper().toLatin1();
    if (ch >= 'A' && ch <= 'Z') {
        if (ch == 'J') ch = 'I';  // Classic: J merged with I
        return (ch <= 'I') ? (ch - 'A') : (ch - 'A' - 1);
    }
    if (ch == '#') return 26;
    return -1;
}

QChar DigrafidCode7::indexChar(int idx) const
{
    if (idx < 0 || idx > 26) return '?';
    if (idx == 26) return '#';
    // 0..8=A..I, 9..25=K..Z
    if (idx <= 8) return QChar('A' + idx);
    return QChar('A' + idx + 1);  // Skip J
}

/* ---- Prepare text ---- */

QString DigrafidCode7::prepare(const QString& text) const
{
    QString out;
    for (QChar c : text.toUpper()) {
        if (c.isLetter()) {
            char ch = c.toLatin1();
            if (ch == 'J') ch = 'I';
            out.append(QChar(ch));
        }
    }
    // Pad with 'X' to even length
    if (out.size() % 2 != 0) out.append('X');
    return out;
}

/* ---- Rebuild 9x9 digraph matrix ---- */

void DigrafidCode7::rebuildMatrix()
{
    // Build key alphabet: unique chars from key, then remaining A-Z(merged J/I) + #
    QString used;
    QString alphabet;
    for (QChar c : m_key) {
        char ch = c.toUpper().toLatin1();
        if (ch == 'J') ch = 'I';
        if (!used.contains(QChar(ch))) {
            used.append(QChar(ch));
            alphabet.append(QChar(ch));
        }
    }
    // Fill remaining
    for (char ch = 'A'; ch <= 'Z'; ++ch) {
        if (ch == 'J') continue;
        if (!used.contains(QChar(ch))) alphabet.append(QChar(ch));
    }
    if (!used.contains('#')) alphabet.append('#');

    // Build char-to-index mapping
    m_charToIdx.fill(-1);
    for (int i = 0; i < alphabet.size() && i < 27; ++i) {
        char ch = alphabet[i].toLatin1();
        if (ch >= 'A' && ch <= 'Z') m_charToIdx[ch] = i;
        else if (ch == '#') m_charToIdx['#'] = 26;
    }
    if (m_charToIdx['J'] < 0) m_charToIdx['J'] = m_charToIdx['I'];

    // Build 9x9 matrix: m_matrix[row][col] = cipher index
    // Column-keyed fractional: column order derived from key
    m_matrix.resize(9);
    for (int r = 0; r < 9; ++r) {
        m_matrix[r].resize(9);
        for (int c = 0; c < 9; ++c) {
            // Fractional encoding: row provides upper digit, col provides lower
            m_matrix[r][c] = r * 9 + c;
        }
    }

    // Apply column permutation from key
    QVector<int> colOrder(9);
    for (int i = 0; i < 9; ++i) colOrder[i] = i;
    // Derive column order from key characters
    for (int i = 0; i < qMin(m_key.size(), 9); ++i) {
        int col = m_key[i].toLatin1() % 9;
        if (i < 9) std::swap(colOrder[i], colOrder[col]);
    }

    // Apply permutation to matrix
    QVector<QVector<int>> permuted(9);
    for (int r = 0; r < 9; ++r) {
        permuted[r].resize(9);
        for (int c = 0; c < 9; ++c)
            permuted[r][c] = m_matrix[r][colOrder[c]];
    }
    m_matrix = permuted;
}

/* ---- Fractional encoding ---- */

int DigrafidCode7::fractionate(int row, int col) const
{
    if (row < 0 || row >= 9 || col < 0 || col >= 9) return 0;
    return m_matrix[row][col];
}

QPair<int, int> DigrafidCode7::defractionate(int cipher) const
{
    // Reverse lookup in matrix
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (m_matrix[r][c] == cipher)
                return {r, c};
        }
    }
    return {0, 0};
}

/* ---- Encode ---- */

DigrafidCode7::CipherResult DigrafidCode7::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString prep = prepare(plaintext);
    result.numDigraphs = prep.size() / 2;
    result.periodUsed = m_period;

    QString cipher;
    for (int i = 0; i < prep.size(); i += 2) {
        int a = charIndex(prep[i]);
        int b = charIndex(prep[i + 1]);
        if (a < 0) a = 0;
        if (b < 0) b = 0;

        // Map to 9x9 grid: row = a/3, col = b/3 for extended encoding
        int row = a % 9;
        int col = b % 9;
        int enc = fractionate(row, col);

        // Apply period-based transposition
        int periodOffset = (i / 2) % m_period;
        enc = (enc + periodOffset) % 81;

        // Output as two characters: upper and lower digit in base-9
        int hi = enc / 9;
        int lo = enc % 9;
        cipher.append(indexChar(hi * 3));
        cipher.append(indexChar(lo * 3));
    }

    result.text = cipher;
    double elapsed = timer.elapsed();
    m_stats.numEncoded++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodeDone(result.numDigraphs, m_period, elapsed);

    return result;
}

/* ---- Decode ---- */

DigrafidCode7::CipherResult DigrafidCode7::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString prep = ciphertext.toUpper();
    result.numDigraphs = prep.size() / 2;
    result.periodUsed = m_period;

    QString plain;
    for (int i = 0; i + 3 < prep.size(); i += 2) {
        int a = charIndex(prep[i]);
        int b = charIndex(prep[i + 1]);
        if (a < 0) a = 0;
        if (b < 0) b = 0;

        int hi = a / 3;
        int lo = b / 3;
        int enc = hi * 9 + lo;

        // Reverse period transposition
        int periodOffset = (i / 2) % m_period;
        enc = (enc - periodOffset + 81) % 81;

        auto [row, col] = defractionate(enc);
        plain.append(indexChar(row));
        plain.append(indexChar(col));
    }

    result.text = plain;
    double elapsed = timer.elapsed();
    m_stats.numDecoded++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodeDone(result.numDigraphs, m_period, elapsed);

    return result;
}

/* ---- Get matrix ---- */

QVector<QVector<int>> DigrafidCode7::digraphMatrix() const { return m_matrix; }

/* ---- Reset ---- */

void DigrafidCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
