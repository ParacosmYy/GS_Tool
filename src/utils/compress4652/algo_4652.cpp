/**
 * @file algo_4652.cpp
 */
#include "compress4652/algo_4652.h"
QVector<double> algo_4652::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
