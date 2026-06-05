/**
 * @file algo_6398.cpp
 */
#include "neural6398/algo_6398.h"
QVector<double> algo_6398::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
