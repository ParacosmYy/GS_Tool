/**
 * @file algo_3423.cpp
 */
#include "string3423/algo_3423.h"
QVector<double> algo_3423::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
