/**
 * @file algo_3092.cpp
 */
#include "compress3092/algo_3092.h"
QVector<double> algo_3092::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
