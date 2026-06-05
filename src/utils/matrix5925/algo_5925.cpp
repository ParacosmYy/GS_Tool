/**
 * @file algo_5925.cpp
 */
#include "matrix5925/algo_5925.h"
QVector<double> algo_5925::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
