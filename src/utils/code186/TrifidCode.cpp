/**
 * @file TrifidCode.cpp
 * @brief TrifidCode 实现
 *
 * 实现三分密码：3x3x3立方体映射、层内分数编码、层转位混淆。
 */

#include "utils/code186/TrifidCode.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TrifidCode::TrifidCode(QObject *parent) : QObject(parent)
{
    buildCube("");
}

TrifidCode::~TrifidCode() = default;

/* ---- Configuration ---- */

void TrifidCode::setKey(const QString& key)
{
    buildCube(key);
}

void TrifidCode::setPeriod(int period) { m_period = qMax(1, period); }

/* ---- Build 3x3x3 cube from key ---- */

void TrifidCode::buildCube(const QString& key)
{
    m_encodeMap.clear();
    m_decodeMap.clear();

    // Deduplicated key chars followed by remaining alphabet
    QString used;
    QString combined = key.toUpper() + m_alphabet;
    for (QChar c : combined) {
        if (!used.contains(c) && m_alphabet.contains(c))
            used.append(c);
    }

    // Fill 3x3x3 = 27 cells (pad if needed)
    while (used.size() < 27)
        used.append('#');

    int idx = 0;
    for (int layer = 0; layer < 3; ++layer) {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                if (idx < used.size()) {
                    QChar c = used[idx];
                    m_encodeMap[c] = {layer, row, col};
                    QString key = QString("%1,%2,%3").arg(layer).arg(row).arg(col);
                    m_decodeMap[key] = c;
                }
                ++idx;
            }
        }
    }
}

/* ---- Encode char → (layer, row, col) ---- */

QVector<int> TrifidCode::encodeChar(QChar c) const
{
    c = c.toUpper();
    if (m_encodeMap.contains(c))
        return m_encodeMap[c];
    return {0, 0, 0}; // Fallback for unknown chars
}

/* ---- Decode (layer, row, col) → char ---- */

QChar TrifidCode::decodeCoord(int layer, int row, int col) const
{
    QString key = QString("%1,%2,%3").arg(layer).arg(row).arg(col);
    return m_decodeMap.value(key, '?');
}

/* ---- Preprocess ---- */

QString TrifidCode::preprocess(const QString& text) const
{
    QString result;
    for (QChar c : text.toUpper()) {
        if (m_alphabet.contains(c))
            result.append(c);
    }
    return result;
}

/* ---- Layer transposition (encrypt) ---- */

QVector<int> TrifidCode::transpose(const QVector<int>& coords, int period) const
{
    // coords contains 3*n values: [layer0,row0,col0, layer1,row1,col1, ...]
    int n = coords.size() / 3;
    if (n == 0) return coords;

    QVector<int> result;
    // Process in blocks of 'period' characters
    int pos = 0;
    while (pos < n) {
        int blockLen = qMin(period, n - pos);
        // Collect layers, rows, cols for this block
        QVector<int> layers, rows, cols;
        for (int i = 0; i < blockLen; ++i) {
            layers.append(coords[(pos + i) * 3 + 0]);
            rows.append(coords[(pos + i) * 3 + 1]);
            cols.append(coords[(pos + i) * 3 + 2]);
        }
        // Concatenate: layers || rows || cols (read down columns)
        QVector<int> combined;
        combined.append(layers);
        combined.append(rows);
        combined.append(cols);
        // Read off in groups of 3 to get new (layer, row, col)
        for (int i = 0; i < combined.size() - 2; i += 3)
            result.append({combined[i], combined[i + 1], combined[i + 2]});

        pos += blockLen;
    }
    return result;
}

/* ---- Inverse transposition (decrypt) ---- */

QVector<int> TrifidCode::inverseTranspose(const QVector<int>& coords, int period) const
{
    int n = coords.size() / 3;
    if (n == 0) return coords;

    QVector<int> result;
    int pos = 0;
    while (pos < n) {
        int blockLen = qMin(period, n - pos);
        // The transposed block has 3*blockLen values
        // Read the combined stream
        QVector<int> combined;
        for (int i = 0; i < blockLen * 3; ++i)
            combined.append(coords[(pos * 3) + i]);

        // Split into three equal parts: layers, rows, cols
        int partLen = blockLen;
        QVector<int> layers, rows, cols;
        for (int i = 0; i < partLen; ++i) layers.append(combined[i]);
        for (int i = partLen; i < 2 * partLen; ++i) rows.append(combined[i]);
        for (int i = 2 * partLen; i < 3 * partLen; ++i) cols.append(combined[i]);

        // Reconstruct original coords
        for (int i = 0; i < blockLen; ++i) {
            result.append({layers[i], rows[i], cols[i]});
        }
        pos += blockLen;
    }
    return result;
}

/* ---- Encrypt ---- */

QString TrifidCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString processed = preprocess(plaintext);
    if (processed.isEmpty()) return {};

    // Encode all chars to coordinate triples
    QVector<int> coords;
    for (QChar c : processed) {
        auto triple = encodeChar(c);
        coords.append(triple[0]);
        coords.append(triple[1]);
        coords.append(triple[2]);
    }

    // Transpose
    auto transposed = transpose(coords, m_period);

    // Decode back to chars
    QString ciphertext;
    for (int i = 0; i < transposed.size() - 2; i += 3)
        ciphertext.append(decodeCoord(transposed[i], transposed[i + 1], transposed[i + 2]));

    m_stats.totalOperations++;
    m_stats.period = m_period;
    m_stats.inputLength = processed.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("encrypt", processed.size(), timer.elapsed());
    return ciphertext;
}

/* ---- Decrypt ---- */

QString TrifidCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString processed = preprocess(ciphertext);
    if (processed.isEmpty()) return {};

    // Encode to coordinates
    QVector<int> coords;
    for (QChar c : processed) {
        auto triple = encodeChar(c);
        coords.append(triple[0]);
        coords.append(triple[1]);
        coords.append(triple[2]);
    }

    // Inverse transpose
    auto inversed = inverseTranspose(coords, m_period);

    // Decode back
    QString plaintext;
    for (int i = 0; i < inversed.size() - 2; i += 3)
        plaintext.append(decodeCoord(inversed[i], inversed[i + 1], inversed[i + 2]));

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("decrypt", processed.size(), timer.elapsed());
    return plaintext;
}

/* ---- Reset ---- */

void TrifidCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
