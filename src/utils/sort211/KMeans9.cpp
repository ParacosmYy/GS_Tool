/**
 * @file KMeans9.cpp
 * @brief KMeans9 implementation
 */
#include "sort211/KMeans9.h"
#include <QElapsedTimer>
QVector<double> KMeans9::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

