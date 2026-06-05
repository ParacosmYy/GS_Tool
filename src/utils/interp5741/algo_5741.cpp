/**
 * @file algo_5741.cpp
 */
#include "interp5741/algo_5741.h"
QVector<double> algo_5741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
