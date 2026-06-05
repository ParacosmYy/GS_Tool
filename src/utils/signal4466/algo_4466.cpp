/**
 * @file algo_4466.cpp
 */
#include "signal4466/algo_4466.h"
QVector<double> algo_4466::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
