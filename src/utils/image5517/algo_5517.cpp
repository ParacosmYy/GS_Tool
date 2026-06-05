/**
 * @file algo_5517.cpp
 */
#include "image5517/algo_5517.h"
QVector<double> algo_5517::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
