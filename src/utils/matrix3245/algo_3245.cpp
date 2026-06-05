/**
 * @file algo_3245.cpp
 */
#include "matrix3245/algo_3245.h"
QVector<double> algo_3245::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
