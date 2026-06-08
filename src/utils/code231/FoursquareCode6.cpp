/**
 * @file FoursquareCode6.cpp
 * @brief FoursquareCode6 实现
 *
 * 实现四方密码：12x12扩展方阵与双列置换嵌套关键词。
 */

#include "utils/code231/FoursquareCode6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode6::FoursquareCode6(QObject *parent) : QObject(parent) {}
FoursquareCode6::~FoursquareCode6() = default;

/* ---- Build character set ---- */

QString FoursquareCode6::buildCharset() const
{
    QString cs;
    // a-z (26) + A-Z (26) + 0-9 (10) + common punctuation + padding
    for (int c = 'a'; c <= 'z'; ++c) cs.append(QChar(c));
    for (int c = 'A'; c <= 'Z'; ++c) cs.append(QChar(c));
    for (int c = '0'; c <= '9'; ++c) cs.append(QChar(c));
    // Add punctuation to fill 144 slots
    QString punct = " .,!?;:'-()[]{}@#$%&*+=/<>\"\\|~^_`";
    for (QChar ch : punct) {
        if (!cs.contains(ch)) cs.append(ch);
    }
    // Pad to 144
    while (cs.size() < CHARSET_SIZE)
        cs.append(QChar(cs.size()));
    return cs;
}

/* ---- Build grid from keyword ---- */

void FoursquareCode6::buildGrid(QVector<QVector<QChar>>& grid, const QString& keyword)
{
    grid.resize(GRID_SIZE);
    for (int i = 0; i < GRID_SIZE; ++i)
        grid[i].resize(GRID_SIZE);

    QString charset = buildCharset();

    // Remove keyword duplicates, keeping first occurrence
    QString filtered;
    for (QChar ch : keyword.toLower()) {
        if (!filtered.contains(ch) && charset.contains(ch))
            filtered.append(ch);
    }

    // Fill grid: keyword chars first, then remaining charset
    QString order = filtered;
    for (QChar ch : charset) {
        if (!order.contains(ch))
            order.append(ch);
    }

    int idx = 0;
    for (int r = 0; r < GRID_SIZE; ++r) {
        for (int c = 0; c < GRID_SIZE; ++c) {
            grid[r][c] = (idx < order.size()) ? order[idx] : QChar(idx);
            idx++;
        }
    }
}

/* ---- Find character in grid ---- */

bool FoursquareCode6::findInGrid(const QVector<QVector<QChar>>& grid, QChar ch, int& row, int& col) const
{
    for (int r = 0; r < GRID_SIZE; ++r) {
        for (int c = 0; c < GRID_SIZE; ++c) {
            if (grid[r][c] == ch) { row = r; col = c; return true; }
        }
    }
    row = 0; col = 0;
    return false;
}

/* ---- Column order from keyword ---- */

QVector<int> FoursquareCode6::columnOrder(const QString& key) const
{
    int n = key.size();
    if (n == 0) return {};

    QVector<int> order(n);
    QVector<int> assigned(n, 0);
    int rank = 0;
    for (int r = 0; r < 256; ++r) {
        for (int i = 0; i < n; ++i) {
            if (!assigned[i] && key[i].toLatin1() == r) {
                order[rank++] = i;
                assigned[i] = 1;
            }
        }
    }
    return order;
}

/* ---- Columnar transposition encrypt ---- */

QString FoursquareCode6::columnarEncrypt(const QString& text, const QString& key) const
{
    if (key.isEmpty()) return text;
    int nCols = key.size();
    QVector<int> order = columnOrder(key);

    QVector<QString> cols(nCols);
    for (int i = 0; i < text.size(); ++i)
        cols[i % nCols].append(text[i]);

    QString result;
    for (int rank = 0; rank < nCols; ++rank)
        result.append(cols[order[rank]]);
    return result;
}

/* ---- Columnar transposition decrypt ---- */

QString FoursquareCode6::columnarDecrypt(const QString& text, const QString& key) const
{
    if (key.isEmpty()) return text;
    int nCols = key.size();
    QVector<int> order = columnOrder(key);

    int nRows = qCeil(static_cast<double>(text.size()) / nCols);
    int extra = text.size() % nCols;

    QVector<QString> cols(nCols);
    int pos = 0;
    for (int rank = 0; rank < nCols; ++rank) {
        int colIdx = order[rank];
        int len = nRows;
        if (extra > 0 && colIdx >= extra) len = nRows - 1;
        cols[colIdx] = text.mid(pos, len);
        pos += len;
    }

    QString result;
    for (int r = 0; r < nRows; ++r) {
        for (int c = 0; c < nCols; ++c) {
            if (r < cols[c].size())
                result.append(cols[c][r]);
        }
    }
    return result;
}

/* ---- Set keywords ---- */

void FoursquareCode6::setKeywords(const QString& keyword1, const QString& keyword2,
                                    const QString& transKey1, const QString& transKey2)
{
    m_keyword1 = keyword1;
    m_keyword2 = keyword2;
    m_transKey1 = transKey1;
    m_transKey2 = transKey2;

    // Build 4 grids: TL=plain, TR=kw1, BL=kw2, BR=plain
    buildGrid(m_gridTL, QString());
    buildGrid(m_gridTR, keyword1);
    buildGrid(m_gridBL, keyword2);
    buildGrid(m_gridBR, QString());
}

/* ---- Encrypt ---- */

QString FoursquareCode6::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // First columnar transposition with nested keyword
    QString step1 = columnarEncrypt(plaintext, m_transKey1);
    QString step2 = columnarEncrypt(step1, m_transKey2);

    // Ensure even length for digraph pairing
    if (step2.size() % 2 != 0)
        step2.append('x');

    // Foursquare encryption: process digraphs
    QString result;
    for (int i = 0; i + 1 < step2.size(); i += 2) {
        int r1, c1, r2, c2;
        findInGrid(m_gridTL, step2[i], r1, c1);
        findInGrid(m_gridBR, step2[i + 1], r2, c2);

        // Encrypted pair: (gridTR[r1][c2], gridBL[r2][c1])
        result.append(m_gridTR[r1][c2]);
        result.append(m_gridBL[r2][c1]);
    }

    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.numTranspositions = 2;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode6::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Foursquare decryption: process digraphs
    QString intermediate;
    for (int i = 0; i + 1 < ciphertext.size(); i += 2) {
        int r1, c1, r2, c2;
        findInGrid(m_gridTR, ciphertext[i], r1, c1);
        findInGrid(m_gridBL, ciphertext[i + 1], r2, c2);

        // Decrypted pair: (gridTL[r1][c2], gridBR[r2][c1])
        intermediate.append(m_gridTL[r1][c2]);
        intermediate.append(m_gridBR[r2][c1]);
    }

    // Reverse columnar transpositions
    QString step1 = columnarDecrypt(intermediate, m_transKey2);
    QString result = columnarDecrypt(step1, m_transKey1);

    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.numTranspositions = 2;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void FoursquareCode6::resetStatistics()
{
    m_gridTL.clear();
    m_gridTR.clear();
    m_gridBL.clear();
    m_gridBR.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
