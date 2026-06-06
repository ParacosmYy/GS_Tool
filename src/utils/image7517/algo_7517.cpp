/**
 * @file algo_7517.cpp
 */
#include "image7517/algo_7517.h"
QVector<double> algo_7517::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
