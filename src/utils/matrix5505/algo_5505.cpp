/**
 * @file algo_5505.cpp
 */
#include "matrix5505/algo_5505.h"
QVector<double> algo_5505::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
