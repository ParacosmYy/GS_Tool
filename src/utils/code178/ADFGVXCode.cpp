/**
 * @file ADFGVXCode.cpp
 * @brief ADFGVXCode 实现
 *
 * 实现ADFGVX分馏密码：6×6 Polybius方阵替换、列置换加密/解密、已知明文攻击。
 */

#include "utils/code178/ADFGVXCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ADFGVXCode::ADFGVXCode(QObject *parent) : QObject(parent)
{
    m_polybiusKey = QStringLiteral("KEYABCDFGHIJLMNOPQRSTUVWXZ0123456789");
    m_transKey = QStringLiteral("ZEBRA");
    buildGrid();
}

ADFGVXCode::~ADFGVXCode() = default;

/* ---- Build 6x6 Polybius square ---- */

void ADFGVXCode::buildGrid()
{
    // Deduplicate key characters, fill with remaining A-Z + 0-9
    QString used;
    QString alphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    for (const QChar& ch : m_polybiusKey.toUpper()) {
        if (alphabet.contains(ch) && !used.contains(ch))
            used.append(ch);
    }
    for (const QChar& ch : alphabet) {
        if (!used.contains(ch)) used.append(ch);
    }
    // Pad to 36 if needed
    while (used.size() < 36) used.append('X');

    m_grid.resize(6);
    for (int r = 0; r < 6; ++r) {
        m_grid[r].resize(6);
        for (int c = 0; c < 6; ++c)
            m_grid[r][c] = used[r * 6 + c];
    }
}

/* ---- Configuration ---- */

void ADFGVXCode::setPolybiusKey(const QString& key)
{
    m_polybiusKey = key;
    buildGrid();
}

void ADFGVXCode::setTranspositionKey(const QString& key)
{
    m_transKey = key.toUpper();
}

/* ---- Compute column read order from key ---- */

QVector<int> ADFGVXCode::keyOrder(const QString& key) const
{
    int n = key.size();
    QVector<QPair<QChar, int>> indexed;
    for (int i = 0; i < n; ++i)
        indexed.append({key[i], i});

    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) {
                  if (a.first != b.first) return a.first < b.first;
                  return a.second < b.second;
              });

    QVector<int> order(n);
    for (int rank = 0; rank < n; ++rank)
        order[indexed[rank].second] = rank;
    return order;
}

/* ---- Columnar transposition encrypt ---- */

QString ADFGVXCode::columnarEncrypt(const QString& fractionated) const
{
    int keyLen = m_transKey.size();
    if (keyLen == 0) return fractionated;

    auto order = keyOrder(m_transKey);
    int n = fractionated.size();
    int rows = (n + keyLen - 1) / keyLen;

    // Write row by row, read column by column in key order
    QVector<QString> cols(keyLen);
    for (int i = 0; i < n; ++i) {
        int col = i % keyLen;
        cols[col].append(fractionated[i]);
    }

    // Read columns sorted by key order
    QVector<int> sortedCols(keyLen);
    for (int i = 0; i < keyLen; ++i) sortedCols[i] = i;
    std::sort(sortedCols.begin(), sortedCols.end(),
              [&](int a, int b) { return order[a] < order[b]; });

    QString result;
    for (int c : sortedCols)
        result.append(cols[c]);
    return result;
}

/* ---- Columnar transposition decrypt ---- */

