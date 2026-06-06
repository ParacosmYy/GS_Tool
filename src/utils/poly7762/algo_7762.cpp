/**
 * @file algo_7762.cpp
 */
#include "poly7762/algo_7762.h"
QVector<double> algo_7762::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
