/**
 * @file algo_3583.cpp
 */
#include "string3583/algo_3583.h"
QVector<double> algo_3583::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
