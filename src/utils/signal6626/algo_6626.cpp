/**
 * @file algo_6626.cpp
 */
#include "signal6626/algo_6626.h"
QVector<double> algo_6626::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
