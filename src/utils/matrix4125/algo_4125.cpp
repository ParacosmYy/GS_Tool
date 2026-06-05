/**
 * @file algo_4125.cpp
 */
#include "matrix4125/algo_4125.h"
QVector<double> algo_4125::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
