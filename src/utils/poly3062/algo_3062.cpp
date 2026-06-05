/**
 * @file algo_3062.cpp
 */
#include "poly3062/algo_3062.h"
QVector<double> algo_3062::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
