/**
 * @file algo_4931.cpp
 */
#include "tree4931/algo_4931.h"
QVector<double> algo_4931::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
