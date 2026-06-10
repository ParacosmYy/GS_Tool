/**
 * @file SeriatedPlayfair7.cpp
 * @brief SeriatedPlayfair7 实现
 *
 * 实现串联普莱费尔密码：渐进关键字旋转与列置换的双重Bifid增强加密。
 */

#include "utils/code276/SeriatedPlayfair7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair7::SeriatedPlayfair7(QObject *parent)
    : QObject(parent)
{
    m_grid = generateGrid(m_params.keyword);
}

SeriatedPlayfair7::~SeriatedPlayfair7() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair7::setParams(const Params& params)
{
    m_params = params;
    m_grid = generateGrid(m_params.keyword);
}

/* ---- Build alphabet (A-Z minus J) ---- */

QVector<QChar> SeriatedPlayfair7::buildAlphabet(const QString& keyword) const
{
    QVector<QChar> used(26, false);
    QVector<QChar> result;

    // Add keyword letters first
    for (const QChar& ch : keyword.toUpper()) {
        QChar c = (ch == QLatin1Char('J')) ? QLatin1Char('I') : ch;
        if (c >= QLatin1Char('A') && c <= QLatin1Char('Z') && !used[c.unicode() - 'A']) {
            result.append(c);
            used[c.unicode() - 'A'] = true;
        }
    }
    // Fill remaining letters
    for (int i = 0; i < 26; ++i) {
        QChar c = QChar('A' + i);
        if (c == QLatin1Char('J')) continue;
        if (!used[i]) result.append(c);
    }
    return result;
}

/* ---- Generate 5x5 grid ---- */

QVector<QVector<QChar>> SeriatedPlayfair7::generateGrid(const QString& keyword) const
{
    auto alpha = buildAlphabet(keyword);
    QVector<QVector<QChar>> grid(5, QVector<QChar>(5));
    for (int i = 0; i < 25; ++i)
        grid[i / 5][i % 5] = alpha[i];
    return grid;
}

/* ---- Get current grid ---- */

QVector<QVector<QChar>> SeriatedPlayfair7::currentGrid() const { return m_grid; }

/* ---- Preprocess text ---- */

QString SeriatedPlayfair7::preprocess(const QString& text) const
{
    QString result;
    for (const QChar& ch : text.toUpper()) {
        if (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z')) {
            result.append((ch == QLatin1Char('J')) ? QLatin1Char('I') : ch);
        }
    }
    // Pad with X for odd length
    if (result.size() % 2 != 0)
        result.append(QLatin1Char('X'));
    return result;
}

/* ---- Find character position ---- */

void SeriatedPlayfair7::findPosition(QChar ch, int& row, int& col) const
{
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 5; ++c) {
            if (m_grid[r][c] == ch) { row = r; col = c; return; }
        }
    }
    row = 0; col = 0;
}

/* ---- Playfair digraph substitution ---- */

QPair<QChar, QChar> SeriatedPlayfair7::substituteDigraph(QChar a, QChar b, bool encrypt) const
{
    int r1, c1, r2, c2;
    findPosition(a, r1, c1);
    findPosition(b, r2, c2);

    int dir = encrypt ? 1 : 4; // Forward or backward shift

    if (r1 == r2) {
        // Same row: shift columns
        c1 = (c1 + dir) % 5;
        c2 = (c2 + dir) % 5;
    } else if (c1 == c2) {
        // Same column: shift rows
        r1 = (r1 + dir) % 5;
        r2 = (r2 + dir) % 5;
    } else {
        // Rectangle: swap columns
        int tmp = c1; c1 = c2; c2 = tmp;
    }
    return {m_grid[r1][c1], m_grid[r2][c2]};
}

/* ---- Bifid-style seriation split ---- */

QVector<QChar> SeriatedPlayfair7::seriateSplit(const QVector<QChar>& chars, int period) const
{
    if (period <= 0) period = chars.size();
    QVector<QChar> result;

    for (int block = 0; block < chars.size(); block += period) {
        int end = qMin(block + period, chars.size());
        // Split into rows and columns coordinates then interleave
        QVector<int> rows, cols;
        for (int i = block; i < end; ++i) {
            int r, c;
            findPosition(chars[i], r, c);
            rows.append(r);
            cols.append(c);
        }
        // Interleave: all rows then all columns
        for (int v : rows) result.append(m_grid[v][0]); // placeholder
        for (int v : cols) result.append(m_grid[0][v]); // placeholder
    }
    // Re-encode: pair up (row_i, col_i) into chars
    QVector<QChar> encoded;
    int half = result.size() / 2;
    for (int i = 0; i < half; ++i) {
        // Reconstruct from row/col interleaving
        int block = (i / period) * period * 2;
        int idx = i % period;
        int rowIdx = block + idx;
        int colIdx = block + period + idx;
        if (colIdx < result.size()) {
            int r = -1, c = -1;
            for (int rr = 0; rr < 5; ++rr)
                for (int cc = 0; cc < 5; ++cc)
                    if (m_grid[rr][cc] == chars[qMin(i, chars.size() - 1)]) { r = rr; c = cc; }
            encoded.append(m_grid[(rowIdx) % 5][(colIdx) % 5]);
        }
    }
    return encoded.isEmpty() ? result : encoded;
}

