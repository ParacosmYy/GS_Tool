/**
 * @file algo_6405.cpp
 */
#include "matrix6405/algo_6405.h"
QVector<double> algo_6405::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
