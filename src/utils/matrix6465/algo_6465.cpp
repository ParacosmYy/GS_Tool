/**
 * @file algo_6465.cpp
 */
#include "matrix6465/algo_6465.h"
QVector<double> algo_6465::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
