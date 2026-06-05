/**
 * @file algo_4086.cpp
 */
#include "signal4086/algo_4086.h"
QVector<double> algo_4086::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
