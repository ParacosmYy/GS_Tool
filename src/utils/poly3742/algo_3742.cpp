/**
 * @file algo_3742.cpp
 */
#include "poly3742/algo_3742.h"
QVector<double> algo_3742::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
