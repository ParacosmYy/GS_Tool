/**
 * @file algo_4114.cpp
 */
#include "numeric4114/algo_4114.h"
QVector<double> algo_4114::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
