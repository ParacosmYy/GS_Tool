/**
 * @file algo_4005.cpp
 */
#include "matrix4005/algo_4005.h"
QVector<double> algo_4005::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
