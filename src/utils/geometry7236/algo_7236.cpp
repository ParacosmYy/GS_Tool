/**
 * @file algo_7236.cpp
 */
#include "geometry7236/algo_7236.h"
QVector<double> algo_7236::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
