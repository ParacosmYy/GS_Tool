/**
 * @file algo_3292.cpp
 */
#include "compress3292/algo_3292.h"
QVector<double> algo_3292::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
