/**
 * @file algo_7365.cpp
 */
#include "matrix7365/algo_7365.h"
QVector<double> algo_7365::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
