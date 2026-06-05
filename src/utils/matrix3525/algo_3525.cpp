/**
 * @file algo_3525.cpp
 */
#include "matrix3525/algo_3525.h"
QVector<double> algo_3525::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
