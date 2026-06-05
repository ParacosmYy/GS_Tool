/**
 * @file algo_3125.cpp
 */
#include "matrix3125/algo_3125.h"
QVector<double> algo_3125::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
