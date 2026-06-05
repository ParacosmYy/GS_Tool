/**
 * @file algo_5357.cpp
 */
#include "image5357/algo_5357.h"
QVector<double> algo_5357::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