QString ADFGVXCode::columnarDecrypt(const QString& ct, int keyLen) const
{
    if (keyLen == 0) return ct;
    int n = ct.size();
    int rows = (n + keyLen - 1) / keyLen;
    int extra = n % keyLen; // columns with extra char

    // Determine column lengths
    QVector<int> colLens(keyLen);
    for (int c = 0; c < keyLen; ++c)
        colLens[c] = rows - ((extra != 0 && c >= extra) ? 1 : 0);

    // Read order: columns sorted by key order
    QString tmpKey = m_transKey.left(keyLen);
    auto order = keyOrder(tmpKey);
    QVector<int> sortedCols(keyLen);
    for (int i = 0; i < keyLen; ++i) sortedCols[i] = i;
    std::sort(sortedCols.begin(), sortedCols.end(),
              [&](int a, int b) { return order[a] < order[b]; });

    // Extract columns
    QVector<QString> cols(keyLen);
    int pos = 0;
    for (int c : sortedCols) {
        int len = colLens[c];
        cols[c] = ct.mid(pos, len);
        pos += len;
    }

    // Read row by row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < keyLen; ++c)
            if (r < cols[c].size())
                result.append(cols[c][r]);
    return result;
}

/* ---- Encrypt ---- */

QString ADFGVXCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Polybius substitution
    QString fractionated;
    for (const QChar& ch : plaintext.toUpper()) {
        bool found = false;
        for (int r = 0; r < 6 && !found; ++r)
            for (int c = 0; c < 6 && !found; ++c)
                if (m_grid[r][c] == ch) {
                    fractionated.append(LABELS[r]);
                    fractionated.append(LABELS[c]);
                    found = true;
                }
        if (!found) { fractionated.append(LABELS[0]); fractionated.append(LABELS[0]); }
    }

    // Step 2: Columnar transposition
    QString ct = columnarEncrypt(fractionated);

    m_stats.totalEncryptions++;
    m_stats.transpositionKeyLen = m_transKey.size();
    m_timeSum += timer.elapsed();
    double total = m_stats.totalEncryptions + m_stats.totalDecryptions;
    m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encryptionCompleted(ct.size());
    return ct;
}

/* ---- Decrypt ---- */

QString ADFGVXCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Reverse columnar transposition
    int keyLen = m_transKey.size();
    QString fractionated = columnarDecrypt(ciphertext, keyLen);

    // Step 2: Reverse Polybius substitution
    QString plaintext;
    for (int i = 0; i + 1 < fractionated.size(); i += 2) {
        int row = -1, col = -1;
        for (int k = 0; k < 6; ++k) {
            if (fractionated[i] == LABELS[k]) row = k;
            if (fractionated[i + 1] == LABELS[k]) col = k;
        }
        if (row >= 0 && col >= 0 && row < 6 && col < 6)
            plaintext.append(m_grid[row][col]);
        else
            plaintext.append('?');
    }

    m_stats.totalDecryptions++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalEncryptions + m_stats.totalDecryptions;
    m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit decryptionCompleted(plaintext.size());
    return plaintext;
}

/* ---- Score a candidate decryption ---- */

double ADFGVXCode::scoreDecryption(const QString& candidate,
                                      const QString& known) const
{
    if (candidate.size() < known.size()) return 0.0;
    int matches = 0;
    for (int i = 0; i <= candidate.size() - known.size(); ++i) {
        int m = 0;
        for (int j = 0; j < known.size(); ++j)
            if (candidate[i + j] == known[j]) ++m;
        if (m == known.size()) { matches++; break; } // Exact substring match
    }
    return static_cast<double>(matches);
}

/* ---- Known plaintext attack ---- */

