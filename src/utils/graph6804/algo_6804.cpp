/**
 * @file algo_6804.cpp
 */
#include "graph6804/algo_6804.h"
QVector<double> algo_6804::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
