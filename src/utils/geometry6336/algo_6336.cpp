/**
 * @file algo_6336.cpp
 */
#include "geometry6336/algo_6336.h"
QVector<double> algo_6336::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
