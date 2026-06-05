/**
 * @file NgramAnalyzer.cpp
 * @brief N-gram分析器实现
 */

#include "utils/ngram/NgramAnalyzer.h"

#include <QElapsedTimer>
#include <algorithm>

NgramAnalyzer::NgramAnalyzer(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QMap<QByteArray, int> NgramAnalyzer::analyzeBytes(const QByteArray& data, int n)
{
    QMap<QByteArray, int> freq;
    if (n < 1 || data.size() < n) return freq;

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i <= data.size() - n; ++i) {
        QByteArray ngram = data.mid(i, n);
        ++freq[ngram];
    }

    m_stats.totalAnalyses++;
    m_stats.totalNgramsExtracted += data.size() - n + 1;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(freq.size(), data.size() - n + 1);
    return freq;
}

QMap<QString, int> NgramAnalyzer::analyzeText(const QString& text, int n)
{
    QMap<QString, int> freq;
    if (n < 1 || text.length() < n) return freq;

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i <= text.length() - n; ++i) {
        QString ngram = text.mid(i, n);
        ++freq[ngram];
    }

    m_stats.totalAnalyses++;
    m_stats.totalNgramsExtracted += text.length() - n + 1;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(freq.size(), text.length() - n + 1);
    return freq;
}

QMap<QByteArray, double> NgramAnalyzer::byteProbabilities(const QByteArray& data,
                                                            int n)
{
    QMap<QByteArray, double> probs;
    QMap<QByteArray, int> freq = analyzeBytes(data, n);

    int total = 0;
    for (auto it = freq.constBegin(); it != freq.constEnd(); ++it) total += it.value();

    if (total > 0) {
        for (auto it = freq.constBegin(); it != freq.constEnd(); ++it) {
            probs[it.key()] = static_cast<double>(it.value()) / total;
        }
    }
    return probs;
}

QVector<QPair<QByteArray, int>> NgramAnalyzer::topK(const QByteArray& data,
                                                      int n, int k)
{
    QMap<QByteArray, int> freq = analyzeBytes(data, n);

    QVector<QPair<QByteArray, int>> sorted;
    sorted.reserve(freq.size());
    for (auto it = freq.constBegin(); it != freq.constEnd(); ++it) {
        sorted.append({it.key(), it.value()});
    }

    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (sorted.size() > k) sorted.resize(k);
    return sorted;
}

void NgramAnalyzer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
