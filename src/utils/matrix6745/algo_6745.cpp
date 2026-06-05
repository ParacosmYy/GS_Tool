/**
 * @file algo_6745.cpp
 */
#include "matrix6745/algo_6745.h"
QVector<double> algo_6745::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
