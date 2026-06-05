/**
 * @file algo_3076.cpp
 */
#include "geometry3076/algo_3076.h"
QVector<double> algo_3076::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
