/**
 * @file DigrafidCode8.cpp
 * @brief DigrafidCode8 实现
 *
 * 实现二合字密码：二合字到列映射与双分坐标网格分数替换实现多图加密。
 */

#include "utils/code296/DigrafidCode8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode8::DigrafidCode8(QObject *parent)
    : QObject(parent)
{
    buildGrid();
}

DigrafidCode8::~DigrafidCode8() = default;

/* ---- Configuration ---- */

void DigrafidCode8::setKey(const QString& key)
{
    // Build a 9-char key from unique uppercase letters, pad with remaining
    QString cleaned;
    for (QChar c : key.toUpper()) {
        if (c.isLetter() && !cleaned.contains(c) && cleaned.size() < 9)
            cleaned.append(c);
    }
    // Fill remaining slots with unused letters
    for (QChar c = u'A'; c <= u'Z' && cleaned.size() < 9; c = c.unicode() + 1) {
        if (!cleaned.contains(c))
            cleaned.append(c);
    }
    m_key = cleaned.left(9);
    buildGrid();
}

void DigrafidCode8::setColumnKey(const QString& colKey)
{
    QString cleaned;
    for (QChar c : colKey.toUpper()) {
        if (c.isLetter() && !cleaned.contains(c))
            cleaned.append(c);
    }
    m_columnKey = cleaned.isEmpty() ? QStringLiteral("KEY") : cleaned;
}

/* ---- Build 3x3 substitution grid ---- */

void DigrafidCode8::buildGrid()
{
    m_grid.resize(3);
    for (int i = 0; i < 3; ++i) {
        m_grid[i].resize(3);
        for (int j = 0; j < 3; ++j) {
            int idx = i * 3 + j;
            m_grid[i][j] = (idx < m_key.size()) ? m_key[idx] : QChar(u'?');
        }
    }
}

/* ---- Find character position in grid ---- */

QPair<int, int> DigrafidCode8::findInGrid(QChar c) const
{
    c = c.toUpper();
    for (int r = 0; r < 3; ++r) {
        for (int col = 0; col < 3; ++col) {
            if (m_grid[r][col] == c)
                return {r, col};
        }
    }
    return {-1, -1};
}

/* ---- Column key permutation ---- */

QVector<int> DigrafidCode8::columnPermutation() const
{
    int n = m_columnKey.size();
    QVector<QPair<int, int>> indexed;
    for (int i = 0; i < n; ++i)
        indexed.append({static_cast<int>(m_columnKey[i].unicode()), i});
    std::sort(indexed.begin(), indexed.end());

    QVector<int> perm(n);
    for (int i = 0; i < n; ++i)
        perm[indexed[i].second] = i;
    return perm;
}

/* ---- Prepare text ---- */

QString DigrafidCode8::prepareText(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (c.isLetter())
            result.append(c);
    }
    // Pad to even length for digraph pairing
    if (result.size() % 2 != 0)
        result.append(u'X');
    return result;
}

/* ---- Encrypt ---- */

DigrafidCode8::EncryptResult DigrafidCode8::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    EncryptResult result;
    QString prepared = prepareText(plaintext);
    int n = prepared.size();
    result.textLength = n;
    result.numDigraphs = n / 2;

    // Collect fractional coordinates: row(1st char), col(1st char), row(2nd char)...
    QVector<int> allCoords;
    for (int i = 0; i < n; i += 2) {
        auto pos1 = findInGrid(prepared[i]);
        auto pos2 = findInGrid(prepared[i + 1]);
        if (pos1.first < 0 || pos2.first < 0) continue;
        allCoords.append(pos1.first);
        allCoords.append(pos1.second);
        allCoords.append(pos2.first);
        allCoords.append(pos2.second);
    }

    // Apply column transposition on groups of 3
    auto perm = columnPermutation();
    int numGroups = allCoords.size() / 3;
    QString ciphertext;

    for (int g = 0; g < numGroups; ++g) {
        int base = g * 3;
        QVector<int> group = {allCoords[base], allCoords[base + 1], allCoords[base + 2]};

        // Transpose within group using column key order
        QVector<int> reordered(3);
        for (int i = 0; i < 3 && i < perm.size(); ++i)
            reordered[perm[i % perm.size()]] = group[i];

        // Map reordered fractional coords back to grid characters
        int r1 = reordered[0], c1 = reordered[1], r2 = reordered[2];
        if (r1 >= 0 && r1 < 3 && c1 >= 0 && c1 < 3)
            ciphertext.append(m_grid[r1][c1]);
        if (r2 >= 0 && r2 < 3)
            ciphertext.append(m_grid[r2][c1]);
    }

    result.ciphertext = ciphertext;

    double elapsed = timer.elapsed();
    m_stats.totalEncrypts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncrypts + m_stats.totalDecrypts);

    emit encryptDone(result.ciphertext.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

DigrafidCode8::DecryptResult DigrafidCode8::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    DecryptResult result;
    QString prepared = prepareText(ciphertext);
    int n = prepared.size();
    result.textLength = n;
    result.numDigraphs = n / 2;

    auto perm = columnPermutation();
    QString plaintext;

    // Reverse the fractional coordinate mapping
    for (int i = 0; i + 1 < n; i += 2) {
        auto pos1 = findInGrid(prepared[i]);
        auto pos2 = findInGrid(prepared[i + 1]);

        if (pos1.first < 0 || pos2.first < 0) continue;

        // Inverse transposition
        QVector<int> encoded = {pos1.first, pos1.second, pos2.first};
        QVector<int> original(3);
        for (int j = 0; j < 3 && j < perm.size(); ++j)
            original[j] = encoded[perm[j % perm.size()]];

        // Reconstruct original digraph coordinates
        int r1 = original[0], c1 = original[1];
        if (r1 >= 0 && r1 < 3 && c1 >= 0 && c1 < 3)
            plaintext.append(m_grid[r1][c1]);
        int r2 = original[2];
        if (r2 >= 0 && r2 < 3 && c1 >= 0 && c1 < 3)
            plaintext.append(m_grid[r2][c1]);
    }

    result.plaintext = plaintext;

    double elapsed = timer.elapsed();
    m_stats.totalDecrypts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncrypts + m_stats.totalDecrypts);

    emit decryptDone(result.plaintext.size(), elapsed);
    return result;
}

/* ---- Reset ---- */

void DigrafidCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
