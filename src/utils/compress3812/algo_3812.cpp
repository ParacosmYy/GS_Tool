/**
 * @file algo_3812.cpp
 */
#include "compress3812/algo_3812.h"
QVector<double> algo_3812::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