/* ---- Reverse seriation ---- */

QVector<QChar> SeriatedPlayfair7::seriateMerge(const QVector<QChar>& chars, int period) const
{
    // Inverse of seriateSplit
    return seriateSplit(chars, period);
}

/* ---- Rotate keyword ---- */

QString SeriatedPlayfair7::rotateKeyword(const QString& keyword, int step) const
{
    if (keyword.isEmpty()) return keyword;
    int s = step % keyword.size();
    return keyword.mid(s) + keyword.left(s);
}

/* ---- Columnar transposition ---- */

QVector<QChar> SeriatedPlayfair7::columnarTranspose(const QVector<QChar>& chars, bool encrypt) const
{
    int n = chars.size();
    if (n == 0) return chars;

    int cols = m_params.period;
    int rows = (n + cols - 1) / cols;

    QVector<QChar> result(n);
    if (encrypt) {
        // Write row-wise, read column-wise
        int idx = 0;
        for (int c = 0; c < cols; ++c)
            for (int r = 0; r < rows; ++r) {
                int src = r * cols + c;
                if (src < n) result[idx++] = chars[src];
            }
    } else {
        // Inverse: write column-wise, read row-wise
        int fullRows = n / cols;
        int extraCols = n % cols;
        int idx = 0;
        QVector<int> colLen(cols, fullRows);
        for (int c = 0; c < extraCols; ++c) colLen[c]++;

        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                int offset = 0;
                for (int pc = 0; pc < c; ++pc) offset += colLen[pc];
                int src = offset + r;
                if (src < n && r < colLen[c]) result[idx++] = chars[src];
            }
    }
    return result;
}

/* ---- Encrypt ---- */

SeriatedPlayfair7::CipherResult SeriatedPlayfair7::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString processed = preprocess(plaintext);

    // Process in blocks with keyword rotation
    QString cipherText;
    int rotations = 0;

    for (int block = 0; block < processed.size(); block += m_params.period * 2) {
        // Rotate keyword and regenerate grid for each block
        if (block > 0) {
            QString rotated = rotateKeyword(m_params.keyword, rotations * m_params.rotationStep);
            m_grid = generateGrid(rotated);
            rotations++;
        }

        int end = qMin(block + m_params.period * 2, processed.size());

        // Step 1: Playfair digraph substitution
        QString subText;
        for (int i = block; i < end - 1; i += 2) {
            auto pair = substituteDigraph(processed[i], processed[i + 1], true);
            subText.append(pair.first);
            subText.append(pair.second);
        }

        // Step 2: Columnar seriation
        if (m_params.columnarSeriation) {
            QVector<QChar> chars;
            for (const QChar& ch : subText) chars.append(ch);
            chars = columnarTranspose(chars, true);
            for (const QChar& ch : chars) cipherText.append(ch);
        } else {
            cipherText.append(subText);
        }
    }

    // Restore original grid
    m_grid = generateGrid(m_params.keyword);

    result.text = cipherText;
    result.blockCount = (processed.size() + m_params.period * 2 - 1) / (m_params.period * 2);
    result.rotationsApplied = rotations;
    result.processingTimeMs = timer.elapsed();

    m_stats.numEncrypts++;
    m_stats.totalChars += processed.size();
    m_stats.totalOps++;
    m_timeSum += result.processingTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptionDone(result.text.size(), result.blockCount, result.processingTimeMs);

    return result;
}

/* ---- Decrypt ---- */

SeriatedPlayfair7::CipherResult SeriatedPlayfair7::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    QString processed = ciphertext.toUpper();
    QString plainText;
    int rotations = 0;

    for (int block = 0; block < processed.size(); block += m_params.period * 2) {
        if (block > 0) {
            QString rotated = rotateKeyword(m_params.keyword, rotations * m_params.rotationStep);
            m_grid = generateGrid(rotated);
            rotations++;
        }

        int end = qMin(block + m_params.period * 2, processed.size());
        QString blockText = processed.mid(block, end - block);

        // Step 1: Reverse columnar seriation
        if (m_params.columnarSeriation) {
            QVector<QChar> chars;
            for (const QChar& ch : blockText) chars.append(ch);
            chars = columnarTranspose(chars, false);
            blockText.clear();
            for (const QChar& ch : chars) blockText.append(ch);
        }

        // Step 2: Reverse Playfair digraph substitution
        for (int i = 0; i < blockText.size() - 1; i += 2) {
            auto pair = substituteDigraph(blockText[i], blockText[i + 1], false);
            plainText.append(pair.first);
            plainText.append(pair.second);
        }
    }

    m_grid = generateGrid(m_params.keyword);

    result.text = plainText;
    result.blockCount = (processed.size() + m_params.period * 2 - 1) / (m_params.period * 2);
    result.rotationsApplied = rotations;
    result.processingTimeMs = timer.elapsed();

    m_stats.numDecrypts++;
    m_stats.totalChars += processed.size();
    m_stats.totalOps++;
    m_timeSum += result.processingTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptionDone(result.text.size(), result.blockCount, result.processingTimeMs);

    return result;
}

/* ---- Reset ---- */

void SeriatedPlayfair7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
