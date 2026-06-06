/**
 * @file algo_6979.cpp
 */
#include "quantum6979/algo_6979.h"
QVector<double> algo_6979::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
