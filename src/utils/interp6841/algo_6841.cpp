/**
 * @file algo_6841.cpp
 */
#include "interp6841/algo_6841.h"
QVector<double> algo_6841::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
