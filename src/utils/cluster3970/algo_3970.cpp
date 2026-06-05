/**
 * @file algo_3970.cpp
 */
#include "cluster3970/algo_3970.h"
QVector<double> algo_3970::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
