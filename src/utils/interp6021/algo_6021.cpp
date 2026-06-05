/**
 * @file algo_6021.cpp
 */
#include "interp6021/algo_6021.h"
QVector<double> algo_6021::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
