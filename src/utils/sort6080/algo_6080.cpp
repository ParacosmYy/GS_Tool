/**
 * @file algo_6080.cpp
 */
#include "sort6080/algo_6080.h"
QVector<double> algo_6080::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
