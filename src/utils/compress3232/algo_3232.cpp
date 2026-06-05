/**
 * @file algo_3232.cpp
 */
#include "compress3232/algo_3232.h"
QVector<double> algo_3232::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
