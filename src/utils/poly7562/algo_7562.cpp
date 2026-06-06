/**
 * @file algo_7562.cpp
 */
#include "poly7562/algo_7562.h"
QVector<double> algo_7562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
