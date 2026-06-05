/**
 * @file algo_3061.cpp
 */
#include "interp3061/algo_3061.h"
QVector<double> algo_3061::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
