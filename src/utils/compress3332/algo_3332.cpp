/**
 * @file algo_3332.cpp
 */
#include "compress3332/algo_3332.h"
QVector<double> algo_3332::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
