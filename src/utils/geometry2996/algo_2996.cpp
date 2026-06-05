/**
 * @file algo_2996.cpp
 */
#include "geometry2996/algo_2996.h"
QVector<double> algo_2996::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
