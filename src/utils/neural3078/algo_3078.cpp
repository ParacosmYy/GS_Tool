/**
 * @file algo_3078.cpp
 */
#include "neural3078/algo_3078.h"
QVector<double> algo_3078::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
