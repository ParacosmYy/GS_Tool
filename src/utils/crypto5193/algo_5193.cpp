/**
 * @file algo_5193.cpp
 */
#include "crypto5193/algo_5193.h"
QVector<double> algo_5193::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
