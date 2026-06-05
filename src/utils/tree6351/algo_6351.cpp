/**
 * @file algo_6351.cpp
 */
#include "tree6351/algo_6351.h"
QVector<double> algo_6351::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
