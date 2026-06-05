/**
 * @file algo_4965.cpp
 */
#include "matrix4965/algo_4965.h"
QVector<double> algo_4965::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
