/**
 * @file algo_6274.cpp
 */
#include "numeric6274/algo_6274.h"
QVector<double> algo_6274::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
