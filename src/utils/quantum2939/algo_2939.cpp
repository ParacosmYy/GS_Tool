/**
 * @file algo_2939.cpp
 */
#include "quantum2939/algo_2939.h"
QVector<double> algo_2939::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
