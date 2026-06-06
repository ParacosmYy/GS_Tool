/**
 * @file algo_7021.cpp
 */
#include "interp7021/algo_7021.h"
QVector<double> algo_7021::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
