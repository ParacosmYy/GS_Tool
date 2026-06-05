/**
 * @file algo_4660.cpp
 */
#include "sort4660/algo_4660.h"
QVector<double> algo_4660::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
