/**
 * @file algo_6521.cpp
 */
#include "interp6521/algo_6521.h"
QVector<double> algo_6521::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
