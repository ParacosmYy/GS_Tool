/**
 * @file algo_7202.cpp
 */
#include "poly7202/algo_7202.h"
QVector<double> algo_7202::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
