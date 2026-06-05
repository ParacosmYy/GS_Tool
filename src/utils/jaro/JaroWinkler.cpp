/**
 * @file JaroWinkler.cpp
 * @brief Jaro-Winkler相似度实现
 */

#include "utils/jaro/JaroWinkler.h"

#include <QElapsedTimer>
#include <QVector>

JaroWinkler::JaroWinkler(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double JaroWinkler::jaro(const QString& s1, const QString& s2)
{
    QElapsedTimer timer;
    timer.start();

    if (s1 == s2) return 1.0;
    int len1 = s1.length(), len2 = s2.length();
    if (len1 == 0 || len2 == 0) return 0.0;

    int matchDist = qMax(len1, len2) / 2 - 1;
    if (matchDist < 0) matchDist = 0;

    QVector<bool> s1Matched(len1, false), s2Matched(len2, false);
    int matches = 0, transpositions = 0;

    for (int i = 0; i < len1; ++i) {
        int start = qMax(0, i - matchDist);
        int end = qMin(i + matchDist + 1, len2);
        for (int j = start; j < end; ++j) {
            if (s2Matched[j] || s1[i] != s2[j]) continue;
            s1Matched[i] = true;
            s2Matched[j] = true;
            ++matches;
            break;
        }
    }

    if (matches == 0) return 0.0;

    int k = 0;
    for (int i = 0; i < len1; ++i) {
        if (!s1Matched[i]) continue;
        while (!s2Matched[k]) ++k;
        if (s1[i] != s2[k]) ++transpositions;
        ++k;
    }

    double jaro = (static_cast<double>(matches) / len1 +
                   static_cast<double>(matches) / len2 +
                   (matches - transpositions / 2.0) / matches) / 3.0;

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(jaro);
    return jaro;
}

double JaroWinkler::jaroWinkler(const QString& s1, const QString& s2, double p)
{
    double jaroSim = jaro(s1, s2);

    int prefix = 0;
    int maxPrefix = qMin(4, qMin(s1.length(), s2.length()));
    for (int i = 0; i < maxPrefix; ++i) {
        if (s1[i] == s2[i]) ++prefix;
        else break;
    }

    return jaroSim + prefix * p * (1.0 - jaroSim);
}

void JaroWinkler::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
