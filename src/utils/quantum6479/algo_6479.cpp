/**
 * @file algo_6479.cpp
 */
#include "quantum6479/algo_6479.h"
QVector<double> algo_6479::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
