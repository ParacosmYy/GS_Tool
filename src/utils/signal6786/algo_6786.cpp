/**
 * @file algo_6786.cpp
 */
#include "signal6786/algo_6786.h"
QVector<double> algo_6786::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
