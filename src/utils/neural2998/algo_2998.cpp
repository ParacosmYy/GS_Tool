/**
 * @file algo_2998.cpp
 */
#include "neural2998/algo_2998.h"
QVector<double> algo_2998::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
