/**
 * @file algo_3063.cpp
 */
#include "string3063/algo_3063.h"
QVector<double> algo_3063::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
