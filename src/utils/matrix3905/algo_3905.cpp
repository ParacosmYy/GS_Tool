/**
 * @file algo_3905.cpp
 */
#include "matrix3905/algo_3905.h"
QVector<double> algo_3905::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
