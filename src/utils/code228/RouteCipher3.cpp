/**
 * @file RouteCipher3.cpp
 * @brief RouteCipher3 实现
 *
 * 实现路线密码：螺旋与对角遍历变体及模拟退火路线模式恢复。
 */

#include "utils/code228/RouteCipher3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RouteCipher3::RouteCipher3(QObject *parent) : QObject(parent) {}
RouteCipher3::~RouteCipher3() = default;

/* ---- LCG random ---- */

double RouteCipher3::randUniform()
{
    m_seed = (m_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return static_cast<double>(m_seed) / 0x7FFFFFFF;
}

/* ---- Configuration ---- */

void RouteCipher3::setGridSize(int rows, int cols)
{
    m_rows = qMax(2, rows);
    m_cols = qMax(2, cols);
}

/* ---- Build grid from text (row-major fill) ---- */

QVector<QVector<QChar>> RouteCipher3::buildGrid(const QString& text) const
{
    QVector<QVector<QChar>> grid(m_rows, QVector<QChar>(m_cols, QChar('X')));
    int idx = 0;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (idx < text.length())
                grid[r][c] = text[idx++];
    return grid;
}

/* ---- Spiral clockwise traversal ---- */

QVector<QChar> RouteCipher3::readSpiralCW(
    const QVector<QVector<QChar>>& grid) const
{
    QVector<QChar> result;
    int top = 0, bot = m_rows - 1, left = 0, right = m_cols - 1;
    while (top <= bot && left <= right) {
        for (int c = left; c <= right; ++c) result.append(grid[top][c]);
        ++top;
        for (int r = top; r <= bot; ++r) result.append(grid[r][right]);
        --right;
        if (top <= bot) {
            for (int c = right; c >= left; --c) result.append(grid[bot][c]);
            --bot;
        }
        if (left <= right) {
            for (int r = bot; r >= top; --r) result.append(grid[r][left]);
            ++left;
        }
    }
    return result;
}

/* ---- Spiral counter-clockwise traversal ---- */

QVector<QChar> RouteCipher3::readSpiralCCW(
    const QVector<QVector<QChar>>& grid) const
{
    QVector<QChar> result;
    int top = 0, bot = m_rows - 1, left = 0, right = m_cols - 1;
    while (top <= bot && left <= right) {
        for (int r = top; r <= bot; ++r) result.append(grid[r][left]);
        ++left;
        for (int c = left; c <= right; ++c) result.append(grid[bot][c]);
        --bot;
        if (left <= right) {
            for (int r = bot; r >= top; --r) result.append(grid[r][right]);
            --right;
        }
        if (top <= bot) {
            for (int c = right; c >= left; --c) result.append(grid[top][c]);
            ++top;
        }
    }
    return result;
}

/* ---- Diagonal downward zigzag ---- */

QVector<QChar> RouteCipher3::readDiagonalDown(
    const QVector<QVector<QChar>>& grid) const
{
    QVector<QChar> result;
    for (int d = 0; d < m_rows + m_cols - 1; ++d) {
        int r = qMin(d, m_rows - 1);
        int c = d - r;
        while (r >= 0 && c < m_cols) {
            result.append(grid[r][c]);
            --r; ++c;
        }
    }
    return result;
}

/* ---- Diagonal upward zigzag ---- */

QVector<QChar> RouteCipher3::readDiagonalUp(
    const QVector<QVector<QChar>>& grid) const
{
    QVector<QChar> result;
    for (int d = 0; d < m_rows + m_cols - 1; ++d) {
        int c = qMin(d, m_cols - 1);
        int r = d - c;
        while (c >= 0 && r < m_rows) {
            result.append(grid[r][c]);
            ++r; --c;
        }
    }
    return result;
}

/* ---- Write chars into grid by pattern (inverse of read) ---- */

QVector<QVector<QChar>> RouteCipher3::writeByPattern(
    const QVector<QChar>& chars, Pattern pattern) const
{
    // Build identity grid, get traversal order, then fill by that order
    QVector<QVector<QChar>> grid(m_rows, QVector<QChar>(m_cols, QChar(' ')));
    QVector<QChar> idChars;
    for (int i = 0; i < m_rows * m_cols; ++i) idChars.append(QChar(' '));
    // Generate traversal indices by reading a coordinate grid
    QVector<int> rowOrd, colOrd;
    QVector<QVector<QChar>> coordGrid(m_rows, QVector<QChar>(m_cols));
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            coordGrid[r][c] = QChar(r * m_cols + c);

    QVector<QChar> order;
    switch (pattern) {
        case SpiralCW: order = readSpiralCW(coordGrid); break;
        case SpiralCCW: order = readSpiralCCW(coordGrid); break;
        case DiagonalDown: order = readDiagonalDown(coordGrid); break;
        case DiagonalUp: order = readDiagonalUp(coordGrid); break;
    }

    for (int i = 0; i < qMin(chars.size(), order.size()); ++i) {
        int pos = order[i].unicode();
        grid[pos / m_cols][pos % m_cols] = chars[i];
    }
    return grid;
}

/* ---- Encrypt ---- */

QString RouteCipher3::encrypt(const QString& plaintext, Pattern pattern) const
{
    QElapsedTimer timer;
    timer.start();

    auto grid = buildGrid(plaintext);
    QVector<QChar> cipherChars;
    switch (pattern) {
        case SpiralCW: cipherChars = readSpiralCW(grid); break;
        case SpiralCCW: cipherChars = readSpiralCCW(grid); break;
        case DiagonalDown: cipherChars = readDiagonalDown(grid); break;
        case DiagonalUp: cipherChars = readDiagonalUp(grid); break;
    }

    QString result;
    for (const auto& ch : cipherChars) result += ch;
    const_cast<RouteCipher3*>(this)->m_stats.totalOps++;
    const_cast<RouteCipher3*>(this)->m_timeSum += timer.elapsed();
    const_cast<RouteCipher3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<RouteCipher3*>(this)->encryptCompleted(
        result.length(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString RouteCipher3::decrypt(const QString& ciphertext, Pattern pattern) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QChar> chars;
    for (int i = 0; i < ciphertext.length(); ++i)
        chars.append(ciphertext[i]);

    auto grid = writeByPattern(chars, pattern);
    QString result;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (grid[r][c] != QChar('X') && grid[r][c] != QChar(' '))
                result += grid[r][c];

    return result;
}

/* ---- Score text against dictionary ---- */

double RouteCipher3::scoreText(const QString& text,
                                const QVector<QString>& dict) const
{
    double score = 0.0;
    QString lower = text.toLower();
    for (const auto& word : dict) {
        if (word.isEmpty()) continue;
        int pos = 0;
        while ((pos = lower.indexOf(word.toLower(), pos)) != -1) {
            score += word.length();
            ++pos;
        }
    }
    return score;
}

/* ---- Simulated annealing route recovery ---- */

QString RouteCipher3::annealRecover(const QString& ciphertext,
                                     const QVector<QString>& dictionary,
                                     int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    // Try all 4 patterns, pick best scored decryption via SA perturbation
    QVector<Pattern> patterns = {SpiralCW, SpiralCCW, DiagonalDown, DiagonalUp};
    double bestScore = -1e9;
    QString bestText;

    for (Pattern pat : patterns) {
        QString dec = decrypt(ciphertext, pat);
        double sc = scoreText(dec, dictionary);

        // SA perturbation: try row/col swaps
        double temperature = 10.0;
        int curRows = m_rows, curCols = m_cols;
        double curScore = sc;
        QString curText = dec;

        for (int it = 0; it < maxIter; ++it) {
            // Perturb grid: swap rows or cols
            int nr = curRows, nc = curCols;
            if (randUniform() < 0.5)
                nr = qMax(2, curRows + (randUniform() < 0.5 ? 1 : -1));
            else
                nc = qMax(2, curCols + (randUniform() < 0.5 ? 1 : -1));

            // Temporary resize
            int savedR = m_rows, savedC = m_cols;
            const_cast<RouteCipher3*>(this)->m_rows = nr;
            const_cast<RouteCipher3*>(this)->m_cols = nc;

            QString newDec = decrypt(ciphertext, pat);
            double newScore = scoreText(newDec, dictionary);

            double delta = newScore - curScore;
            double accept = (delta > 0) ? 1.0 : qExp(delta / qMax(0.01, temperature));

            if (randUniform() < accept) {
                curScore = newScore;
                curText = newDec;
                curRows = nr; curCols = nc;
            } else {
                const_cast<RouteCipher3*>(this)->m_rows = savedR;
                const_cast<RouteCipher3*>(this)->m_cols = savedC;
            }

            temperature *= 0.995;
            if (it % 100 == 0)
                emit annealProgress(it, curScore);
        }

        if (curScore > bestScore) { bestScore = curScore; bestText = curText; }
    }

    m_stats.annealIterations = maxIter;
    m_stats.bestScore = bestScore;
    m_stats.textLength = ciphertext.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return bestText;
}

/* ---- Reset ---- */

void RouteCipher3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
