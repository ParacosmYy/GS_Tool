/**
 * @file algo_4626.cpp
 */
#include "signal4626/algo_4626.h"
QVector<double> algo_4626::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
