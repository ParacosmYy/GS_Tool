/**
 * @file algo_5201.cpp
 */
#include "interp5201/algo_5201.h"
QVector<double> algo_5201::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
