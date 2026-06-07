/**
 * @file ColumnarTransposition.cpp
 * @brief ColumnarTransposition 实现
 *
 * 实现列置换密码：关键词列排序、爬山法密钥恢复、四元组频率分析。
 */

#include "utils/code203/ColumnarTransposition.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ColumnarTransposition::ColumnarTransposition(QObject *parent) : QObject(parent) { initQuadgrams(); }
ColumnarTransposition::~ColumnarTransposition() = default;

/* ---- Column order from keyword ---- */

QVector<int> ColumnarTransposition::columnOrder(const QString& keyword) const
{
    int n = keyword.size();
    // Stable alphabetical sort to determine column read order
    QVector<QPair<QChar, int>> indexed;
    indexed.reserve(n);
    for (int i = 0; i < n; ++i) indexed.append({keyword[i].toLower(), i});

    std::stable_sort(indexed.begin(), indexed.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[indexed[i].second] = i;
    return order;
}

/* ---- Encrypt ---- */

QString ColumnarTransposition::encrypt(const QString& plaintext, const QString& keyword) const
{
    QElapsedTimer timer;
    timer.start();

    int keyLen = keyword.size();
    if (keyLen == 0) return plaintext;

    QVector<int> order = columnOrder(keyword);
    return applyColumnPermutation(plaintext, order, true);

    Q_UNUSED(timer)
}

/* ---- Decrypt ---- */

QString ColumnarTransposition::decrypt(const QString& ciphertext, const QString& keyword) const
{
    QElapsedTimer timer;
    timer.start();

    int keyLen = keyword.size();
    if (keyLen == 0) return ciphertext;

    QVector<int> order = columnOrder(keyword);
    return applyColumnPermutation(ciphertext, order, false);

    Q_UNUSED(timer)
}

/* ---- Apply column permutation ---- */

QString ColumnarTransposition::applyColumnPermutation(const QString& text, const QVector<int>& order, bool enc) const
{
    int keyLen = order.size();
    int textLen = text.size();
    int rows = qCeil(static_cast<double>(textLen) / keyLen);

    // Build grid
    QVector<QString> grid(rows);
    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < keyLen; ++c) {
            if (idx < textLen) grid[r] += text[idx++];
            else grid[r] += QLatin1Char('X'); // padding
        }
    }

    // Read columns in order
    QString result;
    if (enc) {
        // Encryption: read by sorted column order
        QVector<int> sortedCols(keyLen);
        for (int i = 0; i < keyLen; ++i) sortedCols[order[i]] = i;
        for (int c = 0; c < keyLen; ++c) {
            for (int r = 0; r < rows; ++r)
                result += grid[r][sortedCols[c]];
        }
    } else {
        // Decryption: distribute ciphertext to columns then read row-wise
        int fullCols = textLen % keyLen;
        if (fullCols == 0) fullCols = keyLen;
        QVector<int> colLengths(keyLen, rows);
        for (int c = fullCols; c < keyLen; ++c) colLengths[c] = rows - 1;

        QVector<QString> columns(keyLen);
        int pos = 0;
        // Read by permutation order
        QVector<int> sortedCols(keyLen);
        for (int i = 0; i < keyLen; ++i) sortedCols[order[i]] = i;
        for (int c = 0; c < keyLen; ++c) {
            int realCol = sortedCols[c];
            for (int r = 0; r < colLengths[realCol]; ++r)
                columns[realCol] += text[pos++];
        }
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < keyLen; ++c)
                if (r < columns[c].size()) result += columns[c][r];
    }
    return result;
}

/* ---- Quadgram initialization ---- */

void ColumnarTransposition::initQuadgrams()
{
    // Sample English quadgram log-probabilities (representative subset)
    m_quadgrams = {
        {"TION", -1.50}, {"THEA", -1.80}, {"ATIO", -1.65}, {"ENTH", -2.10},
        {"ERIN", -2.30}, {"ANDT", -2.15}, {"INGT", -2.00}, {"MENT", -1.75},
        {"THES", -2.25}, {"WITH", -2.05}, {"TION", -1.50}, {"FORT", -2.60},
        {"THAT", -2.40}, {"FROM", -2.50}, {"THIS", -2.55}, {"HAVE", -2.70},
        {"THEY", -2.80}, {"BEEN", -2.65}, {"WILL", -2.75}, {"NOTH", -2.90},
        {"BUTT", -3.10}, {"HERS", -2.95}, {"SHET", -3.00}, {"WHEN", -2.85},
        {"WHIC", -2.70}, {"THEI", -2.60}, {"WOUT", -3.20}, {"COUL", -3.05},
        {"OTHE", -2.50}, {"CONT", -2.75}, {"RECO", -3.15}, {"MAKT", -3.30}
    };
}

/* ---- Quadgram score ---- */

double ColumnarTransposition::quadgramScore(const QString& text) const
{
    double score = 0.0;
    QString upper = text.toUpper();
    for (int i = 0; i <= upper.size() - 4; ++i) {
        QString qg = upper.mid(i, 4);
        bool found = false;
        for (const auto& entry : m_quadgrams) {
            if (entry.first == qg) { score += entry.second; found = true; break; }
        }
        if (!found) score -= 4.0; // penalty for unseen quadgram
    }
    return score;
}

/* ---- Random key ---- */

QVector<int> ColumnarTransposition::randomKey(int length) const
{
    QVector<int> key(length);
    for (int i = 0; i < length; ++i) key[i] = i;
    for (int i = length - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(key[i], key[j]);
    }
    return key;
}

/* ---- Swap key ---- */

QVector<int> ColumnarTransposition::swapKey(const QVector<int>& key, int i, int j)
{
    QVector<int> result = key;
    std::swap(result[i], result[j]);
    return result;
}

/* ---- Permutation to string ---- */

QString ColumnarTransposition::permutationToString(const QVector<int>& perm)
{
    QString s;
    for (int v : perm) s += QChar('A' + (v % 26));
    return s;
}

/* ---- Recover key via hill climbing ---- */

QString ColumnarTransposition::recoverKey(const QString& ciphertext, int maxKeyLen) const
{
    QElapsedTimer timer;
    timer.start();

    double bestScore = -1e18;
    QVector<int> bestKey;

    for (int keyLen = 2; keyLen <= maxKeyLen; ++keyLen) {
        QVector<int> currentKey = randomKey(keyLen);
        double currentScore = quadgramScore(applyColumnPermutation(ciphertext, currentKey, false));

        // Hill climbing with random restarts
        for (int restart = 0; restart < 5; ++restart) {
            bool improved = true;
            while (improved) {
                improved = false;
                for (int i = 0; i < keyLen; ++i) {
                    for (int j = i + 1; j < keyLen; ++j) {
                        QVector<int> candidate = swapKey(currentKey, i, j);
                        double candScore = quadgramScore(applyColumnPermutation(ciphertext, candidate, false));
                        if (candScore > currentScore) {
                            currentKey = candidate;
                            currentScore = candScore;
                            improved = true;
                        }
                    }
                }
            }
            if (currentScore > bestScore) { bestScore = currentScore; bestKey = currentKey; }
            currentKey = randomKey(keyLen);
            currentScore = quadgramScore(applyColumnPermutation(ciphertext, currentKey, false));
        }
    }

    return permutationToString(bestKey);
}

/* ---- Reset ---- */

void ColumnarTransposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
