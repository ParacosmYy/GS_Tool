/**
 * @file algo_3178.cpp
 */
#include "neural3178/algo_3178.h"
QVector<double> algo_3178::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
