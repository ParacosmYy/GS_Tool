/**
 * @file BazeleriesCode3.cpp
 * @brief BazeleriesCode3 实现
 *
 * 实现Bazeleries密码：嵌套分数网格与ADFGVX坐标编码行列密钥。
 */

#include "utils/code229/BazeleriesCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- ADFGVX coordinate labels ---- */

const QString BazeleriesCode3::COORD_LABELS = QStringLiteral("ADFGVX");

/* ---- Construction / Destruction ---- */

BazeleriesCode3::BazeleriesCode3(QObject *parent) : QObject(parent)
{
    m_alphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    m_gridSize = 6;
    m_grid.resize(m_gridSize);
    for (int i = 0; i < m_gridSize; ++i)
        m_grid[i].resize(m_gridSize);
    buildGrid();
}

BazeleriesCode3::~BazeleriesCode3() = default;

/* ---- Set keys ---- */

void BazeleriesCode3::setKeys(const QString& rowKey, const QString& colKey)
{
    m_rowKey = rowKey.toUpper();
    m_colKey = colKey.toUpper();
    m_rowPerm = keyPermutation(m_rowKey);
    m_colPerm = keyPermutation(m_colKey);
    buildGrid();
}

/* ---- Set alphabet ---- */

void BazeleriesCode3::setAlphabet(const QString& alphabet)
{
    m_alphabet = alphabet;
    buildGrid();
}

/* ---- Build fractionation grid ---- */

void BazeleriesCode3::buildGrid()
{
    // Fill grid row by row with alphabet characters
    QString chars = m_alphabet;
    // Remove duplicates and pad to 36
    QString unique;
    for (QChar c : chars) {
        if (!unique.contains(c))
            unique.append(c);
    }
    while (unique.length() < 36)
        unique.append(QChar('A' + (unique.length() % 26)));

    int idx = 0;
    for (int r = 0; r < m_gridSize; ++r) {
        for (int c = 0; c < m_gridSize; ++c) {
            m_grid[r][c] = unique[idx % unique.length()];
            idx++;
        }
    }
}

/* ---- Key permutation ---- */

QVector<int> BazeleriesCode3::keyPermutation(const QString& key) const
{
    int n = key.length();
    if (n == 0) return QVector<int>();

    QVector<QPair<QChar, int>> indexed;
    for (int i = 0; i < n; ++i)
        indexed.append(qMakePair(key[i], i));

    std::sort(indexed.begin(), indexed.end());
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i)
        perm[indexed[i].second] = i;
    return perm;
}

/* ---- Find in grid ---- */

bool BazeleriesCode3::findInGrid(QChar ch, int& row, int& col) const
{
    for (int r = 0; r < m_gridSize; ++r) {
        for (int c = 0; c < m_gridSize; ++c) {
            if (m_grid[r][c] == ch) {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

/* ---- Encode coordinate ---- */

QString BazeleriesCode3::encodeCoord(QChar ch) const
{
    int row, col;
    if (!findInGrid(ch.toUpper(), row, col))
        return QString();
    return QString(COORD_LABELS[row]) + COORD_LABELS[col];
}

/* ---- Decode coordinate ---- */

QChar BazeleriesCode3::decodeCoord(const QString& coord) const
{
    if (coord.length() != 2) return QChar();
    int row = COORD_LABELS.indexOf(coord[0]);
    int col = COORD_LABELS.indexOf(coord[1]);
    if (row < 0 || col < 0 || row >= m_gridSize || col >= m_gridSize)
        return QChar();
    return m_grid[row][col];
}

/* ---- Columnar transposition encrypt ---- */

QString BazeleriesCode3::columnarEncrypt(const QString& text) const
{
    int keyLen = m_colPerm.size();
    if (keyLen == 0) return text;

    int rows = qCeil(static_cast<double>(text.length()) / keyLen);
    QVector<QString> cols(keyLen);

    for (int i = 0; i < text.length(); ++i) {
        int col = i % keyLen;
        cols[col].append(text[i]);
    }

    // Read columns in permuted order
    QString result;
    for (int i = 0; i < keyLen; ++i) {
        int permCol = m_colPerm.indexOf(i);
        if (permCol >= 0 && permCol < keyLen)
            result += cols[permCol];
    }
    return result;
}

/* ---- Columnar transposition decrypt ---- */

QString BazeleriesCode3::columnarDecrypt(const QString& text, int originalRows) const
{
    int keyLen = m_colPerm.size();
    if (keyLen == 0) return text;

    int fullLen = originalRows * keyLen;
    int shortCols = (fullLen - text.length());

    QVector<QString> cols(keyLen);
    int pos = 0;
    for (int i = 0; i < keyLen; ++i) {
        int permCol = m_colPerm.indexOf(i);
        int colLen = originalRows - (permCol >= keyLen - shortCols ? 1 : 0);
        colLen = qMax(0, qMin(colLen, text.length() - pos));
        cols[permCol >= 0 ? permCol : i] = text.mid(pos, colLen);
        pos += colLen;
    }

    QString result;
    for (int r = 0; r < originalRows; ++r) {
        for (int c = 0; c < keyLen; ++c) {
            if (r < cols[c].length())
                result.append(cols[c][r]);
        }
    }
    return result;
}

/* ---- Encrypt ---- */

QString BazeleriesCode3::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_rowPerm.isEmpty() || m_colPerm.isEmpty()) {
        m_rowPerm = keyPermutation(QStringLiteral("KEY"));
        m_colPerm = keyPermutation(QStringLiteral("SECRET"));
    }

    // Step 1: Substitute each char to ADFGVX coordinates
    QString substituted;
    for (QChar ch : plaintext) {
        QString coord = encodeCoord(ch);
        if (!coord.isEmpty())
            substituted += coord;
    }

    // Step 2: Columnar transposition using column key
    QString result = columnarEncrypt(substituted);

    m_stats.inputLength = plaintext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptionCompleted(plaintext.length(), result.length(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString BazeleriesCode3::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_rowPerm.isEmpty() || m_colPerm.isEmpty()) {
        m_rowPerm = keyPermutation(QStringLiteral("KEY"));
        m_colPerm = keyPermutation(QStringLiteral("SECRET"));
    }

    int keyLen = m_colPerm.size();
    int originalPairs = ciphertext.length() / 2;
    int originalRows = qCeil(static_cast<double>(ciphertext.length()) / keyLen);

    // Step 1: Reverse columnar transposition
    QString detransposed = columnarDecrypt(ciphertext, originalRows);

    // Step 2: Decode ADFGVX coordinate pairs
    QString result;
    for (int i = 0; i + 1 < detransposed.length(); i += 2) {
        QString coord = detransposed.mid(i, 2);
        QChar ch = decodeCoord(coord);
        if (ch != QChar())
            result.append(ch);
    }

    m_stats.inputLength = ciphertext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptionCompleted(ciphertext.length(), result.length(), timer.elapsed());
    return result;
}

/* ---- Get grid ---- */

QVector<QString> BazeleriesCode3::grid() const
{
    QVector<QString> result(m_gridSize);
    for (int r = 0; r < m_gridSize; ++r) {
        QString row;
        for (int c = 0; c < m_gridSize; ++c)
            row.append(m_grid[r][c]);
        result[r] = row;
    }
    return result;
}

/* ---- Reset ---- */

void BazeleriesCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
