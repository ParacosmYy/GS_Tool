/**
 * @file DigrafidCode2.cpp
 * @brief DigrafidCode2 实现
 *
 * 实现Digrafid密码：双图分数化、3×3×3坐标映射、双密钥列置换。
 */

#include "utils/code212/DigrafidCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DigrafidCode2::DigrafidCode2(QObject *parent) : QObject(parent) {}
DigrafidCode2::~DigrafidCode2() = default;

/* ---- Configuration ---- */

void DigrafidCode2::setKeys(const QString& horizontalKey,
                             const QString& verticalKey, int period)
{
    m_hKey = horizontalKey.toUpper().left(27);
    m_vKey = verticalKey.toUpper().left(27);
    m_period = qMax(1, period);
}

/* ---- Normalize text ---- */

QString DigrafidCode2::normalize(const QString& text)
{
    QString result;
    result.reserve(text.size());
    for (QChar c : text.toUpper()) {
        if (c == QLatin1Char('J'))
            result += QLatin1Char('I');
        else if (c.isLetter())
            result += c;
    }
    return result;
}

/* ---- Key position: find char in key, return (row, col, layer) in 3x3x3 ---- */

QVector<int> DigrafidCode2::keyPosition(QChar c, const QString& key)
{
    int idx = key.indexOf(c.toUpper());
    if (idx < 0) idx = 0;
    QVector<int> coord(3);
    coord[0] = idx / 9;           // Layer (0-2)
    coord[1] = (idx % 9) / 3;    // Row (0-2)
    coord[2] = idx % 3;          // Col (0-2)
    return coord;
}

/* ---- Fractionate a digraph ---- */

QPair<int, int> DigrafidCode2::fractionate(QChar a, QChar b) const
{
    auto posA = keyPosition(a, m_hKey);
    auto posB = keyPosition(b, m_vKey);
    // Combine: horizontal trit from a's layer and row
    int hTrit = posA[0] * 3 + posA[1];
    // Vertical trit from b's layer and col
    int vTrit = posB[0] * 3 + posB[2];
    return {hTrit, vTrit};
}

/* ---- Defractionate ---- */

QPair<QChar, QChar> DigrafidCode2::defractionate(int h, int v) const
{
    int hLayer = h / 3;
    int hRow = h % 3;
    int vLayer = v / 3;
    int vCol = v % 3;

    // Find char in horizontal key at (hLayer, hRow, any col) -> use col from v
    int hIdx = hLayer * 9 + hRow * 3 + vCol;
    // Find char in vertical key at (vLayer, any row, vCol) -> use row from h
    int vIdx = vLayer * 9 + hRow * 3 + vCol;

    QChar ca = (hIdx < m_hKey.size()) ? m_hKey[hIdx] : QLatin1Char('A');
    QChar cb = (vIdx < m_vKey.size()) ? m_vKey[vIdx] : QLatin1Char('A');
    return {ca, cb};
}

/* ---- Columnar transposition ---- */

QVector<int> DigrafidCode2::columnarTranspose(const QVector<int>& trits,
                                               const QString& key,
                                               bool encrypt) const
{
    int n = trits.size();
    if (n == 0) return {};

    int cols = qMin(key.size(), 27);
    if (cols <= 0) cols = 7;
    int rows = qCeil(static_cast<double>(n) / cols);

    // Build column order from key
    QVector<QPair<QChar, int>> keyOrder;
    keyOrder.reserve(cols);
    for (int i = 0; i < cols; ++i)
        keyOrder.append({key[i], i});
    std::sort(keyOrder.begin(), keyOrder.end());
    QVector<int> colPerm(cols);
    for (int i = 0; i < cols; ++i)
        colPerm[i] = keyOrder[i].second;

    QVector<int> result;
    result.reserve(n);

    if (encrypt) {
        // Read off by columns in key order
        for (int c = 0; c < cols; ++c) {
            int col = colPerm[c];
            for (int r = 0; r < rows; ++r) {
                int idx = r * cols + col;
                if (idx < n) result.append(trits[idx]);
            }
        }
    } else {
        // Inverse: distribute back
        QVector<int> temp(n, 0);
        int totalFull = n / cols;
        int extra = n % cols;

        int pos = 0;
        for (int c = 0; c < cols; ++c) {
            int col = colPerm[c];
            int colRows = totalFull + (col < extra ? 1 : 0);
            for (int r = 0; r < colRows; ++r) {
                int idx = r * cols + col;
                if (idx < n && pos < n)
                    temp[idx] = trits[pos++];
            }
        }
        result = temp;
    }
    return result;
}

/* ---- Encrypt ---- */

QString DigrafidCode2::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString text = normalize(plaintext);
    // Pad to even length
    if (text.size() % 2 != 0)
        text += QLatin1Char('X');

    int n = text.size();
    QVector<int> hTrits, vTrits;
    hTrits.reserve(n / 2);
    vTrits.reserve(n / 2);

    for (int i = 0; i < n; i += 2) {
        auto [h, v] = fractionate(text[i], text[i + 1]);
        hTrits.append(h);
        vTrits.append(v);
    }

    // Combine trits into single sequence: h0,h1,...,v0,v1,...
    QVector<int> allTrits;
    allTrits.reserve(hTrits.size() + vTrits.size());
    allTrits.append(hTrits);
    allTrits.append(vTrits);

    // Columnar transposition
    auto transposed = columnarTranspose(allTrits, m_hKey, true);

    // Defractionate pairs back
    QString result;
    int half = transposed.size() / 2;
    for (int i = 0; i < half; ++i) {
        auto [a, b] = defractionate(transposed[i], transposed[i + half]);
        result += a;
        result += b;
    }

    auto self = const_cast<DigrafidCode2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.inputLength = text.size();
    self->m_stats.outputLength = result.size();
    self->m_stats.period = m_period;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit cipherCompleted(text.size(), result.size(), timer.elapsed());

    return result;
}

/* ---- Decrypt ---- */

QString DigrafidCode2::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString text = normalize(ciphertext);
    int n = text.size();
    if (n % 2 != 0) n--;

    // Fractionate ciphertext pairs
    QVector<int> hTrits, vTrits;
    hTrits.reserve(n / 2);
    vTrits.reserve(n / 2);
    for (int i = 0; i < n; i += 2) {
        auto [h, v] = fractionate(text[i], text[i + 1]);
        hTrits.append(h);
        vTrits.append(v);
    }

    QVector<int> allTrits;
    allTrits.reserve(hTrits.size() + vTrits.size());
    allTrits.append(hTrits);
    allTrits.append(vTrits);

    // Inverse columnar transposition
    auto untransposed = columnarTranspose(allTrits, m_hKey, false);

    int half = untransposed.size() / 2;
    QString result;
    for (int i = 0; i < half; ++i) {
        auto [a, b] = defractionate(untransposed[i], untransposed[i + half]);
        result += a;
        result += b;
    }

    auto self = const_cast<DigrafidCode2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.inputLength = ciphertext.size();
    self->m_stats.outputLength = result.size();
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit cipherCompleted(ciphertext.size(), result.size(), timer.elapsed());

    return result;
}

/* ---- Reset ---- */

void DigrafidCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
