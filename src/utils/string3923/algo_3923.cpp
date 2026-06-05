/**
 * @file algo_3923.cpp
 */
#include "string3923/algo_3923.h"
QVector<double> algo_3923::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
