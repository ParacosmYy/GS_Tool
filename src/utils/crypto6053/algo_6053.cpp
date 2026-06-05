/**
 * @file algo_6053.cpp
 */
#include "crypto6053/algo_6053.h"
QVector<double> algo_6053::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
