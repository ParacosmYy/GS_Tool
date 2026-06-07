/**
 * @file TwoSquareCode.cpp
 * @brief TwoSquareCode 实现
 *
 * 实现双方阵密码：配对5x5网格、双字母坐标映射、互质周期分析。
 */

#include "utils/code191/TwoSquareCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <QSet>

/* ---- Construction / Destruction ---- */

TwoSquareCode::TwoSquareCode(QObject *parent) : QObject(parent)
{
    // Default: no keyword, use standard alphabet grids
    m_grid1 = buildGrid("");
    m_grid2 = buildGrid("");
}

TwoSquareCode::~TwoSquareCode() = default;

/* ---- Build 5x5 grid from keyword ---- */

QVector<QChar> TwoSquareCode::buildGrid(const QString& key) const
{
    QVector<QChar> grid;
    grid.reserve(25);
    QSet<QChar> used;

    // Add keyword characters (J merged into I)
    for (QChar ch : key.toUpper()) {
        if (!ch.isLetter()) continue;
        QChar c = (ch == 'J') ? 'I' : ch;
        if (!used.contains(c)) {
            used.insert(c);
            grid.append(c);
        }
    }

    // Fill remaining alphabet (A-Z excluding J)
    for (char c = 'A'; c <= 'Z'; ++c) {
        QChar ch(c);
        if (ch == 'J') continue;
        if (!used.contains(ch)) {
            used.insert(ch);
            grid.append(ch);
        }
    }

    // Trim to exactly 25
    grid.resize(25);
    return grid;
}

/* ---- Set keyword ---- */

void TwoSquareCode::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper();
    m_grid1 = buildGrid(m_keyword);
    // Grid 2 uses keyword reversed for variation
    QString rev;
    for (int i = m_keyword.size() - 1; i >= 0; --i)
        rev += m_keyword[i];
    m_grid2 = buildGrid(rev);
    m_stats.keyLength = m_keyword.size();
}

/* ---- Find position in grid ---- */

QPair<int, int> TwoSquareCode::findPosition(const QVector<QChar>& grid,
                                              QChar ch) const
{
    ch = (ch == 'J') ? 'I' : ch.toUpper();
    for (int i = 0; i < grid.size(); ++i) {
        if (grid[i] == ch)
            return {i / 5, i % 5};
    }
    return {0, 0};
}

/* ---- Prepare text ---- */

QString TwoSquareCode::prepareText(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (ch.isLetter()) {
            QChar c = (ch == 'J') ? 'I' : ch;
            result += c;
        }
    }
    // Pad with 'X' if odd length
    if (result.size() % 2 != 0)
        result += 'X';
    return result;
}

/* ---- Encrypt digraph ---- */

QPair<QChar, QChar> TwoSquareCode::encryptDigraph(QChar a, QChar b) const
{
    // Two-square: use grid1 for a, grid2 for b
    auto posA = findPosition(m_grid1, a);
    auto posB = findPosition(m_grid2, b);

    // Same row: swap columns (horizontal mapping)
    if (posA.first == posB.first) {
        int idxA = posA.first * 5 + posB.second;
        int idxB = posB.first * 5 + posA.second;
        return {m_grid1[idxA], m_grid2[idxB]};
    }

    // Different row: rectangle mapping (swap column coordinates)
    int idxA = posA.first * 5 + posB.second;
    int idxB = posB.first * 5 + posA.second;
    return {m_grid1[idxA], m_grid2[idxB]};
}

/* ---- Decrypt digraph ---- */

QPair<QChar, QChar> TwoSquareCode::decryptDigraph(QChar a, QChar b) const
{
    // Decryption is the same as encryption for two-square
    return encryptDigraph(a, b);
}

/* ---- GCD ---- */

int TwoSquareCode::gcd(int a, int b) const
{
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}

/* ---- Encrypt ---- */

QString TwoSquareCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(plaintext);
    QString result;

    for (int i = 0; i < prepared.size(); i += 2) {
        auto enc = encryptDigraph(prepared[i], prepared[i + 1]);
        result += enc.first;
        result += enc.second;
    }

    m_stats.totalOps++;
    m_stats.digraphCount = result.size() / 2;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("encrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString TwoSquareCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString prepared = prepareText(ciphertext);
    QString result;

    for (int i = 0; i < prepared.size(); i += 2) {
        auto dec = decryptDigraph(prepared[i], prepared[i + 1]);
        result += dec.first;
        result += dec.second;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Period analysis via mutual-prime Kasiski ---- */

QVector<QString> TwoSquareCode::periodAnalysis(const QString& ciphertext,
                                                 int maxPeriod) const
{
    QVector<QString> candidates;
    QString ct = ciphertext.toUpper();
    int n = ct.size();

    // Find repeated digraphs and their spacings
    QMap<QString, QVector<int>> positions;
    for (int i = 0; i + 1 < n; i += 2) {
        QString dig = ct.mid(i, 2);
        positions[dig].append(i);
    }

    // Collect spacings between repeated digraphs
    QVector<int> spacings;
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        const auto& pos = it.value();
        for (int i = 0; i < pos.size(); ++i)
            for (int j = i + 1; j < pos.size(); ++j)
                spacings.append(pos[j] - pos[i]);
    }

    // Count GCD-based period candidates
    QMap<int, int> periodScores;
    for (int spacing : spacings) {
        for (int p = 2; p <= maxPeriod; ++p) {
            if (spacing % p == 0)
                periodScores[p]++;
        }
    }

    // Sort by score descending
    QVector<QPair<int, int>> scored;
    for (auto it = periodScores.begin(); it != periodScores.end(); ++it)
        scored.append({it.key(), it.value()});
    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Return top candidates as strings
    int count = qMin(5, scored.size());
    for (int i = 0; i < count; ++i)
        candidates.append(QString("period=%1 (score=%2)")
                              .arg(scored[i].first).arg(scored[i].second));

    return candidates;
}

/* ---- Get grids ---- */

QPair<QVector<QChar>, QVector<QChar>> TwoSquareCode::grids() const
{
    return {m_grid1, m_grid2};
}

/* ---- Reset ---- */

void TwoSquareCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
