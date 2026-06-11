/**
 * @file FoursquareCode15.cpp
 * @brief FoursquareCode15 实现
 *
 * 实现四方密码：二部坐标映射与双表交叉引用实现增强型波利比乌斯加密。
 */

#include "utils/code292/FoursquareCode15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode15::FoursquareCode15(QObject *parent)
    : QObject(parent)
{
    buildStandardTableau(m_tl);
    buildStandardTableau(m_br);
    buildTableau(m_kw1, m_tr);
    buildTableau(m_kw2, m_bl);
}

FoursquareCode15::~FoursquareCode15() = default;

/* ---- Build standard A-Z tableau ---- */

void FoursquareCode15::buildStandardTableau(Tableau& tab) const
{
    int letter = 0;
    for (int r = 0; r < TABLE_SIZE; ++r) {
        for (int c = 0; c < TABLE_SIZE; ++c) {
            // Skip 'J' (index 9)
            int ch = letter;
            if (ch >= 9) ++ch; // shift J→K, etc.
            tab.grid[r][c] = ch;
            tab.position[ch][0] = r;
            tab.position[ch][1] = c;
            ++letter;
        }
    }
}

/* ---- Build tableau from keyword ---- */

void FoursquareCode15::buildTableau(const QString& keyword, Tableau& tab) const
{
    QVector<bool> used(ALPHA_SIZE, false);
    QVector<int> order;
    order.reserve(ALPHA_SIZE);

    // Add unique keyword letters
    for (QChar ch : keyword.toUpper()) {
        int idx = charToIndex(ch);
        if (idx >= 0 && idx < ALPHA_SIZE && !used[idx]) {
            used[idx] = true;
            order.append(idx);
        }
    }
    // Fill remaining letters in order
    for (int i = 0; i < ALPHA_SIZE; ++i) {
        if (!used[i]) order.append(i);
    }

    // Fill grid
    int pos = 0;
    for (int r = 0; r < TABLE_SIZE; ++r) {
        for (int c = 0; c < TABLE_SIZE; ++c) {
            int letter = order[pos++];
            tab.grid[r][c] = letter;
            tab.position[letter][0] = r;
            tab.position[letter][1] = c;
        }
    }
}

/* ---- Set keywords and rebuild tableaux ---- */

void FoursquareCode15::setKeywords(const QString& kw1, const QString& kw2)
{
    m_kw1 = kw1;
    m_kw2 = kw2;
    buildStandardTableau(m_tl);
    buildStandardTableau(m_br);
    buildTableau(m_kw1, m_tr);
    buildTableau(m_kw2, m_bl);
}

/* ---- Normalize text ---- */

QString FoursquareCode15::normalize(const QString& text) const
{
    QString result;
    result.reserve(text.size());
    for (QChar ch : text.toUpper()) {
        if (ch >= 'A' && ch <= 'Z') {
            if (ch == 'J') ch = 'I';
            result.append(ch);
        }
    }
    return result;
}

/* ---- Pad to even length ---- */

QString FoursquareCode15::padEven(const QString& text) const
{
    QString result = text;
    if (result.size() % 2 != 0)
        result.append('X');    // Padding character
    return result;
}

/* ---- Character mapping ---- */

int FoursquareCode15::charToIndex(QChar ch) const
{
    if (ch >= 'A' && ch <= 'Z') {
        int idx = ch.toLatin1() - 'A';
        if (idx > 8) --idx;   // Merge J into I
        return idx;
    }
    return -1;
}

QChar FoursquareCode15::indexToChar(int idx) const
{
    if (idx < 0 || idx >= ALPHA_SIZE) return 'A';
    if (idx >= 9) ++idx;  // Skip J
    return QChar('A' + idx);
}

/* ---- Encrypt ---- */

FoursquareCode15::CipherResult FoursquareCode15::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString norm = padEven(normalize(plaintext));
    int n = norm.size();
    result.inputLength = n;

    // Bipartite digraph encryption via dual-tableau cross-reference
    QString ct;
    ct.reserve(n);
    for (int i = 0; i < n; i += 2) {
        int a = charToIndex(norm[i]);
        int b = charToIndex(norm[i + 1]);

        // Coordinate mapping: find positions in plain tableau (TL)
        int rowA = m_tl.position[a][0];
        int colA = m_tl.position[a][1];
        int rowB = m_br.position[b][0];
        int colB = m_br.position[b][1];

        // Cross-reference: TR[rowA][colB] and BL[rowB][colA]
        int encA = m_tr.grid[rowA][colB];
        int encB = m_bl.grid[rowB][colA];

        ct.append(indexToChar(encA));
        ct.append(indexToChar(encB));
    }

    result.ciphertext = ct;
    result.outputLength = ct.size();

    double elapsed = timer.elapsed();
    m_stats.encryptCount++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptDone(n, ct.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

FoursquareCode15::CipherResult FoursquareCode15::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString norm = normalize(ciphertext);
    int n = norm.size();
    result.inputLength = n;

    QString pt;
    pt.reserve(n);
    for (int i = 0; i + 1 < n; i += 2) {
        int a = charToIndex(norm[i]);
        int b = charToIndex(norm[i + 1]);

        // Find positions in key tableaux
        int rowA = m_tr.position[a][0];
        int colA = m_tr.position[a][1];
        int rowB = m_bl.position[b][0];
        int colB = m_bl.position[b][1];

        // Reverse cross-reference: TL[rowA][colB] and BR[rowB][colA]
        int decA = m_tl.grid[rowA][colB];
        int decB = m_br.grid[rowB][colA];

        pt.append(indexToChar(decA));
        pt.append(indexToChar(decB));
    }

    result.ciphertext = pt;
    result.outputLength = pt.size();

    double elapsed = timer.elapsed();
    m_stats.decryptCount++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decryptDone(n, pt.size(), elapsed);
    return result;
}

/* ---- Reset ---- */

void FoursquareCode15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
