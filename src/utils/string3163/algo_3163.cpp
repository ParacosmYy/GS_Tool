/**
 * @file algo_3163.cpp
 */
#include "string3163/algo_3163.h"
QVector<double> algo_3163::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
