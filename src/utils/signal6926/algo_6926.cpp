/**
 * @file algo_6926.cpp
 */
#include "signal6926/algo_6926.h"
QVector<double> algo_6926::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
