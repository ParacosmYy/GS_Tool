/**
 * @file algo_3057.cpp
 */
#include "image3057/algo_3057.h"
QVector<double> algo_3057::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
