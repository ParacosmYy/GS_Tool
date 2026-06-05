/**
 * @file algo_6293.cpp
 */
#include "crypto6293/algo_6293.h"
QVector<double> algo_6293::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
