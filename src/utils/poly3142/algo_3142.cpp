/**
 * @file algo_3142.cpp
 */
#include "poly3142/algo_3142.h"
QVector<double> algo_3142::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
