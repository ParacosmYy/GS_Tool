/**
 * @file FoursquareCode7.cpp
 * @brief FoursquareCode7 实现
 *
 * 实现四方密码：扩展14x14栅格、三层列转置与Polybius坐标编码。
 */

#include "utils/code236/FoursquareCode7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode7::FoursquareCode7(QObject *parent) : QObject(parent) {}
FoursquareCode7::~FoursquareCode7() = default;

/* ---- Build 14x14 grid ---- */

void FoursquareCode7::buildGrid(const QString& keyword, QVector<QVector<QChar>>& grid)
{
    grid.resize(14);
    for (auto& row : grid) row.resize(14);

    QVector<bool> used(196, false);  // 14x14 = 196 cells
    QString chars = keyword.toUpper();
    // Remove non-alpha but allow digits and extended symbols
    QString unique;
    for (QChar ch : chars) {
        int idx = ch.unicode();
        if (idx < 0 || idx >= 196) continue;
        if (!used[idx]) { unique.append(ch); used[idx] = true; }
    }

    // Fill remaining with extended alphabet (A-Z, 0-9, and symbols)
    QString extended = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-:?/+=()&%$#@!^*<>{}[]|~");
    for (QChar ch : extended) {
        int idx = ch.unicode() % 196;
        if (!used[idx]) { unique.append(ch); used[idx] = true; }
    }
    // Pad remaining cells
    for (int i = 0; i < 196; ++i) {
        if (!used[i]) unique.append(QChar(32 + i));
    }

    for (int r = 0; r < 14; ++r)
        for (int c = 0; c < 14; ++c)
            grid[r][c] = unique[r * 14 + c];
}

/* ---- Find character in grid ---- */

bool FoursquareCode7::findInGrid(const QVector<QVector<QChar>>& grid, QChar ch, int& row, int& col) const
{
    for (int r = 0; r < grid.size(); ++r)
        for (int c = 0; c < grid[r].size(); ++c)
            if (grid[r][c] == ch.toUpper()) { row = r; col = c; return true; }
    return false;
}

/* ---- Column order from key ---- */

QVector<int> FoursquareCode7::columnOrder(const QString& key) const
{
    int n = key.size();
    if (n == 0) return {};
    QVector<QPair<QChar, int>> indexed;
    for (int i = 0; i < n; ++i) indexed.append({key[i], i});
    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first || (a.first == b.first && a.second < b.second);
              });
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[indexed[i].second] = i;
    return order;
}

/* ---- Single-layer columnar transposition ---- */

QString FoursquareCode7::columnarTranspose(const QString& text, const QString& key, bool encrypt) const
{
    int cols = key.size();
    if (cols <= 0) return text;
    QVector<int> order = columnOrder(key);
    int rows = qCeil(static_cast<double>(text.size()) / cols);

    if (encrypt) {
        // Write row-wise, read column-wise in key order
        QVector<QString> cols_data(cols);
        for (int i = 0; i < text.size(); ++i)
            cols_data[i % cols].append(text[i]);

        QString result;
        for (int rank = 0; rank < cols; ++rank) {
            for (int c = 0; c < cols; ++c) {
                if (order[c] == rank) { result += cols_data[c]; break; }
            }
        }
        return result;
    } else {
        // Reverse: distribute by column, read row-wise
        QVector<int> colLens(cols, rows);
        int extra = text.size() % cols;
        if (extra != 0) for (int i = extra; i < cols; ++i) colLens[order[i]]--;

        QVector<QString> cols_data(cols);
        int pos = 0;
        for (int rank = 0; rank < cols; ++rank) {
            for (int c = 0; c < cols; ++c) {
                if (order[c] == rank) {
                    cols_data[c] = text.mid(pos, colLens[c]);
                    pos += colLens[c];
                    break;
                }
            }
        }

        QString result;
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                if (r < cols_data[c].size()) result += cols_data[c][r];
        return result;
    }
}

/* ---- Triple-layer columnar transposition ---- */

QString FoursquareCode7::tripleTranspose(const QString& text, bool encrypt) const
{
    QString result = text;
    if (encrypt) {
        for (int i = 0; i < m_transKeys.size() && i < 3; ++i)
            result = columnarTranspose(result, m_transKeys[i], true);
    } else {
        for (int i = m_transKeys.size() - 1; i >= 0; --i)
            if (i < 3) result = columnarTranspose(result, m_transKeys[i], false);
    }
    return result;
}

