/**
 * @file algo_6201.cpp
 */
#include "interp6201/algo_6201.h"
QVector<double> algo_6201::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
