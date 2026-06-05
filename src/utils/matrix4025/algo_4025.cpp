/**
 * @file algo_4025.cpp
 */
#include "matrix4025/algo_4025.h"
QVector<double> algo_4025::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