/* ---- Grid char ---- */

QChar FoursquareCode7::gridChar(int row, int col) const
{
    if (row < 14 && col < 14) return QChar('A' + (row * 14 + col) % 26);
    return QChar('X');
}

/* ---- Coordinates ---- */

QVector<QPair<int, int>> FoursquareCode7::toCoordinates(const QString& text) const
{
    QVector<QPair<int, int>> coords;
    for (QChar ch : text) {
        int row = ch.unicode() % 14;
        int col = (ch.unicode() / 14) % 14;
        coords.append({row, col});
    }
    return coords;
}

/* ---- Pad text ---- */

QString FoursquareCode7::padText(const QString& text) const
{
    QString result = text;
    while (result.size() % 2 != 0) result += 'X';
    return result;
}

/* ---- Encrypt ---- */

QString FoursquareCode7::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    buildGrid(m_keyword1.isEmpty() ? "KEYWORD1" : m_keyword1, m_gridTL);
    buildGrid(m_keyword1.isEmpty() ? "KEYWORD1" : m_keyword1, m_gridTR);
    buildGrid(m_keyword2.isEmpty() ? "KEYWORD2" : m_keyword2, m_gridBL);
    buildGrid(m_keyword2.isEmpty() ? "KEYWORD2" : m_keyword2, m_gridBR);

    QString padded = padText(plaintext.toUpper());

    // Foursquare encryption: bigram substitution
    QString substituted;
    for (int i = 0; i < padded.size(); i += 2) {
        int r1, c1, r2, c2;
        if (!findInGrid(m_gridTL, padded[i], r1, c1)) { r1 = 0; c1 = i % 14; }
        if (!findInGrid(m_gridBR, padded[i + 1], r2, c2)) { r2 = 0; c2 = (i + 1) % 14; }
        // Foursquare: use opposite corners
        substituted += m_gridTR[r1][c2];
        substituted += m_gridBL[r2][c1];
    }

    // Apply triple columnar transposition
    QString result = tripleTranspose(substituted, true);

    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptCompleted(plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FoursquareCode7::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    buildGrid(m_keyword1.isEmpty() ? "KEYWORD1" : m_keyword1, m_gridTL);
    buildGrid(m_keyword1.isEmpty() ? "KEYWORD1" : m_keyword1, m_gridTR);
    buildGrid(m_keyword2.isEmpty() ? "KEYWORD2" : m_keyword2, m_gridBL);
    buildGrid(m_keyword2.isEmpty() ? "KEYWORD2" : m_keyword2, m_gridBR);

    // Reverse triple columnar transposition first
    QString untransposed = tripleTranspose(ciphertext, true);

    // Reverse bigram substitution
    QString result;
    for (int i = 0; i < untransposed.size(); i += 2) {
        int r1, c1, r2, c2;
        if (!findInGrid(m_gridTR, untransposed[i], r1, c2)) { r1 = 0; c2 = 0; }
        if (!findInGrid(m_gridBL, untransposed[i + 1], r2, c1)) { r2 = 0; c1 = 0; }
        result += m_gridTL[r1][c1];
        result += m_gridBR[r2][c2];
    }

    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptCompleted(ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Accessors ---- */

QVector<QVector<QChar>> FoursquareCode7::grid(int index) const
{
    switch (index) {
    case 0: return m_gridTL;
    case 1: return m_gridTR;
    case 2: return m_gridBL;
    case 3: return m_gridBR;
    default: return {};
    }
}

/* ---- Configuration ---- */

void FoursquareCode7::setKeyword1(const QString& kw) { m_keyword1 = kw; }
void FoursquareCode7::setKeyword2(const QString& kw) { m_keyword2 = kw; }
void FoursquareCode7::setTranspositionKeys(const QVector<QString>& keys) { m_transKeys = keys; }

/* ---- Reset ---- */

void FoursquareCode7::resetStatistics()
{
    m_gridTL.clear(); m_gridTR.clear(); m_gridBL.clear(); m_gridBR.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
