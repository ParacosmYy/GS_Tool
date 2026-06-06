/**
 * @file algo_7096.cpp
 */
#include "geometry7096/algo_7096.h"
QVector<double> algo_7096::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
