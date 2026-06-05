/**
 * @file algo_4765.cpp
 */
#include "matrix4765/algo_4765.h"
QVector<double> algo_4765::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
