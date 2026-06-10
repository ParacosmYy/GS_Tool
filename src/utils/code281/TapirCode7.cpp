/**
 * @file TapirCode7.cpp
 * @brief TapirCode7 实现
 *
 * 实现Tapir密码：扩展同音表与上下文依赖符号选择的频率均衡加密。
 */

#include "utils/code281/TapirCode7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

TapirCode7::TapirCode7(QObject *parent)
    : QObject(parent) {}

TapirCode7::~TapirCode7() = default;

/* ---- Configuration ---- */

void TapirCode7::setAlphabetSize(int size) { m_alphabetSize = qMax(2, size); }
void TapirCode7::setExpansionFactor(int factor) { m_expansion = qBound(2, factor, 16); }

/* ---- Context mask from previous symbol ---- */

quint32 TapirCode7::contextMask(int prevSymbol) const
{
    // Generate a deterministic hash from previous symbol for context-dependent selection
    quint32 mask = static_cast<quint32>(prevSymbol * 2654435761u);  // Knuth multiplicative hash
    mask ^= mask >> 16;
    return mask;
}

/* ---- Build homophone table from frequency map ---- */

void TapirCode7::buildTable(const QMap<QChar, double>& freqMap)
{
    QElapsedTimer timer;
    timer.start();

    m_homophoneTable.clear();
    m_reverseTable.clear();
    m_contextCounter.clear();
    m_nextSymbolId = 0;

    // Compute total frequency for normalization
    double totalFreq = 0.0;
    for (auto it = freqMap.begin(); it != freqMap.end(); ++it)
        totalFreq += it.value();

    if (totalFreq <= 0.0) return;

    // Allocate homophones proportional to frequency
    int totalSlots = freqMap.size() * m_expansion;
    for (auto it = freqMap.begin(); it != freqMap.end(); ++it) {
        QChar ch = it.key();
        double relFreq = it.value() / totalFreq;
        int numHomophones = qMax(1, static_cast<int>(qRound(relFreq * totalSlots)));

        QVector<QPair<int, quint32>> homophones;
        for (int h = 0; h < numHomophones; ++h) {
            int symbolId = m_nextSymbolId++;
            // Assign a context mask for each homophone entry
            quint32 mask = static_cast<quint32>(symbolId * 2246822519u);
            homophones.append({symbolId, mask});
            m_reverseTable[symbolId] = ch;
        }
        m_homophoneTable[ch] = homophones;
        m_contextCounter[ch] = 0;
    }

    double elapsed = timer.elapsed();
    m_stats.tableSize = m_nextSymbolId;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tableBuilt(m_alphabetSize, m_nextSymbolId, elapsed);
}

/* ---- Select homophone based on context ---- */

int TapirCode7::selectHomophone(QChar ch, int prevSymbol)
{
    auto it = m_homophoneTable.find(ch);
    if (it == m_homophoneTable.end()) {
        // Fallback: map unknown to 'X' or use first available
        if (!m_homophoneTable.isEmpty())
            it = m_homophoneTable.begin();
        else
            return 0;
    }

    QVector<QPair<int, quint32>>& homophones = it.value();
    int count = homophones.size();
    if (count == 0) return 0;

    // Context-dependent selection: use previous symbol to influence choice
    quint32 ctx = contextMask(prevSymbol);
    int counter = m_contextCounter[ch];

    // Combine context hash with cycling counter for deterministic but varied selection
    int idx = static_cast<int>((ctx + static_cast<quint32>(counter)) % static_cast<quint32>(count));
    m_contextCounter[ch] = counter + 1;

    return homophones[idx].first;
}

/* ---- Encrypt plaintext ---- */

TapirCode7::EncResult TapirCode7::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    EncResult result;
    int len = plaintext.size();
    if (len == 0) return result;

    int prevSymbol = 0;  // Initial context

    for (int i = 0; i < len; ++i) {
        QChar ch = plaintext[i].toUpper();
        int symbol = selectHomophone(ch, prevSymbol);
        result.cipherSymbols.append(symbol);
        prevSymbol = symbol;
    }

    result.frequencyVariance = computeVariance(result.cipherSymbols);

    double elapsed = timer.elapsed();
    m_stats.numSymbols = len;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptDone(len, result.cipherSymbols.size(), result.frequencyVariance, elapsed);

    return result;
}

/* ---- Decrypt cipher symbols ---- */

QString TapirCode7::decrypt(const QVector<int>& cipherSymbols)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (int symbol : cipherSymbols) {
        auto it = m_reverseTable.find(symbol);
        if (it != m_reverseTable.end())
            result.append(it.value());
        else
            result.append(QChar('?'));
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Analyze cipher symbol frequency ---- */

QMap<int, double> TapirCode7::analyzeCipherFrequency(const QVector<int>& cipherSymbols) const
{
    QMap<int, int> counts;
    int total = cipherSymbols.size();
    for (int sym : cipherSymbols)
        counts[sym]++;

    QMap<int, double> freq;
    for (auto it = counts.begin(); it != counts.end(); ++it)
        freq[it.key()] = (total > 0) ? static_cast<double>(it.value()) / total : 0.0;

    return freq;
}

/* ---- Compute frequency variance ---- */

double TapirCode7::computeVariance(const QVector<int>& symbols) const
{
    if (symbols.isEmpty()) return 0.0;

    QMap<int, int> counts;
    for (int s : symbols) counts[s]++;

    int n = symbols.size();
    double expectedFreq = 1.0 / counts.size();

    double variance = 0.0;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        double observed = static_cast<double>(it.value()) / n;
        double diff = observed - expectedFreq;
        variance += diff * diff;
    }
    return variance / counts.size();
}

/* ---- Reset ---- */

void TapirCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_homophoneTable.clear();
    m_reverseTable.clear();
    m_contextCounter.clear();
    m_nextSymbolId = 0;
}
