/**
 * @file algo_4372.cpp
 */
#include "compress4372/algo_4372.h"
QVector<double> algo_4372::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
