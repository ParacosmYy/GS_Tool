/**
 * @file algo_5765.cpp
 */
#include "matrix5765/algo_5765.h"
QVector<double> algo_5765::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
