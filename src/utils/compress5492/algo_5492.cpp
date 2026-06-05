/**
 * @file algo_5492.cpp
 */
#include "compress5492/algo_5492.h"
QVector<double> algo_5492::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
