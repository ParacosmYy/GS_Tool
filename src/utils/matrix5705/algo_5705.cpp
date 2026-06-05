/**
 * @file algo_5705.cpp
 */
#include "matrix5705/algo_5705.h"
QVector<double> algo_5705::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
