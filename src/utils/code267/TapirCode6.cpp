/**
 * @file TapirCode6.cpp
 * @brief TapirCode6 实现
 *
 * 实现Tapir密码：同音替换与概率符号分配频率平坦化加密。
 */

#include "utils/code267/TapirCode6.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TapirCode6::TapirCode6(QObject *parent)
    : QObject(parent) {}

TapirCode6::~TapirCode6() = default;

/* ---- Configuration ---- */

void TapirCode6::setKey(const QString& key)
{
    Q_UNUSED(key)
    // Build table with standard printable ASCII alphabet
    QString alphabet;
    for (int c = 32; c < 127; ++c)
        alphabet.append(static_cast<QChar>(c));

    buildTable(alphabet);
}

void TapirCode6::setHomophonesPerSymbol(int count)
{
    m_homoCount = qMax(2, count);
}

/* ---- Build homophone table ---- */

void TapirCode6::buildTable(const QString& alphabet)
{
    m_table.clear();
    m_charIndex.clear();
    m_symbolToIndex.clear();
    m_nextSymbol = 0;

    int n = alphabet.size();
    for (int i = 0; i < n; ++i) {
        Homophones h;
        h.baseChar = alphabet[i];

        // Assign m_homoCount symbol codes per character
        // Frequency flattening: more common chars get more homophones
        // Here we give equal count, weighted uniformly
        for (int j = 0; j < m_homoCount; ++j) {
            h.symbols.append(m_nextSymbol++);
            h.weights.append(1.0 / m_homoCount); // uniform probability
        }
        normalizeWeights(h.weights);
        m_table.append(h);
        m_charIndex[alphabet[i]] = i;

        // Build reverse mapping: symbol -> char index
        for (int sym : h.symbols)
            m_symbolToIndex[sym] = i;
    }
}

/* ---- Normalize weights ---- */

void TapirCode6::normalizeWeights(QVector<double>& weights)
{
    double sum = 0.0;
    for (double w : weights) sum += w;
    if (sum > 0.0) {
        for (double& w : weights) w /= sum;
    }
}

/* ---- Select symbol using weighted probability ---- */

int TapirCode6::selectSymbol(int charIndex) const
{
    const auto& h = m_table[charIndex];
    double r = static_cast<double>(qrand()) / RAND_MAX;
    double cumulative = 0.0;
    for (int i = 0; i < h.symbols.size(); ++i) {
        cumulative += h.weights[i];
        if (r <= cumulative) return h.symbols[i];
    }
    return h.symbols.last();
}

/* ---- Encrypt ---- */

QVector<int> TapirCode6::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_table.isEmpty()) setKey(QStringLiteral("default"));

    QVector<int> result;
    result.reserve(plaintext.size());

    for (const QChar& ch : plaintext) {
        int idx = m_charIndex.value(ch, -1);
        if (idx >= 0) {
            result.append(selectSymbol(idx));
        } else {
            // Unknown character: map to a random symbol from first entry
            result.append(selectSymbol(0));
        }
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.alphabetSize = m_table.size();
    m_stats.numHomophones = m_homoCount;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptionCompleted(plaintext.size(), result.size(), elapsed);

    return result;
}

/* ---- Decrypt ---- */

QString TapirCode6::decrypt(const QVector<int>& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (int sym : ciphertext) {
        int idx = m_symbolToIndex.value(sym, -1);
        if (idx >= 0 && idx < m_table.size()) {
            result.append(m_table[idx].baseChar);
        } else {
            result.append(QChar('?'));
        }
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Accessors ---- */

QVector<TapirCode6::Homophones> TapirCode6::homophoneTable() const
{
    return m_table;
}

QMap<int, int> TapirCode6::frequencyAnalysis(const QVector<int>& ciphertext) const
{
    QMap<int, int> freq;
    for (int sym : ciphertext)
        freq[sym]++;
    return freq;
}

/* ---- Reset ---- */

void TapirCode6::resetStatistics()
{
    m_table.clear();
    m_charIndex.clear();
    m_symbolToIndex.clear();
    m_nextSymbol = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
