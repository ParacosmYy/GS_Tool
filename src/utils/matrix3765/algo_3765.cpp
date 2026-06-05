/**
 * @file algo_3765.cpp
 */
#include "matrix3765/algo_3765.h"
QVector<double> algo_3765::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
