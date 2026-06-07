/**
 * @file DigrafidCode.cpp
 * @brief DigrafidCode 实现
 *
 * 实现Digrafid密码：双图替换、分数坐标分解、行列转置。
 */

#include "utils/code201/DigrafidCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode::DigrafidCode(QObject *parent) : QObject(parent)
{
    m_charSet = "ABCDEFGHIKLMNOPQRSTUVWXYZ#"; // 27 chars (J merged with I)
    buildSubstitutionTable(m_key);
}

DigrafidCode::~DigrafidCode() = default;

/* ---- Configuration ---- */

void DigrafidCode::setKey(const QString& key)
{
    m_key = key.toUpper();
    buildSubstitutionTable(m_key);
}

void DigrafidCode::setPeriod(int period) { m_period = qMax(0, period); }

/* ---- Character mapping ---- */

int DigrafidCode::charToIdx(QChar c) const
{
    QChar uc = c.toUpper();
    // Map J -> I
    if (uc == 'J') uc = 'I';
    int idx = m_charSet.indexOf(uc);
    return idx >= 0 ? idx : (m_charSet.size() - 1); // unknown -> #
}

QChar DigrafidCode::idxToChar(int idx) const
{
    if (idx >= 0 && idx < m_charSet.size()) return m_charSet[idx];
    return '#';
}

/* ---- Build substitution table ---- */

void DigrafidCode::buildSubstitutionTable(const QString& key)
{
    Q_UNUSED(key)
    int n = m_charSet.size(); // 27

    // Build a 3x3x3 fractional coordinate system
    // Row table: maps char index -> (row, col) in 3x9 grid
    m_rowTable.resize(n);
    m_colTable.resize(n);
    for (int i = 0; i < n; ++i) {
        // First coordinate: row in 3x3 grid (0-2)
        int r1 = i / 9;
        int c1 = (i / 3) % 3;
        int r2 = i % 3;
        m_rowTable[i] = {r1, c1};
        m_colTable[i] = {r2};
    }
}

/* ---- Fractionate a digraph ---- */

QPair<QVector<int>, QVector<int>> DigrafidCode::fractionate(QChar a, QChar b) const
{
    int ia = charToIdx(a);
    int ib = charToIdx(b);

    // Extract row coordinates from first char, column from second
    QVector<int> rowCoords = {ia / 9, (ia / 3) % 3, ia % 3};
    QVector<int> colCoords = {ib / 9, (ib / 3) % 3, ib % 3};

    return {rowCoords, colCoords};
}

/* ---- Transpose by period ---- */

QVector<int> DigrafidCode::transpose(const QVector<int>& coords, int period) const
{
    int n = coords.size();
    if (period <= 0 || period >= n) return coords;

    // Read row-by-row, write column-by-column
    QVector<int> result(n);
    int rows = (n + period - 1) / period;
    int idx = 0;
    for (int col = 0; col < period; ++col)
        for (int row = 0; row < rows; ++row) {
            int srcIdx = row * period + col;
            if (srcIdx < n && idx < n)
                result[idx++] = coords[srcIdx];
        }
    return result;
}

/* ---- Reverse transpose ---- */

QVector<int> DigrafidCode::reverseTranspose(const QVector<int>& coords, int period) const
{
    int n = coords.size();
    if (period <= 0 || period >= n) return coords;

    // Inverse of the column-major read
    QVector<int> result(n);
    int rows = (n + period - 1) / period;
    int idx = 0;
    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < period; ++col) {
            int srcIdx = col * rows + row;
            if (srcIdx < n && idx < n)
                result[idx++] = coords[srcIdx];
        }
    return result;
}

/* ---- Defractionate coordinates ---- */

QVector<QPair<QChar, QChar>> DigrafidCode::defractionate(const QVector<int>& coords) const
{
    QVector<QPair<QChar, QChar>> result;
    int n = coords.size();
    for (int i = 0; i + 5 < n; i += 6) {
        // Reconstruct two chars from 6 base-3 coordinates
        int a = coords[i] * 9 + coords[i + 1] * 3 + coords[i + 2];
        int b = coords[i + 3] * 9 + coords[i + 4] * 3 + coords[i + 5];
        a = qBound(0, a, m_charSet.size() - 1);
        b = qBound(0, b, m_charSet.size() - 1);
        result.append({idxToChar(a), idxToChar(b)});
    }
    return result;
}

/* ---- Encrypt ---- */

QString DigrafidCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString pt = plaintext.toUpper();
    // Pad to even length
    if (pt.size() % 2 != 0) pt += 'X';

    // Collect all fractional coordinates
    QVector<int> allCoords;
    for (int i = 0; i < pt.size(); i += 2) {
        auto [rowC, colC] = fractionate(pt[i], pt[i + 1]);
        for (int c : rowC) allCoords.append(c);
        for (int c : colC) allCoords.append(c);
    }

    int period = m_period > 0 ? m_period : pt.size() / 2;
    QVector<int> transposed = transpose(allCoords, period);
    auto pairs = defractionate(transposed);

    QString result;
    for (const auto& p : pairs) {
        result += p.first;
        result += p.second;
    }

    m_stats.totalOperations++;
    m_stats.inputLength = plaintext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted("encrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString DigrafidCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString ct = ciphertext.toUpper();
    QVector<int> allCoords;
    for (int i = 0; i < ct.size(); i += 2) {
        auto [rowC, colC] = fractionate(ct[i], ct[i + 1]);
        for (int c : rowC) allCoords.append(c);
        for (int c : colC) allCoords.append(c);
    }

    int period = m_period > 0 ? m_period : ct.size() / 2;
    QVector<int> reversed = reverseTranspose(allCoords, period);
    auto pairs = defractionate(reversed);

    QString result;
    for (const auto& p : pairs) {
        result += p.first;
        result += p.second;
    }

    m_stats.totalOperations++;
    m_stats.inputLength = ciphertext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted("decrypt", result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DigrafidCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
