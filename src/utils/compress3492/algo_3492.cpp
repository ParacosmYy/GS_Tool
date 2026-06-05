/**
 * @file algo_3492.cpp
 */
#include "compress3492/algo_3492.h"
QVector<double> algo_3492::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
