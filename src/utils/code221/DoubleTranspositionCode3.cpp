/**
 * @file DoubleTranspositionCode3.cpp
 * @brief DoubleTranspositionCode3 实现
 *
 * 实现双重置换密码：字谜搜索、杆相交模式分析、密钥恢复。
 */

#include "utils/code221/DoubleTranspositionCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DoubleTranspositionCode3::DoubleTranspositionCode3(QObject *parent) : QObject(parent) {}
DoubleTranspositionCode3::~DoubleTranspositionCode3() = default;

/* ---- Set keys ---- */

void DoubleTranspositionCode3::setKeys(const QVector<int>& columnKey,
                                          const QVector<int>& rowKey)
{
    m_columnKey = columnKey;
    m_rowKey = rowKey;
    m_cols = columnKey.size();
    m_rows = rowKey.size();
}

/* ---- Single transposition ---- */

QString DoubleTranspositionCode3::singleTranspose(const QString& text,
    const QVector<int>& key, int cols, bool encrypt) const
{
    int len = text.length();
    int totalCells = cols * ((len + cols - 1) / cols);
    QString padded = text;
    padded.resize(totalCells, 'X');

    int rows = totalCells / cols;
    QString result;
    result.resize(totalCells, 'X');

    if (encrypt) {
        // Read by key order columns
        for (int k = 0; k < cols; ++k) {
            int col = (k < key.size()) ? key[k] : k;
            for (int r = 0; r < rows; ++r)
                result[k * rows + r] = padded[r * cols + col];
        }
    } else {
        // Inverse: write by key order columns
        for (int k = 0; k < cols; ++k) {
            int col = (k < key.size()) ? key[k] : k;
            for (int r = 0; r < rows; ++r)
                result[r * cols + col] = padded[k * rows + r];
        }
    }
    return result;
}

/* ---- Validate key ---- */

bool DoubleTranspositionCode3::validateKey(const QVector<int>& key) const
{
    int n = key.size();
    QVector<bool> seen(n, false);
    for (int v : key) {
        if (v < 0 || v >= n || seen[v]) return false;
        seen[v] = true;
    }
    return true;
}

/* ---- Encrypt ---- */

QString DoubleTranspositionCode3::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_columnKey.isEmpty() || m_rowKey.isEmpty()) return plaintext;

    // First transposition by columns
    QString first = singleTranspose(plaintext, m_columnKey, m_cols, true);
    // Second transposition by rows
    int r = first.length() / m_cols;
    if (r < 1) r = 1;
    QString second = singleTranspose(first, m_rowKey, m_rows, true);

    const_cast<DoubleTranspositionCode3*>(this)->m_stats.totalOps++;
    const_cast<DoubleTranspositionCode3*>(this)->m_timeSum += timer.elapsed();
    const_cast<DoubleTranspositionCode3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<DoubleTranspositionCode3*>(this)->encryptionCompleted(
        plaintext.length(), timer.elapsed());
    return second;
}

/* ---- Decrypt ---- */

QString DoubleTranspositionCode3::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_columnKey.isEmpty() || m_rowKey.isEmpty()) return ciphertext;

    // Reverse second transposition
    QString first = singleTranspose(ciphertext, m_rowKey, m_rows, false);
    // Reverse first transposition
    QString result = singleTranspose(first, m_columnKey, m_cols, false);

    const_cast<DoubleTranspositionCode3*>(this)->m_stats.totalOps++;
    const_cast<DoubleTranspositionCode3*>(this)->m_timeSum += timer.elapsed();
    const_cast<DoubleTranspositionCode3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Build rods ---- */

QVector<QString> DoubleTranspositionCode3::buildRods(const QString& ciphertext,
                                                       int cols) const
{
    int len = ciphertext.length();
    int rows = (len + cols - 1) / cols;
    QVector<QString> rods(cols);
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            int idx = r * cols + c;
            if (idx < len) rods[c] += ciphertext[idx];
        }
    }
    return rods;
}

