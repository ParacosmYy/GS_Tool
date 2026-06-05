/**
 * @file algo_3443.cpp
 */
#include "string3443/algo_3443.h"
QVector<double> algo_3443::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
