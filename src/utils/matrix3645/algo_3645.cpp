/**
 * @file algo_3645.cpp
 */
#include "matrix3645/algo_3645.h"
QVector<double> algo_3645::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
