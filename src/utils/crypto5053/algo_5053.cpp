/**
 * @file algo_5053.cpp
 */
#include "crypto5053/algo_5053.h"
QVector<double> algo_5053::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
