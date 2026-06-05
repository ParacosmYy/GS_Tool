/**
 * @file algo_4856.cpp
 */
#include "geometry4856/algo_4856.h"
QVector<double> algo_4856::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