QVector<QString> ADFGVXCode::knownPlaintextAttack(const QString& ciphertext,
                                                     const QString& knownPt) const
{
    QVector<QString> candidates;
    int maxKeyLen = qMin(10, ciphertext.size() / 2);

    // Try all key lengths
    for (int keyLen = 2; keyLen <= maxKeyLen; ++keyLen) {
        // For each key length, try to find the transposition key order
        // that places known plaintext at expected positions
        int n = ciphertext.size();
        int rows = (n + keyLen - 1) / keyLen;

        // Generate all permutations of key order (limited for small keys)
        if (keyLen > 7) continue; // Skip very large key lengths
        QVector<int> perm(keyLen);
        for (int i = 0; i < keyLen; ++i) perm[i] = i;

        do {
            // Reconstruct column order from permutation
            QVector<int> order(keyLen);
            for (int i = 0; i < keyLen; ++i) order[i] = perm[i];

            // Decrypt with this order
            QString tmpKey;
            for (int i = 0; i < keyLen; ++i) tmpKey.append(QChar('A' + order[i]));

            QVector<int> sortedCols(keyLen);
            for (int i = 0; i < keyLen; ++i) sortedCols[i] = i;
            std::sort(sortedCols.begin(), sortedCols.end(),
                      [&](int a, int b) { return order[a] < order[b]; });

            QVector<int> colLens(keyLen);
            int extra = n % keyLen;
            for (int c = 0; c < keyLen; ++c)
                colLens[c] = rows - ((extra != 0 && c >= extra) ? 1 : 0);

            QVector<QString> cols(keyLen);
            int pos = 0;
            for (int c : sortedCols) {
                cols[c] = ciphertext.mid(pos, colLens[c]);
                pos += colLens[c];
            }

            QString fractionated;
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < keyLen; ++c)
                    if (r < cols[c].size())
                        fractionated.append(cols[c][r]);

            // Reverse Polybius
            QString pt;
            for (int i = 0; i + 1 < fractionated.size(); i += 2) {
                int row = -1, col = -1;
                for (int k = 0; k < 6; ++k) {
                    if (fractionated[i] == LABELS[k]) row = k;
                    if (fractionated[i + 1] == LABELS[k]) col = k;
                }
                if (row >= 0 && col >= 0) pt.append(m_grid[row][col]);
            }

            if (scoreDecryption(pt, knownPt) > 0)
                candidates.append(tmpKey);

        } while (std::next_permutation(perm.begin(), perm.end()));
    }
    return candidates;
}

/* ---- Brute force key search ---- */

QPair<QString, QString> ADFGVXCode::bruteForceKey(const QString& ciphertext,
                                                     const QString& knownPt,
                                                     int maxKeyLen) const
{
    // Try each key length with simple alphabet permutations
    for (int keyLen = 2; keyLen <= maxKeyLen; ++keyLen) {
        QVector<int> perm(keyLen);
        for (int i = 0; i < keyLen; ++i) perm[i] = i;

        do {
            QString tmpKey;
            for (int i = 0; i < keyLen; ++i) tmpKey.append(QChar('A' + perm[i]));

            // Quick decrypt and check
            int n = ciphertext.size();
            int rows = (n + keyLen - 1) / keyLen;
            QVector<int> colLens(keyLen);
            int extra = n % keyLen;
            for (int c = 0; c < keyLen; ++c)
                colLens[c] = rows - ((extra != 0 && c >= extra) ? 1 : 0);

            QVector<int> sortedCols(keyLen);
            for (int i = 0; i < keyLen; ++i) sortedCols[i] = i;
            std::sort(sortedCols.begin(), sortedCols.end(),
                      [&](int a, int b) { return perm[a] < perm[b]; });

            QVector<QString> cols(keyLen);
            int pos = 0;
            for (int c : sortedCols) {
                cols[c] = ciphertext.mid(pos, colLens[c]);
                pos += colLens[c];
            }

            QString frac;
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < keyLen; ++c)
                    if (r < cols[c].size()) frac.append(cols[c][r]);

            QString pt;
            for (int i = 0; i + 1 < frac.size(); i += 2) {
                int row = -1, col = -1;
                for (int k = 0; k < 6; ++k) {
                    if (frac[i] == LABELS[k]) row = k;
                    if (frac[i + 1] == LABELS[k]) col = k;
                }
                if (row >= 0 && col >= 0) pt.append(m_grid[row][col]);
            }

            if (pt.contains(knownPt))
                return {m_polybiusKey, tmpKey};

        } while (std::next_permutation(perm.begin(), perm.end()));
    }
    return {{}, {}};
}

/* ---- Reset ---- */

void ADFGVXCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
