/**
 * @file algo_3526.cpp
 */
#include "signal3526/algo_3526.h"
QVector<double> algo_3526::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
