/**
 * @file algo_4682.cpp
 */
#include "poly4682/algo_4682.h"
QVector<double> algo_4682::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
