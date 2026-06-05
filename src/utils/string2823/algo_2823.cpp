/**
 * @file algo_2823.cpp
 */
#include "string2823/algo_2823.h"
QVector<double> algo_2823::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
