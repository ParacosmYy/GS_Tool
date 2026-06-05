/**
 * @file algo_4859.cpp
 */
#include "quantum4859/algo_4859.h"
QVector<double> algo_4859::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
