/**
 * @file algo_4966.cpp
 */
#include "signal4966/algo_4966.h"
QVector<double> algo_4966::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