/* ---- Rod intersection analysis ---- */

QVector<QVector<int>> DoubleTranspositionCode3::rodIntersectionAnalysis(
    const QString& ciphertext, int cols) const
{
    QVector<QString> rods = buildRods(ciphertext, cols);
    int n = rods.size();

    // Build intersection matrix: count of matching characters at same rod position
    QVector<QVector<int>> intersections(n, QVector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int matches = 0;
            int minLen = qMin(rods[i].length(), rods[j].length());
            for (int k = 0; k < minLen; ++k) {
                if (rods[i][k] == rods[j][k]) matches++;
            }
            intersections[i][j] = matches;
            intersections[j][i] = matches;
        }
    }
    return intersections;
}

/* ---- Bigram score ---- */

double DoubleTranspositionCode3::bigramScore(const QString& text) const
{
    if (text.length() < 2) return 0.0;
    double score = 0.0;
    QString upper = text.toUpper();
    // Common English bigrams
    static const QSet<QString> common = {
        "TH","HE","IN","ER","AN","RE","ON","AT","EN","ND",
        "TI","ES","OR","TE","OF","ED","IS","IT","AL","AR"
    };
    for (int i = 0; i < upper.length() - 1; ++i) {
        QString bigram = upper.mid(i, 2);
        if (common.contains(bigram)) score += 1.0;
    }
    return score / (upper.length() - 1);
}

/* ---- Score plaintext ---- */

double DoubleTranspositionCode3::scorePlaintext(const QString& text) const
{
    return bigramScore(text);
}

/* ---- Anagramming search ---- */

DoubleTranspositionCode3::AnalysisResult DoubleTranspositionCode3::anagrammingSearch(
    const QString& ciphertext, const QString& crib) const
{
    QElapsedTimer timer;
    timer.start();
    AnalysisResult best;
    best.score = -1.0;

    int cribLen = crib.length();
    int ctLen = ciphertext.length();
    if (cribLen > ctLen || crib.isEmpty()) return best;

    // Try different column counts
    for (int cols = 2; cols <= qMin(20, ctLen); ++cols) {
        int rows = (ctLen + cols - 1) / cols;
        QVector<int> colKey(cols);
        for (int i = 0; i < cols; ++i) colKey[i] = i;

        // Try permutations (limited for tractability)
        int maxPerm = 1;
        for (int i = 2; i <= cols && maxPerm < 500; ++i) maxPerm *= i;

        for (int trial = 0; trial < qMin(maxPerm, 500); ++trial) {
            // Generate a permutation
            QVector<int> key = colKey;
            // Fisher-Yates shuffle
            for (int i = key.size() - 1; i > 0; --i) {
                int j = qrand() % (i + 1);
                std::swap(key[i], key[j]);
            }

            QString decrypted = singleTranspose(ciphertext, key, cols, false);
            double score = scorePlaintext(decrypted);

            // Check if crib appears
            if (decrypted.contains(crib, Qt::CaseInsensitive)) score += 10.0;

            if (score > best.score) {
                best.columnKey = key;
                best.plaintext = decrypted;
                best.score = score;
                best.valid = true;
            }
        }
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit analysisCompleted(best.score, timer.elapsed());
    return best;
}

/* ---- Generate permutations (limited) ---- */

void DoubleTranspositionCode3::generatePermutations(int n, int maxCount,
    QVector<QVector<int>>& perms) const
{
    QVector<int> base(n);
    for (int i = 0; i < n; ++i) base[i] = i;
    perms.append(base);
    for (int count = 1; count < maxCount; ++count) {
        QVector<int> p = base;
        for (int i = n - 1; i > 0; --i) {
            int j = qrand() % (i + 1);
            std::swap(p[i], p[j]);
        }
        perms.append(p);
    }
}

/* ---- Reset ---- */

void DoubleTranspositionCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_columnKey.clear();
    m_rowKey.clear();
    m_cols = 0;
    m_rows = 0;
}
