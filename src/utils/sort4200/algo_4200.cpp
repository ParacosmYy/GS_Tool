/**
 * @file algo_4200.cpp
 */
#include "sort4200/algo_4200.h"
QVector<double> algo_4200::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
