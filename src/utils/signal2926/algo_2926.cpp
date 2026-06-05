/**
 * @file algo_2926.cpp
 */
#include "signal2926/algo_2926.h"
QVector<double> algo_2926::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
